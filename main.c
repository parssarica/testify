/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

arguments args = {0, 0, -1, 0, 0, 0, NULL, NULL, NULL};
int main(int argc, char **argv)
{
    FILE *fptr;
    long size;
    char *json_file;
    int is_creating_json = 0;
    int run_tests = 0;
    if (argc == 1)
    {
        help();
        return 1;
    }

    if (strcmp(argv[1], "help") == 0)
    {
        help();
        goto exit;
    }
    else if (strcmp(argv[1], "version") == 0)
    {
        version();
        goto exit;
    }
    else if (strcmp(argv[1], "run") == 0)
    {
        run_tests = 1;
        parse_run(&argv[1], argc - 1);
        if (args.help_shown == 1)
            goto exit;
    }
    else if (strcmp(argv[1], "create") == 0)
    {
        is_creating_json = 1;
    }
    else
    {
        help();
        goto exit;
    }

    if (run_tests)
    {
        fptr = fopen(args.filename, "rt");
        if (fptr == NULL)
        {
            perror("fopen");
            goto exit;
        }
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        if (size < 0)
        {
            perror("ftell");
            fclose(fptr);
            return 1;
        }
        json_file = malloc(size);
        fread(json_file, 1, size, fptr);
        runtests(json_file);
        fclose(fptr);
        free(json_file);
    }
    else if (is_creating_json)
    {
        create_json();
    }
exit:
    sdsfree(args.filename);
    if (run_tests)
        sdsfree(args.binary_file);
}
