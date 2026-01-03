/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include <cjson/cJSON.h>

#define VERSION "0.1"

#ifndef TESTIFY_H
#define TESTIFY_H
typedef struct
{
    int help_shown;
    sds filename;
} arguments;

extern arguments args;

typedef struct
{
    sds name;
    sds description;
    sds input;
    sds expectedoutput;
    sds notexpectedoutput;
    sds containingoutput;
    sds notcontainingoutput;
    sds *commandargs;
    int type;
} testcase;

void logo();
void help();
void version();
void runhelp();
void parse_run(char **, int);
const char *get_error();
void get_binary_json(sds *, char *);
testcase parse_testcase(cJSON *);
int runtests(char *);
sds execute(char *[], char *);
#endif
