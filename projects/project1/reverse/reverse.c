#include <stdio.h>

struct DoubleLinkedList {
    char *content;
    struct DoubleLinkedList *prev, *next;

};

int main(int argc, char* argv[]) {

    // check the number of arguments
    if (argc >= 4)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

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
    char *buffer = (char *)malloc(INT_MAX * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "malloc failed: not enough memory\n");
        exit(1);
    }

    // load lines to a double linked list
    struct DoubleLinkedList *line;
    line->prev = NULL;
    line->next = NULL;
    size_t if_getline = getline(&buffer, INT_MAX, fin);
    while (if_getline != -1)
    {
        line->content = buffer;
        line->next = (struct DoubleLinkedList *)malloc(sizeof(struct DoubleLinkedList));
        struct DoubleLinkedList *prev_line = line;
        line = line->next;
        line->prev = prev_line;
        lines->next = NULL;
        if_getline = getline(&buffer, INT_MAX, fin);
    }
    free(buffer);

    // output lines
    struct DoubleLinkedList *curr_line;
    while(true)
    {
        curr_line = line;
        line = line->prev;
        free(curr_line);

        fprintf(fout, line->content);

        if (line->prev != NULL)
        {
            break;
        }
    }

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
