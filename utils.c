/*
Pars SARICA <pars@parssarica.com>
*/

#include "testify.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>

void logo()
{
    printf("████████ ███████ ███████ ████████ ██ ███████ ██    ██ \n");
    printf("   ██    ██      ██         ██    ██ ██       ██  ██  \n");
    printf("   ██    █████   ███████    ██    ██ █████     ████   \n");
    printf("   ██    ██           ██    ██    ██ ██         ██    \n");
    printf("   ██    ███████ ███████    ██    ██ ██         ██    \n\n");
}

void help()
{
    logo();
    printf("testify <commands>\n");
    printf("\nCommands:\n\n");
    printf("\thelp\tShows this help message and exit\n");
    printf("\tversion\tShows the version of program\n");
    printf("\trun\tReads the test cases and runs them\n");
    printf("\tcreate\tCreates the JSON file\n");
}

void runhelp()
{
    logo();
    printf("testify run <options>\n");
    printf("\nOptions:\n\n");
    printf("\t-h / --help\tShows this help message and exit\n");
    printf("\t-f / --file\tSpecifies the file including test cases\n");
    printf("\t--no-reason\tDoesn't shows the reason on test report\n");
}

void version()
{
    logo();
    printf("\nPars SARICA <pars@parssarica.com>\n");
    printf("v%s\n", VERSION);
}

size_t length(char *s)
{
    size_t count = 0;
    while (*s)
    {
        if ((*s & 0xC0) != 0x80)
            count++;
        s++;
    }
    return count;
}

void execution_summary(int passed, int failed)
{
    sds firstline = sdsempty();
    sds secondline = sdsempty();
    sds thirdline = sdsempty();
    sds fourthline = sdsempty();
    sds fifthline = sdsempty();
    sds sixthline = sdsempty();
    sds seventhline = sdsempty();
    sds passed_str = sdsempty();
    sds failed_str = sdsempty();
    sds successrate_str = sdsempty();
    sds legend = sdsempty();
    size_t max = 0;
    size_t which_max = 0;
    int firstspace;
    int secondspace;

    setlocale(LC_ALL, "");
    passed_str = sdscatprintf(passed_str, "%d", passed);
    failed_str = sdscatprintf(failed_str, "%d", failed);
    successrate_str = sdscatprintf(
        successrate_str, "%.1f%%",
        (((double)passed / ((double)passed + (double)failed)) * 100));
    fourthline =
        sdscatprintf(fourthline, "│ Passed           │ %s", passed_str);
    fifthline = sdscatprintf(fifthline, "│ Failed           │ %s", failed_str);
    sixthline =
        sdscatprintf(sixthline, "│ Success Rate     │ %s", successrate_str);
    max = length(fourthline);
    if (length(fifthline) > max)
    {
        max = length(fifthline);
        which_max = 1;
    }
    if (length(sixthline) > max)
    {
        max = length(sixthline);
        which_max = 2;
    }

    if (which_max == 0)
    {
        while (length(fifthline) != length(fourthline))
        {
            fifthline = sdscatlen(fifthline, " ", 1);
        }
        while (length(sixthline) != length(fourthline))
        {
            sixthline = sdscatlen(sixthline, " ", 1);
        }
    }
    else if (which_max == 1)
    {
        while (length(fourthline) != length(fifthline))
        {
            fourthline = sdscatlen(fourthline, " ", 1);
        }
        while (length(sixthline) != length(fifthline))
        {
            sixthline = sdscatlen(sixthline, " ", 1);
        }
    }
    else if (which_max == 2)
    {
        while (length(fourthline) != length(sixthline))
        {
            fourthline = sdscatlen(fourthline, " ", 1);
        }
        while (length(fifthline) != length(sixthline))
        {
            fifthline = sdscatlen(fifthline, " ", 1);
        }
    }
    fourthline = sdscat(fourthline, " │");
    fifthline = sdscat(fifthline, " │");
    sixthline = sdscat(sixthline, " │");
    for (size_t i = 0; length(seventhline) < (length(fourthline)); i++)
    {
        if (i == 0)
        {
            seventhline = sdscat(seventhline, "└");
        }
        else if (i == 19)
        {
            seventhline = sdscat(seventhline, "┴");
        }
        else if (i == length(fourthline) - 1)
        {
            seventhline = sdscat(seventhline, "┘");
        }
        else
        {
            seventhline = sdscat(seventhline, "─");
        }
    }
    for (size_t i = 0; length(firstline) < (length(fourthline)); i++)
    {
        if (i == 0)
        {
            firstline = sdscat(firstline, "┌");
        }
        else if (i == length(fourthline) - 1)
        {
            firstline = sdscat(firstline, "┐");
        }
        else
        {
            firstline = sdscat(firstline, "─");
        }
    }
    for (size_t i = 0; length(thirdline) < (length(fourthline)); i++)
    {
        if (i == 0)
        {
            thirdline = sdscat(thirdline, "├");
        }
        else if (i == 19)
        {
            thirdline = sdscat(thirdline, "┬");
        }
        else if (i == length(fourthline) - 1)
        {
            thirdline = sdscat(thirdline, "┤");
        }
        else
        {
            thirdline = sdscat(thirdline, "─");
        }
    }
    secondline = sdscpy(secondline, "│");
    if ((length(fourthline) - 17) % 2 != 0)
    {
        firstspace = (int)((length(fourthline) - 17) / 2 - 0.5);
        secondspace = firstspace + 1;
    }
    else
    {
        firstspace = (length(fourthline) - 17) / 2 - 1;
        secondspace = (length(fourthline) - 17) / 2 - 1;
    }

    for (int i = 0; i < firstspace; i++)
        secondline = sdscat(secondline, " ");

    secondline = sdscat(secondline, "EXECUTION SUMMARY");

    for (int i = 0; i < secondspace; i++)
        secondline = sdscat(secondline, " ");

    secondline = sdscat(secondline, "│");
    // Legend: ↵ Newline, · Space, →··· Tab
    if (length(fourthline) > 36)
    {
        for (size_t i = 0; i < ((length(fourthline) - 36) / 2); i++)
        {
            legend = sdscat(legend, " ");
        }
    }
    legend = sdscat(legend, "Legend: ↵ Newline, · Space, →··· Tab");
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", firstline, secondline, thirdline,
           fourthline, fifthline, sixthline, seventhline, legend);
    sdsfree(firstline);
    sdsfree(secondline);
    sdsfree(thirdline);
    sdsfree(fourthline);
    sdsfree(fifthline);
    sdsfree(sixthline);
    sdsfree(seventhline);
    sdsfree(passed_str);
    sdsfree(failed_str);
    sdsfree(successrate_str);
    sdsfree(legend);
}

