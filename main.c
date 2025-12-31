/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <stdio.h>
#include <string.h>

arguments args = {0, NULL};
int main(int argc, char **argv)
{
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
        parse_run(&argv[1], argc - 1);
        if (args.help_shown == 1)
            goto exit;
    }

exit:
    sdsfree(args.filename);
}
