/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include <cjson/cJSON.h>
#include <stdint.h>

#define VERSION "0.1"

#ifndef TESTIFY_H
#define TESTIFY_H
typedef struct
{
    int help_shown;
    int reason;
    int timeout;
    sds filename;
} arguments;

extern arguments args;

typedef struct
{
    int count;
    sds *outputs;
} output;

typedef struct
{
    int count;
    size_t *diffs;
} difference;

typedef struct
{
    sds name;
    sds description;
    sds input;
    sds validationtype;
    output *expectedoutput;
    output *notexpectedoutput;
    output *containingoutput;
    output *notcontainingoutput;
    output *matchregex;
    output *notmatchregex;
    int exitcodesmaller;
    int exitcodeequals;
    int exitcodegreater;
    int type;
    int timeout;
    int expectedoutputgiven;
    int notexpectedoutputgiven;
    int containingoutputgiven;
    int notcontainingoutputgiven;
    int exitcodesmallergiven;
    int exitcodeequalsgiven;
    int exitcodegreatergiven;
    int matchregexgiven;
    int notmatchregexgiven;
    int exitcode;
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
sds execute(char **, char *, int *, int *);
int passed_or_not(char *, testcase, int, sds *, int64_t, int);
int get_testcase_count(char *);
void get_timeout_json(char *);
void create_json();
void execution_summary(int, int);
void replaced_print(char *, difference *);
int compare(char *, char *);
difference *diff(char *, char *);
int regex_pass(char *, char *, int);
#endif
