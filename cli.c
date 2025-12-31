/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <getopt.h>
#include <stddef.h>
#include <string.h>

void parse_run(char **argv, int argc)
{
    int opt, option_index;
    int help_shown = 0;
    sds filename = sdsnew("testcases.json");

    struct option long_options[] = {{"file", required_argument, NULL, 'f'},
                                    {"help", no_argument, NULL, 'h'},
                                    {NULL, 0, NULL, 0}};

    while ((opt = getopt_long(argc, argv, "f:h", long_options,
                              &option_index)) != -1)
    {
        switch (opt)
        {
        case 'f':
            filename = sdscpylen(filename, optarg, strlen(optarg));
            break;
        case 'h':
            runhelp();
            help_shown = 1;
            break;
        case 0:
            if (strcmp(long_options[option_index].name, "file") == 0)
            {
                filename = sdscpylen(filename, optarg, strlen(optarg));
            }
            else if (strcmp(long_options[option_index].name, "help") == 0)
            {
                runhelp();
                help_shown = 1;
            }
        }
    }

    args.filename = sdsdup(filename);
    args.help_shown = help_shown;
    sdsfree(filename);
}
