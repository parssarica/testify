/*
Pars SARICA <pars@parssarica.com>
*/

#include "testify.h"
#include <stdio.h>

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
