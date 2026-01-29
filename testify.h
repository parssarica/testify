/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include <cjson/cJSON.h>
#include <stdint.h>

#define VERSION "0.1"

#ifndef TESTIFY_H
#define TESTIFY_H

#define VARIABLE_STRING 0
#define VARIABLE_INT 1
#define VARIABLE_DOUBLE 2

#define EPSILON 0.000001

typedef struct
{
    int help_shown;
    int reason;
    int timeout;
    int testcase_count;
    int testcase_counter;
    int env_count;
    sds filename;
    sds binary_file;
    sds *environmental_values;
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
    int64_t duration;
    sds *environmental_values;
} testcase;

typedef struct
{
    sds cmd;
    sds source;
    sds store;
    sds lhs;
    sds rhs;
    sds *values;
    int value_count;
    int lhs_int;
    int rhs_int;
    double lhs_double;
    double rhs_double;
    int lhs_type;
    int rhs_type;
    size_t index;
    int background;
} command;

typedef struct
{
    int type;
    sds name;
    sds valuestring;
    int valueint;
    double valuedouble;
    int empty;
} variable;

typedef struct
{
    pid_t pid;
    int pty_fd;
} process;

void logo();
void help();
void version();
void runhelp();
void parse_run(char **, int);
const char *get_error();
void get_binary_json(sds *, char *);
testcase parse_testcase(cJSON *);
int runtests(char *);
sds execute(char **, char *, int *, int *, sds *, int);
int passed_or_not(char *, testcase, int, sds *, int64_t, int);
int get_testcase_count(char *);
void get_timeout_json(char *);
void create_json();
void execution_summary(int, int);
void replaced_print(char *, difference *);
int compare(char *, char *);
difference *diff(char *, char *);
int regex_pass(char *, char *, int);
int test(cJSON *);
void print_output(testcase, int, sds *, sds *);
char **make_env(sds *, int, int *);
int complex_test(cJSON *);
void new_variable(char *, int, char *, int, double);
char *get_source_str(char *, char *);
int get_source_int(char *);
double get_source_double(char *);
int define_variable_type(char *);
sds to_str(variable);
double to_double(variable);
variable get_var_object(char *, int, double, int);
void destroy_empty_variable(variable);
int file_exists(char *);
double get_source_non_char(char *);
sds regex_extract(char *, char *, int, int);
int regex_count(char *, char *, size_t);
process execute_background(char **, sds *, int);
void interact_write(process *, sds *);
ssize_t interact_read(process *, char *, size_t, int);
void close_child(process *, int *, int *);
int child_alive(process *, int *, int *);
void kill_process(process *, int *, int *);
#endif
