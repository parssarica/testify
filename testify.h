/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"

#define VERSION "0.1"

#ifndef TESTIFY_H
#define TESTIFY_H
typedef struct
{
    int help_shown;
    sds filename;
} arguments;

extern arguments args;
#endif

void logo();
void help();
void version();
void runhelp();
void parse_run(char **, int);
