#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TRUE 1
#define FALSE !TRUE


struct DoubleLinkedList
{
    char *content;
    struct DoubleLinkedList *prev, *next;

};


void reportMallocErr(size_t condition)
{
    if (condition == 0)
    {
        fprintf(stderr, "malloc failed: not enough memory\n");
        exit(1);
    }
}


int main(int argc, char *argv[]) {

    // check the number of arguments
    if (argc >= 4)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    // creat io objects, std by default
    FILE *fin = stdin, *fout = stdout;

    // update to file io objects if provided
    if (argc >= 2)
    {
        fin = fopen(argv[1], "r");
        if (fin == NULL)
        {
            fprintf(stderr, "%s \'%s\'\n", "reverse: cannot open file", argv[1]);
            exit(1);
        }
    }

    // make sure input and output file names are different
    if (argc == 3)
    {
        struct stat *statbuf1 = (struct stat *)malloc(sizeof(struct stat));
        struct stat *statbuf2 = (struct stat *)malloc(sizeof(struct stat));
        stat(argv[1], statbuf1);
        stat(argv[2], statbuf2);
        if(statbuf1->st_ino == statbuf2->st_ino)
        {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }

        fout = fopen(argv[2], "w");
        if (fout == NULL)
        {
            fprintf(stderr, "%s \'%s\'\n", "reverse: cannot open file", argv[2]);
            exit(1);
        }
    }

    // load lines to a double linked list
    size_t buffer_size = INT_MAX;
    struct DoubleLinkedList *line = (struct DoubleLinkedList *)malloc(sizeof(struct DoubleLinkedList));
    reportMallocErr(line != NULL);

    // allocate memory for content, initialize prev and next to null
    line->content = (char *)malloc(buffer_size * sizeof(char));
    reportMallocErr(line->content != NULL);
    line->prev = NULL;
    line->next = NULL;
    while (getline(&(line->content), &buffer_size, fin) > -1)
    {
        line->next = (struct DoubleLinkedList *)malloc(sizeof(struct DoubleLinkedList));
        reportMallocErr(line->next != NULL);
        struct DoubleLinkedList *prev_line = line;
        line = line->next;
        // allocate memory for content, initialize prev to prev and next to null
        line->content = (char *)malloc(buffer_size * sizeof(char));
        reportMallocErr(line->content != NULL);
        line->prev = prev_line;
        line->next = NULL;
    }

    // output lines and clean up memory
    while (line->prev != NULL)
    {
        line = line->prev;
        free(line->next);
        fprintf(fout, "%s", line->content);
    }
    free(line);

    // close files
    if (argc >= 2)
    {
        fclose(fin);
    }
    if (argc == 3)
    {
        fclose(fout);
    }

    return 0;

}
