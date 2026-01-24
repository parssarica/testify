/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

variable *variables;
int variable_count = 0;

int complex_test(cJSON *testcase_json)
{
    struct timespec ts;
    struct timespec ts2;
    testcase testcase_obj;
    command *commands;
    int fault;
    int exitcode;
    int64_t start_date;
    int64_t end_date;
    int program_args_length;
    sds output;
    sds reason = sdsempty();
    char **program_args;
    cJSON *testcase_input_str;
    cJSON *env_var;
    cJSON *complex_command;
    cJSON *cmd_json;
    cJSON *source_json;
    cJSON *store_json;
    cJSON *lhs_json;
    cJSON *rhs_json;
    cJSON *index_json;
    char extract_char_character[2];
    sds cmd = sdsempty();
    sds source_string = sdsempty();
    sds assert_lhs;
    sds assert_rhs;
    sds file_content;
    double assert_lhs_double = 0;
    double assert_rhs_double = 0;
    int source_int = 0;
    double source_double = 0;
    int commandcount = 0;
    int result = 1;
    int i;
    int env_count;
    int k;
    size_t tmp;
    int64_t duration = 0;
    FILE *fptr;
    char *endptr;

    variable_count = 0;
    extract_char_character[1] = 0;
    testcase_obj = parse_testcase(testcase_json);
    program_args = malloc(sizeof(char **));
    program_args[0] = strdup(args.binary_file);
    i = 1;
    cJSON_ArrayForEach(testcase_input_str, cJSON_GetObjectItemCaseSensitive(
                                               testcase_json, "commandArgs"))
    {
        program_args = realloc(program_args, sizeof(char **) * ++i);
        if (cJSON_IsString(testcase_input_str) &&
            (testcase_input_str->valuestring) != NULL)
        {
            program_args[i - 1] = strdup(testcase_input_str->valuestring);
        }
    }
    program_args = realloc(program_args, sizeof(char **) * ++i + sizeof(NULL));
    program_args[i - 1] = NULL;
    program_args_length = i;
    env_count = 0;
    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcase_json, "enviromentalVariables"))
    {
        env_count++;
    }

    testcase_obj.enviromental_values = malloc(sizeof(sds *) * env_count);
    i = 0;
    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcase_json, "enviromentalVariables"))
    {
        if (cJSON_IsString(env_var) && (env_var->valuestring != NULL))
        {
            testcase_obj.enviromental_values[i] = sdsnew(env_var->valuestring);
            i++;
        }
    }

    commandcount = 0;
    cJSON_ArrayForEach(complex_command, cJSON_GetObjectItemCaseSensitive(
                                            testcase_json, "pipeline"))
    {
        commandcount++;
    }

    commands = malloc(sizeof(command) * commandcount);
    cJSON_ArrayForEach(complex_command, cJSON_GetObjectItemCaseSensitive(
                                            testcase_json, "pipeline"))
    {
        cmd_json = cJSON_GetObjectItemCaseSensitive(complex_command, "cmd");
        if (cJSON_IsString(cmd_json) && (cmd_json->valuestring != NULL))
        {
            commands[i].cmd = sdsnew(cmd_json->valuestring);
        }
        else
        {
            commands[i].cmd = sdsempty();
        }

        source_json =
            cJSON_GetObjectItemCaseSensitive(complex_command, "source");
        if (cJSON_IsString(source_json) && (source_json->valuestring != NULL))
        {
            commands[i].source = sdsnew(source_json->valuestring);
        }
        else
        {
            commands[i].source = sdsempty();
        }

        store_json = cJSON_GetObjectItemCaseSensitive(complex_command, "store");
        if (cJSON_IsString(store_json) && (store_json->valuestring != NULL))
        {
            commands[i].store = sdsnew(store_json->valuestring);
        }
        else
        {
            commands[i].store = sdsempty();
        }

        lhs_json = cJSON_GetObjectItemCaseSensitive(complex_command, "lhs");
        if (cJSON_IsString(lhs_json) && (lhs_json->valuestring != NULL))
        {
            commands[i].lhs = sdsnew(lhs_json->valuestring);
            commands[i].lhs_type = VARIABLE_STRING;
        }
        else if (cJSON_IsNumber(lhs_json))
        {
            if (lhs_json->valueint == lhs_json->valuedouble)
            {
                commands[i].lhs_int = lhs_json->valueint;
                commands[i].lhs_type = VARIABLE_INT;
            }
            else
            {
                commands[i].lhs_double = lhs_json->valuedouble;
                commands[i].lhs_double = VARIABLE_DOUBLE;
            }
            commands[i].lhs = sdsempty();
        }
        else
        {
            commands[i].lhs = sdsempty();
        }

        rhs_json = cJSON_GetObjectItemCaseSensitive(complex_command, "rhs");
        if (cJSON_IsString(rhs_json) && (rhs_json->valuestring != NULL))
        {
            commands[i].rhs = sdsnew(rhs_json->valuestring);
            commands[i].rhs_type = VARIABLE_STRING;
        }
        else if (cJSON_IsNumber(rhs_json))
        {
            if (rhs_json->valueint == rhs_json->valuedouble)
            {
                commands[i].rhs_int = rhs_json->valueint;
                commands[i].rhs_type = VARIABLE_INT;
            }
            else
            {
                commands[i].rhs_double = rhs_json->valuedouble;
                commands[i].rhs_type = VARIABLE_DOUBLE;
            }
            commands[i].rhs = sdsempty();
        }
        else
        {
            commands[i].rhs = sdsempty();
        }

        index_json = cJSON_GetObjectItemCaseSensitive(complex_command, "index");
        if (cJSON_IsNumber(index_json))
        {
            commands[i].index = index_json->valuedouble;
        }
        else
        {
            commands[i].index = -1;
        }
        i++;
    }

    testcase_obj.exitcode = exitcode;

    for (i = 0; i < commandcount; i++)
    {
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_STRING ||
             !strcmp(commands[i].source, "output")))
            source_string = sdscpy(source_string,
                                   get_source_str(commands[i].source, output));
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_INT))
            source_int = get_source_int(commands[i].source);
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_DOUBLE))
            source_double = get_source_double(commands[i].source);

        if (!strcmp(commands[i].cmd, "run"))
        {
            if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
            {
                start_date =
                    ((int64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
            }
            else
            {
                perror("clock_gettime");
                start_date = 0;
            }
            if (env_count == 0)
                output =
                    execute(program_args, testcase_obj.input, &fault, &exitcode,
                            args.enviromental_values, args.env_count);
            else
                output =
                    execute(program_args, testcase_obj.input, &fault, &exitcode,
                            testcase_obj.enviromental_values, env_count);
            if (clock_gettime(CLOCK_REALTIME, &ts2) == 0)
            {
                end_date =
                    ((int64_t)ts2.tv_sec * 1000) + (ts2.tv_nsec / 1000000);
            }
            else
            {
                perror("clock_gettime");
                end_date = 0;
            }
            duration = end_date - start_date;
            testcase_obj.duration = duration;
            new_variable("output", VARIABLE_STRING, output, -1, -1);
        }
        else if (!strcmp(commands[i].cmd, "extract_char"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING)
            {
                if (strlen(source_string) > commands[i].index)
                    extract_char_character[0] =
                        source_string[commands[i].index];

                if (strcmp(commands[i].store, ""))
                    new_variable(commands[i].store, VARIABLE_STRING,
                                 extract_char_character, -1, -1);
            }
        }
        else if (!strcmp(commands[i].cmd, "atoi"))
        {
            if (strcmp(commands[i].store, ""))
            {
                k = 1;
                if (define_variable_type(commands[i].source) == VARIABLE_STRING)
                {
                    for (size_t j = 0; j < strlen(source_string); j++)
                    {
                        if (!((size_t)(source_string[j]) >= 0x30 &&
                              (size_t)(source_string[j]) <= 0x39))
                        {
                            k = 0;
                            break;
                        }
                    }
                    if (k)
                    {
                        new_variable(commands[i].store, VARIABLE_INT, NULL,
                                     atoi(source_string), -1);
                    }
                }
                else if (define_variable_type(commands[i].source) ==
                         VARIABLE_DOUBLE)
                {
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 (int)source_double, -1);
                }
            }
        }
        else if (!strcmp(commands[i].cmd, "atof"))
        {
            if (strcmp(commands[i].store, ""))
            {
                k = 1;
                if (define_variable_type(commands[i].source) == VARIABLE_STRING)
                {
                    for (size_t j = 0; j < strlen(source_string); j++)
                    {
                        if (!(((size_t)(source_string[j]) >= 0x30 &&
                               (size_t)(source_string[j]) <= 0x39) ||
                              (size_t)(source_string[j] == 0x2e)))
                        {
                            k = 0;
                            break;
                        }
                    }
                    if (k)
                    {
                        new_variable(commands[i].store, VARIABLE_DOUBLE, NULL,
                                     -1, strtod(source_string, &endptr));
                    }
                }
                else if (define_variable_type(commands[i].source) ==
                         VARIABLE_INT)
                {
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 (double)source_int);
                }
            }
        }
        else if (!strcmp(commands[i].cmd, "to_string"))
        {
            if (strcmp(commands[i].store, ""))
            {
                assert_lhs = sdsempty();
                if (define_variable_type(commands[i].source) == VARIABLE_INT)
                {
                    assert_lhs = sdscatprintf(assert_lhs, "%d", source_int);
                }
                else if (define_variable_type(commands[i].source) ==
                         VARIABLE_DOUBLE)
                {
                    assert_lhs = sdscatprintf(assert_lhs, "%f", source_double);
                }
                else
                {
                    assert_lhs = sdscpylen(assert_lhs, source_string,
                                           strlen(source_string));
                }
                new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "to_int"))
        {
            if (strcmp(commands[i].store, "") &&
                define_variable_type(commands[i].source) == VARIABLE_STRING &&
                strlen(source_string) == 1)
            {
                new_variable(commands[i].store, VARIABLE_INT, NULL,
                             (int)(source_string[0]), -1);
            }
        }
        else if (!strcmp(commands[i].cmd, "trim"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING)
            {
                assert_lhs = to_str(get_var_object(commands[i].source, -1, -1,
                                                   VARIABLE_STRING));
                sdstrim(assert_lhs, " \n\t\r");
                new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "upper"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING)
            {
                assert_lhs = to_str(get_var_object(commands[i].source, -1, -1,
                                                   VARIABLE_STRING));
                for (tmp = 0; tmp < sdslen(assert_lhs); tmp++)
                {
                    if (assert_lhs[tmp] >= 0x61 && assert_lhs[tmp] <= 0x7a)
                    {
                        assert_lhs[tmp] -= 0x20;
                    }
                }
                new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "lower"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING)
            {
                assert_lhs = to_str(get_var_object(commands[i].source, -1, -1,
                                                   VARIABLE_STRING));
                for (tmp = 0; tmp < sdslen(assert_lhs); tmp++)
                {
                    if (assert_lhs[tmp] >= 0x41 && assert_lhs[tmp] <= 0x5a)
                    {
                        assert_lhs[tmp] += 0x20;
                    }
                }
                new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "add"))
        {
            if (strcmp(commands[i].store, ""))
            {
                if (define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                    define_variable_type(commands[i].rhs) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 get_source_int(commands[i].lhs) +
                                     get_source_int(commands[i].rhs),
                                 -1);
                if (define_variable_type(commands[i].lhs) == VARIABLE_DOUBLE &&
                    define_variable_type(commands[i].rhs) == VARIABLE_DOUBLE)
                    new_variable(commands[i].store, VARIABLE_INT, NULL, -1,
                                 get_source_double(commands[i].lhs) +
                                     get_source_double(commands[i].rhs));
            }
        }
        else if (!strcmp(commands[i].cmd, "subtract"))
        {
            if (strcmp(commands[i].store, ""))
            {
                if (define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                    define_variable_type(commands[i].rhs) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 get_source_int(commands[i].lhs) -
                                     get_source_int(commands[i].rhs),
                                 -1);
                if (define_variable_type(commands[i].lhs) == VARIABLE_DOUBLE &&
                    define_variable_type(commands[i].rhs) == VARIABLE_DOUBLE)
                    new_variable(commands[i].store, VARIABLE_INT, NULL, -1,
                                 get_source_double(commands[i].lhs) -
                                     get_source_double(commands[i].rhs));
            }
        }
        else if (!strcmp(commands[i].cmd, "multiply"))
        {
            if (strcmp(commands[i].store, ""))
            {
                if (define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                    define_variable_type(commands[i].rhs) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 get_source_int(commands[i].lhs) *
                                     get_source_int(commands[i].rhs),
                                 -1);
                if (define_variable_type(commands[i].lhs) == VARIABLE_DOUBLE &&
                    define_variable_type(commands[i].rhs) == VARIABLE_DOUBLE)
                    new_variable(commands[i].store, VARIABLE_INT, NULL, -1,
                                 get_source_double(commands[i].lhs) *
                                     get_source_double(commands[i].rhs));
            }
        }
        else if (!strcmp(commands[i].cmd, "divide"))
        {
            if (strcmp(commands[i].store, ""))
            {
                if (define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                    define_variable_type(commands[i].rhs) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 get_source_int(commands[i].lhs) /
                                     get_source_int(commands[i].rhs));
                if (define_variable_type(commands[i].lhs) == VARIABLE_DOUBLE &&
                    define_variable_type(commands[i].rhs) == VARIABLE_DOUBLE)
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 get_source_double(commands[i].lhs) *
                                     get_source_double(commands[i].rhs));
                if (define_variable_type(commands[i].lhs) == VARIABLE_DOUBLE &&
                    define_variable_type(commands[i].rhs) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 get_source_double(commands[i].lhs) *
                                     get_source_int(commands[i].rhs));
                if (define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                    define_variable_type(commands[i].rhs) == VARIABLE_DOUBLE)
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 get_source_int(commands[i].lhs) *
                                     get_source_double(commands[i].rhs));
            }
        }
        else if (!strcmp(commands[i].cmd, "mod"))
        {
            if (strcmp(commands[i].store, "") &&
                define_variable_type(commands[i].lhs) == VARIABLE_INT &&
                define_variable_type(commands[i].rhs) == VARIABLE_INT &&
                get_source_int(commands[i].rhs) != 0)
            {
                new_variable(commands[i].store, VARIABLE_INT, NULL,
                             get_source_int(commands[i].lhs) %
                                 get_source_int(commands[i].rhs),
                             -1);
            }
        }
        else if (!strcmp(commands[i].cmd, "set"))
        {
            if (commands[i].lhs_type == VARIABLE_STRING)
            {
                assert_lhs = sdsempty();
                if (commands[i].rhs_type == VARIABLE_STRING)
                    assert_lhs = sdscpylen(assert_lhs, commands[i].rhs,
                                           strlen(commands[i].rhs));
                else if (commands[i].rhs_type == VARIABLE_INT)
                    assert_lhs =
                        sdscatprintf(assert_lhs, "%d", commands[i].rhs_int);
                else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                    assert_lhs =
                        sdscatprintf(assert_lhs, "%f", commands[i].rhs_double);
                new_variable(commands[i].lhs, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "assert_equals") ||
                 !strcmp(commands[i].cmd, "assert_not_equals") ||
                 !strcmp(commands[i].cmd, "assert_regex_match") ||
                 !strcmp(commands[i].cmd, "assert_not_regex_match") ||
                 !strcmp(commands[i].cmd, "assert_contains") ||
                 !strcmp(commands[i].cmd, "assert_not_contains") ||
                 !strcmp(commands[i].cmd, "assert_starts_with") ||
                 !strcmp(commands[i].cmd, "assert_not_starts_with") ||
                 !strcmp(commands[i].cmd, "assert_ends_with") ||
                 !strcmp(commands[i].cmd, "assert_not_ends_with") ||
                 !strcmp(commands[i].cmd, "assert_file_exists") ||
                 !strcmp(commands[i].cmd, "assert_file_not_exists") ||
                 !strcmp(commands[i].cmd, "assert_file_contains") ||
                 !strcmp(commands[i].cmd, "assert_file_not_contains"))
        {
            if (commands[i].lhs_type == VARIABLE_STRING)
                assert_lhs = to_str(
                    get_var_object(commands[i].lhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].lhs_type == VARIABLE_INT)
                assert_lhs = to_str(get_var_object(NULL, commands[i].lhs_int,
                                                   -1, VARIABLE_INT));
            else if (commands[i].lhs_type == VARIABLE_DOUBLE)
                assert_lhs = to_str(get_var_object(
                    NULL, -1, commands[i].lhs_double, VARIABLE_DOUBLE));
            else
                assert_lhs = sdsempty();

            if (commands[i].rhs_type == VARIABLE_STRING)
                assert_rhs = to_str(
                    get_var_object(commands[i].rhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].rhs_type == VARIABLE_INT)
                assert_rhs = to_str(get_var_object(NULL, commands[i].rhs_int,
                                                   -1, VARIABLE_INT));
            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                assert_rhs = to_str(get_var_object(
                    NULL, -1, commands[i].rhs_double, VARIABLE_DOUBLE));
            else
                assert_rhs = sdsempty();

            if (!strcmp(commands[i].cmd, "assert_equals"))
            {
                if (!strcmp(assert_lhs, assert_rhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_not_equals"))
            {
                if (strcmp(assert_lhs, assert_rhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_regex_match"))
            {
                if (regex_pass(assert_rhs, assert_lhs, sdslen(assert_lhs)))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_not_regex_match"))
            {
                if (!regex_pass(assert_rhs, assert_lhs, sdslen(assert_lhs)))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_contains"))
            {
                if (strstr(assert_lhs, assert_rhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_not_contains"))
            {
                if (!strstr(assert_lhs, assert_rhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_starts_with"))
            {
                if (!strncmp(assert_lhs, assert_rhs, strlen(assert_rhs)))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_not_starts_with"))
            {
                if (strncmp(assert_lhs, assert_rhs, strlen(assert_rhs)))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_ends_with"))
            {
                if ((!(strlen(assert_rhs) > strlen(assert_lhs))) &&
                    (!(strncmp(assert_lhs + strlen(assert_lhs) -
                                   strlen(assert_rhs),
                               assert_rhs, strlen(assert_rhs)))))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_not_ends_with"))
            {
                if (!((!(strlen(assert_rhs) > strlen(assert_lhs))) &&
                      (!(strncmp(assert_lhs + strlen(assert_lhs) -
                                     strlen(assert_rhs),
                                 assert_rhs, strlen(assert_rhs))))))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_file_exists"))
            {
                if (file_exists(assert_lhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_file_not_exists"))
            {
                if (!file_exists(assert_lhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_file_contains") ||
                     !strcmp(commands[i].cmd, "assert_file_not_contains"))
            {
                if (!file_exists(assert_lhs))
                {
                    result = 0;
                    sdsfree(assert_lhs);
                    sdsfree(assert_rhs);
                    continue;
                }
                fptr = fopen(assert_lhs, "rt");
                if (fptr == NULL)
                {
                    perror("fopen");
                    result = 0;
                    sdsfree(assert_lhs);
                    sdsfree(assert_rhs);
                    continue;
                }
                fseek(fptr, 0, SEEK_END);
                k = ftell(fptr);
                fseek(fptr, 0, SEEK_SET);
                if (k < 0)
                {
                    perror("ftell");
                    fclose(fptr);
                    sdsfree(assert_lhs);
                    sdsfree(assert_rhs);
                    continue;
                }

                file_content = sdsempty();
                file_content = sdsgrowzero(file_content, k);
                fread(file_content, 1, k, fptr);
                if (!strcmp(commands[i].cmd, "assert_file_contains"))
                {
                    if (strstr(file_content, assert_rhs))
                    {
                        result = 1;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    if (strstr(file_content, assert_rhs) == NULL)
                    {
                        result = 1;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                sdsfree(file_content);
                fclose(fptr);
            }

            sdsfree(assert_lhs);
            sdsfree(assert_rhs);
        }
        else if (!strcmp(commands[i].cmd, "assert_less") ||
                 !strcmp(commands[i].cmd, "assert_greater") ||
                 !strcmp(commands[i].cmd, "assert_less_or_equal") ||
                 !strcmp(commands[i].cmd, "assert_greater_or_equal"))
        {
            if (commands[i].lhs_type == VARIABLE_STRING)
                assert_lhs_double = to_double(
                    get_var_object(commands[i].lhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].lhs_type == VARIABLE_INT)
                assert_lhs_double = to_double(get_var_object(
                    NULL, commands[i].lhs_int, -1, VARIABLE_INT));
            else if (commands[i].lhs_type == VARIABLE_DOUBLE)
                assert_lhs_double = to_double(get_var_object(
                    NULL, -1, commands[i].lhs_double, VARIABLE_DOUBLE));

            if (commands[i].rhs_type == VARIABLE_STRING)
                assert_rhs_double = to_double(
                    get_var_object(commands[i].rhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].rhs_type == VARIABLE_INT)
                assert_rhs_double = to_double(get_var_object(
                    NULL, commands[i].rhs_int, -1, VARIABLE_INT));
            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                assert_rhs_double = to_double(get_var_object(
                    NULL, -1, commands[i].rhs_double, VARIABLE_DOUBLE));

            if (!strcmp(commands[i].cmd, "assert_less"))
            {
                if (assert_lhs_double < assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_greater"))
            {
                if (assert_lhs_double > assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_less_or_equal"))
            {
                if (assert_lhs_double <= assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_greater_or_equal"))
            {
                if (assert_lhs_double >= assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
        }
        else if (!strcmp(commands[i].cmd, "assert_exitcode") ||
                 !strcmp(commands[i].cmd, "assert_exitcode_less") ||
                 !strcmp(commands[i].cmd, "assert_exitcode_greater"))
        {
            assert_lhs_double = exitcode;

            if (commands[i].lhs_type == VARIABLE_STRING)
                assert_rhs_double = to_double(
                    get_var_object(commands[i].lhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].lhs_type == VARIABLE_INT)
                assert_rhs_double = to_double(get_var_object(
                    NULL, commands[i].lhs_int, -1, VARIABLE_INT));
            else if (commands[i].lhs_type == VARIABLE_DOUBLE)
                assert_rhs_double = to_double(get_var_object(
                    NULL, -1, commands[i].lhs_double, VARIABLE_DOUBLE));

            if (!strcmp(commands[i].cmd, "assert_exitcode"))
            {
                if (assert_lhs_double == assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_exitcode_less"))
            {
                if (assert_lhs_double < assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_exitcode_greater"))
            {
                if (assert_lhs_double > assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
        }
        else if (!strcmp(commands[i].cmd, "assert_duration_less_than") ||
                 !strcmp(commands[i].cmd, "assert_duration_greater_than"))
        {
            assert_lhs_double = duration;

            if (commands[i].lhs_type == VARIABLE_STRING)
                assert_rhs_double = to_double(
                    get_var_object(commands[i].lhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].lhs_type == VARIABLE_INT)
                assert_rhs_double = to_double(get_var_object(
                    NULL, commands[i].lhs_int, -1, VARIABLE_INT));
            else if (commands[i].lhs_type == VARIABLE_DOUBLE)
                assert_rhs_double = to_double(get_var_object(
                    NULL, -1, commands[i].lhs_double, VARIABLE_DOUBLE));

            if (!strcmp(commands[i].cmd, "assert_duration_less_than"))
            {
                if (assert_lhs_double < assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else if (!strcmp(commands[i].cmd, "assert_duration_greater_than"))
            {
                if (assert_lhs_double > assert_rhs_double)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
        }
    }
    reason = sdscpylen(reason, "Assertion failed.", 17);
    print_output(testcase_obj, result, &reason, &output);
    sdsfree(testcase_obj.name);
    sdsfree(testcase_obj.description);
    sdsfree(testcase_obj.input);
    sdsfree(testcase_obj.validationtype);
    sdsfree(cmd);
    sdsfree(source_string);
    for (int j = 0; j < program_args_length; j++)
    {
        free(program_args[j]);
    }
    free(program_args);
    sdsfree(output);
    for (int j = 0; j < env_count; j++)
    {
        sdsfree(testcase_obj.enviromental_values[j]);
    }
    free(testcase_obj.enviromental_values);
    for (int j = 0; j < variable_count; j++)
    {
        sdsfree(variables[j].name);
        sdsfree(variables[j].valuestring);
    }
    if (variable_count > 0)
        free(variables);

    for (int j = 0; j < commandcount; j++)
    {
        sdsfree(commands[j].cmd);
        sdsfree(commands[j].source);
        sdsfree(commands[j].store);
        sdsfree(commands[j].lhs);
        sdsfree(commands[j].rhs);
    }
    free(commands);

    return result;
}

void new_variable(char *name, int type, char *valuestring, int valueint,
                  double valuedouble)
{
    if (variable_count == 0)
    {
        variables = malloc(sizeof(variable));
        variable_count++;
    }
    else
    {
        variables = realloc(variables, sizeof(variable) * ++variable_count);
    }

    variables[variable_count - 1].type = type;
    variables[variable_count - 1].name = sdsnew(name);
    variables[variable_count - 1].empty = 0;
    if (type == VARIABLE_STRING)
    {
        variables[variable_count - 1].valuestring = sdsnew(valuestring);
        variables[variable_count - 1].valueint = -1;
        variables[variable_count - 1].valuedouble = -1;
    }
    else if (type == VARIABLE_INT)
    {
        variables[variable_count - 1].valuestring = NULL;
        variables[variable_count - 1].valueint = valueint;
        variables[variable_count - 1].valuedouble = -1;
    }
    else if (type == VARIABLE_DOUBLE)
    {
        variables[variable_count - 1].valuestring = NULL;
        variables[variable_count - 1].valueint = -1;
        variables[variable_count - 1].valuedouble = valuedouble;
    }
}

char *get_source_str(char *source, char *output)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return output;

    sds varname;
    int i;
    int index = -1;
    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return source;
    return variables[index].valuestring;
}

int get_source_int(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname;
    int i;
    int index = -1;

    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }

    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return atoi(source);
    return variables[index].valueint;
}

double get_source_double(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname;
    int i;
    int index = -1;
    char *endptr;

    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }

    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return strtod(source, &endptr);
    return variables[index].valuedouble;
}

int define_variable_type(char *varname)
{
    sds variablename;
    int i;
    size_t j;
    int type = -1;
    int dot_seen = 0;
    int letter_seen = 0;
    if (!strcmp(varname, ""))
        return -1;
    if (varname[0] == '{' && varname[1] == '{' &&
        varname[strlen(varname) - 1] == '}' &&
        varname[strlen(varname) - 2] == '}')
    {
        variablename = sdsnewlen(varname + 2, strlen(varname) - 4);
    }
    else
    {
        variablename = sdsnew(varname);
    }

    if (!strcmp(varname, "{{output}}") || !strcmp(varname, "output"))
    {
        sdsfree(variablename);
        return VARIABLE_STRING;
    }
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, variablename))
        {
            type = variables[i].type;
        }
    }
    sdsfree(variablename);
    if (type == -1)
    {
        for (j = 0; j < strlen(varname); j++)
        {
            if (varname[j] == '.')
            {
                dot_seen = 1;
                continue;
            }
            if (!(varname[j] >= 0x30 && varname[j] <= 0x39))
            {
                letter_seen = 1;
            }
        }
        if (letter_seen)
            return VARIABLE_STRING;
        else if (dot_seen)
            return VARIABLE_DOUBLE;
        else
            return VARIABLE_INT;
    }
    return type;
}

sds to_str(variable var)
{
    sds stringified_str = sdsempty();
    if (var.type == VARIABLE_STRING)
    {
        stringified_str = sdscpylen(stringified_str, var.valuestring,
                                    strlen(var.valuestring));
    }
    else if (var.type == VARIABLE_INT)
    {
        stringified_str = sdscatprintf(stringified_str, "%d", var.valueint);
    }
    else if (var.type == VARIABLE_DOUBLE)
    {
        stringified_str = sdscatprintf(stringified_str, "%f", var.valuedouble);
    }

    destroy_empty_variable(var);
    return stringified_str;
}

double to_double(variable var)
{
    double converted;
    char *end_ptr;
    if (var.type == VARIABLE_STRING)
    {
        converted = strtod(var.valuestring, &end_ptr);
        destroy_empty_variable(var);
        return converted;
    }
    else if (var.type == VARIABLE_INT)
    {
        destroy_empty_variable(var);
        return var.valueint;
    }
    else if (var.type == VARIABLE_DOUBLE)
    {
        destroy_empty_variable(var);
        return var.valuedouble;
    }

    destroy_empty_variable(var);
    return -1;
}

variable get_var_object(char *name, int name_int, double name_double, int type)
{
    variable emptyobj = {-1, NULL, NULL, -1, -1, 1};
    sds namesds;
    int found = 0;
    int i;
    int name_type;
    char *endptr;

    if (name == NULL)
    {
        if (type == VARIABLE_INT)
        {
            emptyobj.valueint = name_int;
        }
        else if (type == VARIABLE_DOUBLE)
        {
            emptyobj.valuedouble = name_double;
        }
        emptyobj.type = type;
        return emptyobj;
    }
    if (name[0] == '{' && name[1] == '{' && name[strlen(name) - 1] == '}' &&
        name[strlen(name) - 2] == '}')
        namesds = sdsnewlen(name + 2, strlen(name) - 4);
    else
        namesds = sdsnewlen(name, strlen(name));
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(namesds, variables[i].name))
        {
            found = 1;
            break;
        }
    }

    sdsfree(namesds);
    if (found)
        return variables[i];
    else
    {
        name_type = define_variable_type(name);
        if (name_type == VARIABLE_STRING)
        {
            emptyobj.valuestring = sdsnewlen(name, strlen(name));
            emptyobj.valueint = -1;
            emptyobj.valuedouble = -1;
        }
        else if (name_type == VARIABLE_INT)
        {
            emptyobj.valuestring = sdsempty();
            emptyobj.valueint = atoi(name);
            emptyobj.valuedouble = -1;
        }
        else if (name_type == VARIABLE_DOUBLE)
        {
            emptyobj.valuestring = sdsempty();
            emptyobj.valueint = -1;
            emptyobj.valuedouble = strtod(name, &endptr);
        }
        emptyobj.type = name_type;
        return emptyobj;
    }
}
