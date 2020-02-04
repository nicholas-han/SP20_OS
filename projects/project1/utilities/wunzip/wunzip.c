#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    // loop through all files
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL)
        {
            printf("wunzip: unable to open file\n");
            exit(1);
        }
        // unpack all characters
        while (!feof(fp))
        {
            size_t counter;
            char buffer;
            fread(&counter, 4, 1, fp);
            fread(&buffer, 1, 1, fp);
            for (size_t i = 0; i < counter; i++)
            {
                fprintf(stdout, "%c", buffer);
            }

        }
        fclose(fp);
    }

    return 0;
}
