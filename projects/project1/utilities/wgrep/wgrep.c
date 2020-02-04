#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


struct Linkedlist
{
    char *content;
    struct Linkedlist *prev, *next;
};


int wordInLine(char *word, char *line);
void grep(char *keyword, FILE* fp);


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    char *keyword = argv[1];

    // if keyword is empty string, terminate program with no error
    if (*keyword == '\0') return 0;

    // if no files provided, take console input
    if (argc == 2)
    {
        grep(keyword, stdin);
        return 0;
    }

    // if files provided, loop through each file
    FILE *fp = NULL;
    for (int i = 2; i < argc; i++)
    {
        fp = fopen(argv[i], "r");
        if(fp == NULL)
        {
            printf("wgrep: cannot open file\n");
            exit(1);
        }
        grep(keyword, fp);
    }
    fclose(fp);

    return 0;
}


int wordInLine(char *word, char *line)
{
    if (strlen(word) > strlen(line)) return 0;
    if (strncmp(line, word, strlen(word)) == 0) return 1;
    else return wordInLine(word, line + 1);
}


void grep(char *keyword, FILE* fp)
{
    size_t buffer_size = SIZE_MAX;

    // keep only lines with keyword and save to a linked list
    struct Linkedlist *lines = (struct Linkedlist *)malloc(sizeof(struct Linkedlist));
    struct Linkedlist *root = lines;
    lines->content = (char *)malloc(buffer_size * sizeof(char));
    lines->prev = NULL;
    while(getline(&lines->content, &buffer_size, fp) > -1)
    {
        if (wordInLine(keyword, lines->content))
        {
            lines->next = (struct Linkedlist *)malloc(sizeof(struct Linkedlist));
            struct Linkedlist *prev_line = lines;
            lines = lines->next;
            lines->prev = prev_line;
            lines->content = (char *)malloc(buffer_size * sizeof(char));
        }
    }

    // print every line from linked list
    lines = root;
    while(lines->next != NULL)
    {
        lines = lines->next;
        printf("%s", lines->prev->content);
    }

    // clean up memory backward
    while(lines->prev != NULL)
    {
        lines = lines->prev;
        free(lines->next);
    }
    free(root);

}