void replaced_print(char *string)
{
    char byte[2];
    int skip = 0;
    int strip_characters = 0;
    printf("'");
    for (size_t i = strlen(string) - 1; i > 0; i--)
    {
        if (string[i] != ' ' && string[i] != '\n' && string[i] != '\t' &&
            string[i] != '\r')
            break;
        else
            strip_characters++;
    }
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (skip)
        {
            skip = 0;
            continue;
        }
        if (i == 0)
        {
            if (string[i] == ' ')
            {
                printf("\033[90m·\033[0m");
                continue;
            }
        }
        else
        {
            if (string[i] == ' ' &&
                (string[i + 1] == ' ' || string[i - 1] == ' '))
            {
                printf("\033[90m·\033[0m");
                continue;
            }
        }

        if (string[i] == '\n' || (string[i] == '\r' && string[i + 1] == '\n'))
        {
            printf("\033[90m↵\033[0m");
            if (string[i] == '\r')
                skip = 1;
            if (string[i] != '\r' &&
                i < (strlen(string) - strip_characters - 1))
            {
                printf("\n");
            }
        }
        else if (string[i] == '\t')
        {
            printf("\033[90m→···\033[0m");
        }
        else
        {
            byte[0] = string[i];
            printf("%s", byte);
        }
    }
    printf("'");
}

int compare(char *s1, char *s2)
{
    char byte[2];
    byte[1] = 0;
    sds string1 = sdsempty();
    sds string2 = sdsempty();
    int output;
    size_t i;
    for (i = 0; i < strlen(s1); i++)
    {
        if (s1[i] == '\r')
        {
            if (i + 1 < strlen(s1))
            {
                if (s1[i + 1] != '\n')
                {
                    string1 = sdscatlen(string1, "\r", 1);
                }
            }
        }
        else
        {
            byte[0] = s1[i];
            string1 = sdscatlen(string1, byte, 1);
        }
    }

    for (i = 0; i < strlen(s2); i++)
    {
        if (s2[i] == '\r')
        {
            if (i + 1 < strlen(s2))
            {
                if (s2[i + 1] != '\n')
                {
                    string2 = sdscatlen(string2, "\r", 1);
                }
            }
        }
        else
        {
            byte[0] = s2[i];
            string2 = sdscatlen(string2, byte, 1);
        }
    }

    output = strcmp(string1, string2);
    sdsfree(string1);
    sdsfree(string2);
    return output;
}
