#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


const int error_message_len = 30;
char error_message[error_message_len] = "An error has occurred\n";

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
    size_t buffer_size = 3500;
    char *path[100];
    path[0] = "/bin/";
    for (int i = 1; i < 100; ++i) path[i] = NULL;

    // load command line by line
    char *buffer;
    if (argc == 1) printf("wish> ");
    while (getline(&buffer, &buffer_size, fin) > -1) {
        // parse line into words
        buffer[strlen(buffer) - 1] = 0; // remove \n to avoid strsep issues
        char *token, *bufferv[100];
        for (int i = 0; i < 100; ++i) bufferv[i] = NULL;
        int i = 0;
        while ((token = strsep(&buffer, " \t")) != NULL) {
            if (strlen(token) > 0) {
                bufferv[i] = token;
                ++i;
                if (i >= 100) break;
            }
        }

        // if no command given, re-prompt for user inputs
        if (bufferv[0] == NULL) {
            if (argc == 1) printf("wish> ");
            continue;
        }

        // built-in types
        if (strcmp(bufferv[0], "exit") == 0){
            exit(0);
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
                path[0] = NULL;
                ++i;
            }
            // write new paths
            i = 1;
            while (bufferv[i] != NULL) {
                path[i - 1] = strcat(bufferv[i], "/");
                ++i;
            }
        }
        // for other commands, call execv()
        else {
            // // if no paths given, don't do anything and promopt for new command
            // if (path[0] == NULL) {
            //     if (argc == 1) printf("wish> ");
            //     continue;
            // }
            int rc = fork();
            if (rc < 0) {
                // fork failed
                write(STDERR_FILENO, error_message, error_message_len);
            }
            else if (rc == 0) { // execute command in child process
                // pick the first valid path + executable
                int i = 0;
                while (path[i] != NULL) {
                    char full_path[100];
                    strcpy(full_path, path[i]);
                    strcat(full_path, bufferv[0]);
                    if (access(full_path, X_OK) == 0) break;
                    ++i;
                }
                if (path[i] == NULL) {
                    // if no path works, that's an error
                    write(STDERR_FILENO, error_message, error_message_len);

                }
                char full_path[100];
                strcpy(full_path, path[i]);
                strcat(full_path, bufferv[0]);
                printf("%s", full_path);
                // execute command using absolute path per project requirement
                // if want to use relative path, use execvp()
                execv(full_path, bufferv);

                // if execv does return, there is an error
                write(STDERR_FILENO, error_message, error_message_len);

            }
            else {
                // let the parent job wait till the child is finished
                wait(NULL);
            }
        }

        // prompt user for next command
        if (argc == 1) printf("wish> ");
    }

    // close file if used as batch input
    if (argc == 2) {
        fclose(fin);
    }
    exit(0);
}
