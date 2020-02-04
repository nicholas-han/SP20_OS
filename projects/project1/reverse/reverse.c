#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

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


int main(int argc, char* argv[]) {

    // check the number of arguments
    if (argc >= 4)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc == 3)
    {
        fprintf(stdout, "%s", argv[1]);
        fprintf(stdout, "%s", argv[2]);

    }
    return 0;
    // make sure input and output file names are different
    if (argc == 3 && argv[1] == argv[2])
    {
        fprintf(stderr, "Input and output file must differ\n");
        exit(1);
    }

    // creat io objects, std by default
    FILE *fin = stdin, *fout = stdout;

    // update to file io objects if provided
    if (argc >= 2)
    {
        fin = fopen(argv[1], "r");
    }

    if (argc == 3)
    {
        fout = fopen(argv[2], "w");
    }

    // create buffer for getline()
    size_t buffer_size = INT_MAX;

    // load lines to a double linked list
    struct DoubleLinkedList *line = (struct DoubleLinkedList *)malloc(sizeof(struct DoubleLinkedList));
    reportMallocErr(line != NULL);

    // allocate memory for content, initialize prev and next to null
    line->content = (char *)malloc(buffer_size * sizeof(char));
    reportMallocErr(line->content != NULL);
    line->prev = NULL;
    line->next = NULL;
    size_t if_getline = getline(&(line->content), &buffer_size, fin);
    while (if_getline > 1)
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
        if_getline = getline(&(line->content), &buffer_size, fin);
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
