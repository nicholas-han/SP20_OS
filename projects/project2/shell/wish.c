#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


// global variables
const int error_message_len = 22;
const char error_message[error_message_len] = "An error has occurred\n";
const size_t PATH_LEN = 100;
const size_t ARGS_NUM = 100;
size_t buffer_size = 3500;

int main(int argc, char *argv[]) {

    // exit if more than one file
    if (argc > 2) {
        write(STDERR_FILENO, error_message, error_message_len);
        exit(1);
    }

    // validate file input
    FILE *fin = stdin;
    if (argc == 2) {
        fin = fopen(argv[1], "r");
        if (fin == NULL)
        {
            write(STDERR_FILENO, error_message, error_message_len);
            exit(1);
        }
    }

    // set initial path to /bin
    char *path[PATH_LEN];
    path[0] = "/bin";
    for (int i = 1; i < PATH_LEN; ++i) path[i] = NULL;

    // load command line by line
    char *buffer;
    while (1) {
        if (argc == 1) printf("wish> ");
        if (getline(&buffer, &buffer_size, fin) <= -1) break;
        // parse line into words
        buffer[strlen(buffer) - 1] = 0; // remove \n to avoid strsep issues
        char *word_token, *bufferv[ARGS_NUM];
        int bufferv_ct = 0;
        for (int i = 0; i < ARGS_NUM; ++i) bufferv[i] = NULL;
        // tokenize by white spaces and tabs
        while ((word_token = strsep(&buffer, " \t")) != NULL) {
            if (strlen(word_token) > 0) {
                // tokenize by > and &; store them in argument array too
                // if no > or &, load whole token as one argument
                if (strchr(word_token, '>') == NULL && strchr(word_token, '&') == NULL)
                    bufferv[bufferv_ct++] = word_token;
                // otherwise, further separarate into arguments
                else {
                    char *left_char = word_token, *right_char = word_token;
                    while (right_char != NULL && strlen(right_char) != 0) {
                        // find the next delim char
                        if (*right_char == '&' || *right_char == '>') {
                            // if left != right, first store regular cmd
                            if (left_char != right_char) {
                                // store regular command
                                char cmd_token[right_char - left_char + 1];
                                strncpy(cmd_token, left_char, right_char - left_char);
                                cmd_token[right_char - left_char] = 0;
                                char *ins = strdup(cmd_token);
                                bufferv[bufferv_ct++] = ins;
                            }
                            // then store delim char
                            // bufferv[bufferv_ct++] = (char[2]){*right_char, '\0'};
                            char *ins = strdup((char[2]){*right_char, '\0'});
                            bufferv[bufferv_ct++] = ins;
                            // update both pointers
                            ++right_char;
                            left_char = right_char;
                        }
                        else // if not delim char, only update right_char
                            ++right_char;
                    }
                    if (left_char != NULL && strlen(left_char) != 0)
                        bufferv[bufferv_ct++] = left_char;
                }
                if (bufferv_ct >= ARGS_NUM) break; // only takes up to max num of araguments
            }
        }

        // if no command given, re-prompt for user inputs
        if (bufferv[0] == NULL) continue;

        // built-in types
        if (strcmp(bufferv[0], "exit") == 0){
            int kk = 0;
            if (bufferv[1] == NULL) exit(0);
            else write(STDERR_FILENO, error_message, error_message_len);
        }
        else if (strcmp(bufferv[0], "cd") == 0) {
            // check number of inputs
            if (bufferv[1] == NULL || bufferv[2] != NULL) {
                write(STDERR_FILENO, error_message, error_message_len);
            }
            else {
                if(chdir(bufferv[1]) != 0) {
                    write(STDERR_FILENO, error_message, error_message_len);
                }
            }
        }
        else if (strcmp(bufferv[0], "path") == 0) {
            // clear previous paths
            int i = 0;
            while (path[i] != NULL) {
                path[i] = NULL;
                ++i;
            }
            // write new paths
            i = 1;
            while (bufferv[i] != NULL) {
                path[i - 1] = bufferv[i];
                ++i;
            }
        }
        // for other commands, handle parallel commands,
        // then handle output redirection,
        // and finally call execv()
        else {
            // handle parallel commands
            int parallel_idx = 0;
            int sub_bufferv_idx = 0;
            char *sub_bufferv[ARGS_NUM];
            int child_ct = 0;
            for (int i = 0; i < ARGS_NUM; ++i) sub_bufferv[i] = NULL;
            while (bufferv[parallel_idx] != NULL || sub_bufferv_idx != 0) {
                if (bufferv[parallel_idx] != NULL && strcmp(bufferv[parallel_idx], "&") != 0) {
                    // load buffer vector args to sub buffer vector
                    sub_bufferv[sub_bufferv_idx] = strdup(bufferv[parallel_idx]);
                    ++parallel_idx;
                    ++sub_bufferv_idx;
                }
                else {
                    // stand alone & is an error
                    if (sub_bufferv_idx == 0) {
                        // reset sub_bufferv before starting next parallel command
                        for(int i = 0; i <= sub_bufferv_idx; ++i) sub_bufferv[i] = NULL;
                        sub_bufferv_idx = 0;
                        ++parallel_idx;
                        continue;
                    }
                    // pick the first valid path + executable
                    int path_idx = 0;
                    char *full_path = NULL;
                    while (path[path_idx] != NULL) {
                        char valid_path[PATH_LEN];
                        strcpy(valid_path, path[path_idx]);
                        strcat(valid_path, "/");
                        strcat(valid_path, sub_bufferv[0]);
                        if (access(valid_path, X_OK) == 0) {
                            full_path = strdup(valid_path);
                            break;
                        }
                        ++path_idx;
                    }

                    if (path[path_idx] == NULL) {
                        // if no path works, that's an error
                        write(STDERR_FILENO, error_message, error_message_len);
                        // reset sub_bufferv before starting next parallel command
                        for(int i = 0; i <= sub_bufferv_idx; ++i) sub_bufferv[i] = NULL;
                        sub_bufferv_idx = 0;
                        ++parallel_idx;
                        continue;
                    }

                    // check redirection
                    int redirect_idx = 0;
                    while (sub_bufferv[redirect_idx] != NULL) {
                        if (strcmp(sub_bufferv[redirect_idx], ">") == 0) break;
                        ++redirect_idx;
                    }

                    int out_dest, if_redirected = 0;
                    // if no redirection, skip this part
                    if (sub_bufferv[redirect_idx] != NULL) {
                        if_redirected = 1;
                        // check if follow-up arguments are correct
                        // 1. non-zero args before >, exact 1 arg after >, arg after > cannot be >
                        if (redirect_idx == 0 || sub_bufferv[redirect_idx + 1] == NULL || sub_bufferv[redirect_idx + 2] != NULL || strcmp(sub_bufferv[redirect_idx + 1], ">") == 0) {
                            write(STDERR_FILENO, error_message, error_message_len);
                            // reset sub_bufferv before starting execv()
                            for(int i = 0; i <= sub_bufferv_idx; ++i) sub_bufferv[i] = NULL;
                            sub_bufferv_idx = 0;
                            ++parallel_idx;
                            continue;
                        }
                        // 2. redirection file is openable
                        out_dest = open(sub_bufferv[redirect_idx + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        if (out_dest < 0) {
                            write(STDERR_FILENO, error_message, error_message_len);
                            // reset sub_bufferv before starting execv()
                            for(int i = 0; i <= sub_bufferv_idx; ++i) sub_bufferv[i] = NULL;
                            sub_bufferv_idx = 0;
                            ++parallel_idx;
                            continue;
                        }
                        // remove redirection araguments
                        sub_bufferv[redirect_idx] = NULL;
                        sub_bufferv[redirect_idx + 1] = NULL;
                    }

                    // if all good, create child process
                    int rc = fork();
                    if (rc < 0) {
                        // fork failed
                        write(STDERR_FILENO, error_message, error_message_len);
                    }
                    else if (rc == 0) { // execute command in child process
                        if (if_redirected) {
                            dup2(out_dest, STDOUT_FILENO); // copy file descriptor to std output
                            dup2(out_dest, STDERR_FILENO); // copy to std error
                        }
                        // execute command using absolute path per project requirement
                        // if want to use relative path, use execvp()
                        // printf("child process: %d, parent %d\n", getpid(), getppid());
                        execv(full_path, sub_bufferv);

                        // if execv does return, there is an error
                        write(STDERR_FILENO, error_message, error_message_len);
                    }
                    // reset sub_bufferv before starting execv()
                    for(int i = 0; i <=sub_bufferv_idx; ++i) sub_bufferv[i] = NULL;
                    sub_bufferv_idx = 0;
                    ++parallel_idx;
                    ++child_ct;
                } // end for a single parallel command
            } // end for parallel commands
            // let the parent job wait till all children are finished
            for (int i = 0; i < child_ct; ++i) wait(NULL);
            // printf("num of children: %d\n", child_ct);
            // printf("parent process: %d\n", getpid());
        } // end for other commands
    } // end for user inputs

    // close file if used as batch input
    if (argc == 2) {
        fclose(fin);
    }
    exit(0);
}
