#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    // loop through all files
    char iter_char, curr_char = '\0';
    int char_counter = 0;
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL)
        {
            printf("wzip: unable to open file\n");
            exit(1);
        }
        // loop through all characters
        while (1)
        {
            iter_char = fgetc(fp);
            if (feof(fp)) break;

            if (curr_char == '\0') curr_char = iter_char;
            if (curr_char == iter_char)
            {
                char_counter++;
            }
            else
            {
                fwrite(&char_counter, 4, 1, stdout);
                fwrite(&curr_char, 1, 1, stdout);
                curr_char = iter_char;
                char_counter = 1;
            }
        }
        fclose(fp);
    }
    if (char_counter != 0)
    {
        fwrite(&char_counter, 4, 1, stdout);
        fwrite(&curr_char, 1, 1, stdout);
    }

    return 0;
}
