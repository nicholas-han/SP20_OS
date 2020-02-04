#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    FILE *fp = NULL;
    // loop through all the files provided by user
    for (int i = 1; i < argc; i++)
    {
        fp = fopen(argv[i], "r");
        if(fp == NULL)
        {
            printf("wcat: cannot open file\n");
            exit(1);
        }

        // read file line by line and output to console
        char *buffer = (char *)malloc(INT_MAX * sizeof(char));
        while(fgets(buffer, INT_MAX, fp) != NULL)
        {
            printf("%s", buffer);
        }

        fclose(fp);
    }
    return 0;
}
