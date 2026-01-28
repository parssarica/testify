/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <math.h>
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
    int fault = -1;
    int exitcode = -1;
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
    cJSON *value_json;
    cJSON *background_json;
    char extract_char_character[2];
    sds cmd = sdsempty();
    sds source_string = sdsempty();
    sds assert_lhs;
    sds assert_rhs;
    sds file_content;
    sds emptyvar = sdsempty();
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
    int found;
    sds varname;
    sds varname2;
    variable *vars_tmp;
    int var1_index;
    int var2_index;
    process pr;
    ssize_t n;
    char output_buf[4096];
    int ran_background = 0;
    int assert_count = 0;

    memset(output_buf, 0, 4096);
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
                                    testcase_json, "environmentalVariables"))
    {
        env_count++;
    }

    testcase_obj.environmental_values = malloc(sizeof(sds *) * env_count);
    i = 0;
    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcase_json, "environmentalVariables"))
    {
        if (cJSON_IsString(env_var) && (env_var->valuestring != NULL))
        {
            testcase_obj.environmental_values[i] = sdsnew(env_var->valuestring);
            i++;
        }
    }

    commandcount = 0;
    i = 0;
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
            commands[i].lhs_int = 0;
            commands[i].lhs_double = 0;
            commands[i].lhs_type = VARIABLE_STRING;
        }
        else if (cJSON_IsNumber(lhs_json))
        {
            if (lhs_json->valueint == lhs_json->valuedouble)
            {
                commands[i].lhs_int = lhs_json->valueint;
                commands[i].lhs_double = 0;
                commands[i].lhs_type = VARIABLE_INT;
            }
            else
            {
                commands[i].lhs_double = lhs_json->valuedouble;
                commands[i].lhs_int = 0;
                commands[i].lhs_double = VARIABLE_DOUBLE;
            }
            commands[i].lhs = sdsempty();
        }
        else
        {
            commands[i].lhs = sdsempty();
            commands[i].lhs_int = 0;
            commands[i].lhs_double = 0;
            commands[i].lhs_type = VARIABLE_STRING;
        }

        rhs_json = cJSON_GetObjectItemCaseSensitive(complex_command, "rhs");
        if (cJSON_IsString(rhs_json) && (rhs_json->valuestring != NULL))
        {
            commands[i].rhs = sdsnew(rhs_json->valuestring);
            commands[i].rhs_int = 0;
            commands[i].rhs_double = 0;
            commands[i].rhs_type = VARIABLE_STRING;
        }
        else if (cJSON_IsNumber(rhs_json))
        {
            if (rhs_json->valueint == rhs_json->valuedouble)
            {
                commands[i].rhs_int = rhs_json->valueint;
                commands[i].rhs_double = 0;
                commands[i].rhs_type = VARIABLE_INT;
            }
            else
            {
                commands[i].rhs_double = rhs_json->valuedouble;
                commands[i].rhs_int = 0;
                commands[i].rhs_type = VARIABLE_DOUBLE;
            }
            commands[i].rhs = sdsempty();
        }
        else
        {
            commands[i].rhs = sdsempty();
            commands[i].rhs_int = 0;
            commands[i].rhs_double = 0;
            commands[i].rhs_type = VARIABLE_STRING;
        }

        index_json = cJSON_GetObjectItemCaseSensitive(complex_command, "index");
        if (cJSON_IsNumber(index_json))
        {
            commands[i].index = index_json->valuedouble;
        }
        else
        {
            commands[i].index = 0;
        }

        k = 0;
        cJSON_ArrayForEach(value_json, cJSON_GetObjectItemCaseSensitive(
                                           complex_command, "values"))
        {
            k++;
        }

        commands[i].value_count = k;
        if (k)
        {
            commands[i].values = malloc(sizeof(sds) * k);
            k = 0;
            cJSON_ArrayForEach(value_json, cJSON_GetObjectItemCaseSensitive(
                                               complex_command, "values"))
            {
                commands[i].values[k++] = sdsnew(value_json->valuestring);
            }
        }

        background_json =
            cJSON_GetObjectItemCaseSensitive(complex_command, "background");
        commands[i].background = 0;
        if (cJSON_IsBool(background_json))
        {
            if (cJSON_IsTrue(background_json))
            {
                commands[i].background = 1;
                ran_background = 1;
            }
            else
            {
                commands[i].background = 0;
            }
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
            if (!commands[i].background)
            {
                if (env_count == 0)
                    output = execute(program_args, testcase_obj.input, &fault,
                                     &exitcode, args.environmental_values,
                                     args.env_count);
                else
                    output = execute(
                        program_args, testcase_obj.input, &fault, &exitcode,
                        testcase_obj.environmental_values, env_count);
            }
            else
            {
                if (env_count == 0)
                    pr = execute_background(program_args,
                                            args.environmental_values,
                                            args.env_count);
                else
                    pr = execute_background(program_args,
                                            testcase_obj.environmental_values,
                                            env_count);
            }
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
            if (!commands[i].background)
                new_variable("output", VARIABLE_STRING, output, -1, -1);
        }
        else if (!strcmp(commands[i].cmd, "send_input"))
        {
            interact_write(&pr, &source_string);
        }
        else if (!strcmp(commands[i].cmd, "send_line"))
        {
            source_string = sdscatlen(source_string, "\n", 1);
            interact_write(&pr, &source_string);
        }
        else if (!strcmp(commands[i].cmd, "wait_for_output"))
        {
            if (source_int)
                n = interact_read(&pr, output_buf, 4096, source_int);
            else
                n = interact_read(&pr, output_buf, 4096, 200);

            if (n <= 0)
                close_child(&pr, &fault, &exitcode);

            new_variable(commands[i].store, VARIABLE_STRING, output_buf, -1,
                         -1);
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
        else if (!strcmp(commands[i].cmd, "min"))
        {
            if (commands[i].value_count > 0)
            {
                assert_lhs_double = get_source_non_char(commands[i].values[0]);
                for (int j = 0; j < commands[i].value_count; j++)
                {
                    if (assert_lhs_double >
                        get_source_non_char(commands[i].values[j]))
                    {
                        assert_lhs_double =
                            get_source_non_char(commands[i].values[j]);
                    }
                }
                new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                             assert_lhs_double);
            }
        }
        else if (!strcmp(commands[i].cmd, "max"))
        {
            if (commands[i].value_count > 0)
            {
                assert_lhs_double = get_source_non_char(commands[i].values[0]);
                for (int j = 0; j < commands[i].value_count; j++)
                {
                    if (assert_lhs_double <
                        get_source_non_char(commands[i].values[j]))
                    {
                        assert_lhs_double =
                            get_source_non_char(commands[i].values[j]);
                    }
                }
                new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                             assert_lhs_double);
            }
        }
        else if (!strcmp(commands[i].cmd, "sum"))
        {
            if (commands[i].value_count > 0)
            {
                assert_lhs_double = 0;
                for (int j = 0; j < commands[i].value_count; j++)
                {
                    assert_lhs_double +=
                        get_source_non_char(commands[i].values[j]);
                }
                new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                             assert_lhs_double);
            }
        }
        else if (!strcmp(commands[i].cmd, "increment"))
        {
            if (strcmp(commands[i].store, "") &&
                (define_variable_type(commands[i].source) == VARIABLE_INT ||
                 define_variable_type(commands[i].source) == VARIABLE_DOUBLE))
            {
                assert_lhs_double = get_source_non_char(commands[i].source);
                assert_lhs_double++;
                if (define_variable_type(commands[i].source) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 assert_lhs_double, -1);
                else
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 assert_lhs_double);
            }
        }
        else if (!strcmp(commands[i].cmd, "decrement"))
        {
            if (strcmp(commands[i].store, "") &&
                (define_variable_type(commands[i].source) == VARIABLE_INT ||
                 define_variable_type(commands[i].source) == VARIABLE_DOUBLE))
            {
                assert_lhs_double = get_source_non_char(commands[i].source);
                assert_lhs_double--;
                if (define_variable_type(commands[i].source) == VARIABLE_INT)
                    new_variable(commands[i].store, VARIABLE_INT, NULL,
                                 assert_lhs_double, -1);
                else
                    new_variable(commands[i].store, VARIABLE_DOUBLE, NULL, -1,
                                 assert_lhs_double);
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
                found = 0;
                varname = sdsempty();
                if (commands[i].lhs[0] == '{' && commands[i].lhs[1] == '{' &&
                    commands[i].lhs[strlen(commands[i].lhs) - 1] == '}' &&
                    commands[i].lhs[strlen(commands[i].lhs) - 2] == '}')
                    varname = sdscpylen(varname, commands[i].lhs + 2,
                                        sdslen(commands[i].lhs) - 4);
                else
                    varname = sdscpylen(varname, commands[i].lhs,
                                        sdslen(commands[i].lhs));

                for (k = 0; k < variable_count; k++)
                {
                    if (!strcmp(variables[k].name, varname))
                    {
                        found = 1;
                        if (variables[k].type == VARIABLE_STRING)
                        {
                            variables[k].valuestring =
                                sdscpylen(variables[k].valuestring, "", 0);
                            if (commands[i].rhs_type == VARIABLE_STRING)
                                variables[k].valuestring =
                                    sdscatprintf(variables[k].valuestring, "%s",
                                                 commands[i].rhs);
                            else if (commands[i].rhs_type == VARIABLE_INT)
                                variables[k].valuestring =
                                    sdscatprintf(variables[k].valuestring, "%d",
                                                 commands[i].rhs_int);
                            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                                variables[k].valuestring =
                                    sdscatprintf(variables[k].valuestring, "%f",
                                                 commands[i].rhs_double);
                        }
                        else if (variables[k].type == VARIABLE_INT)
                        {
                            if (commands[i].rhs_type == VARIABLE_STRING)
                                variables[k].valueint = atoi(commands[i].rhs);
                            else if (commands[i].rhs_type == VARIABLE_INT)
                                variables[k].valueint = commands[i].rhs_int;
                            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                                variables[k].valueint =
                                    (int)commands[i].rhs_double;
                        }
                        else if (variables[k].type == VARIABLE_DOUBLE)
                        {
                            if (commands[i].rhs_type == VARIABLE_STRING)
                                variables[k].valuedouble =
                                    strtod(commands[i].rhs, &endptr);
                            else if (commands[i].rhs_type == VARIABLE_INT)
                                variables[k].valuedouble =
                                    commands[i].rhs_double;
                            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                                variables[k].valuedouble =
                                    (double)commands[i].rhs_double;
                        }
                    }
                }
                if (!found)
                {
                    if (commands[i].rhs_type == VARIABLE_STRING)
                        new_variable(commands[i].lhs, VARIABLE_STRING,
                                     commands[i].rhs, -1, -1);
                    else if (commands[i].rhs_type == VARIABLE_INT)
                        new_variable(commands[i].lhs, VARIABLE_INT, NULL,
                                     commands[i].rhs_int, -1);
                    else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                        new_variable(commands[i].lhs, VARIABLE_INT, NULL, -1,
                                     commands[i].rhs_double);
                }
                sdsfree(varname);
            }
        }
        else if (!strcmp(commands[i].cmd, "clear"))
        {
            found = 0;
            varname = sdsempty();
            if (commands[i].source[0] == '{' && commands[i].source[1] == '{' &&
                commands[i].source[strlen(commands[i].source) - 1] == '}' &&
                commands[i].source[strlen(commands[i].source) - 2] == '}')
                varname = sdscpylen(varname, commands[i].source + 2,
                                    sdslen(commands[i].source) - 4);
            else
                varname = sdscpylen(varname, commands[i].source,
                                    sdslen(commands[i].source));

            for (k = 0; k < variable_count; k++)
            {
                if (!strcmp(varname, variables[k].name))
                {
                    found = 1;
                    break;
                }
            }
            if (found)
            {
                vars_tmp = malloc(sizeof(variable) * (variable_count - 1));
                found = 0;
                for (int j = 0; j < variable_count; j++)
                {
                    if (j == k)
                    {
                        found = 1;
                        continue;
                    }
                    vars_tmp[j - found].name = sdsnew(variables[j].name);
                    vars_tmp[j - found].valuestring =
                        sdsnew(variables[j].valuestring);
                    vars_tmp[j - found].type = variables[j].type;
                    vars_tmp[j - found].valueint = variables[j].valueint;
                    vars_tmp[j - found].valuedouble = variables[j].valuedouble;
                    vars_tmp[j - found].empty = variables[j].empty;
                }
                for (int j = 0; j < variable_count; j++)
                {
                    sdsfree(variables[j].name);
                    sdsfree(variables[j].valuestring);
                }
                free(variables);
                variables = vars_tmp;
                vars_tmp = NULL;
                variable_count--;
            }
            sdsfree(varname);
        }
        else if (!strcmp(commands[i].cmd, "copy"))
        {
            if (!(commands[i].lhs_type == VARIABLE_STRING &&
                  commands[i].rhs_type == VARIABLE_STRING))
                continue;

            var1_index = -1;
            var2_index = -1;
            varname = sdsempty();
            varname2 = sdsempty();
            if (commands[i].lhs[0] == '{' && commands[i].lhs[1] == '{' &&
                commands[i].lhs[strlen(commands[i].lhs) - 1] == '}' &&
                commands[i].lhs[strlen(commands[i].lhs) - 2] == '}')
                varname = sdscpylen(varname, commands[i].lhs + 2,
                                    sdslen(commands[i].lhs) - 4);
            else
                varname = sdscpylen(varname, commands[i].lhs,
                                    sdslen(commands[i].lhs));

            if (commands[i].rhs[0] == '{' && commands[i].rhs[1] == '{' &&
                commands[i].rhs[strlen(commands[i].rhs) - 1] == '}' &&
                commands[i].rhs[strlen(commands[i].rhs) - 2] == '}')
                varname2 = sdscpylen(varname2, commands[i].rhs + 2,
                                     sdslen(commands[i].rhs) - 4);
            else
                varname2 = sdscpylen(varname2, commands[i].rhs,
                                     sdslen(commands[i].rhs));

            for (k = 0; k < variable_count; k++)
            {
                if (!strcmp(variables[k].name, varname))
                    var1_index = k;
                if (!strcmp(variables[k].name, varname2))
                    var2_index = k;
            }
            if (var1_index == -1 || var2_index == -1)
            {
                sdsfree(varname);
                sdsfree(varname2);
                continue;
            }
            sdsfree(variables[var1_index].name);
            sdsfree(variables[var1_index].valuestring);
            variables[var1_index].name = sdsnew(variables[var2_index].name);
            variables[var1_index].valuestring =
                sdsnew(variables[var2_index].valuestring);
            variables[var1_index].type = variables[var2_index].type;
            variables[var1_index].valueint = variables[var2_index].valueint;
            variables[var1_index].valuedouble =
                variables[var2_index].valuedouble;
            variables[var1_index].empty = variables[var2_index].empty;
            sdsfree(varname);
            sdsfree(varname2);
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
            assert_count++;
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
            assert_count++;
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
            assert_count++;
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
            assert_count++;
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
        else if (!strcmp(commands[i].cmd, "log"))
        {
            if (commands[i].source[0] == '{' && commands[i].source[1] == '{' &&
                commands[i].source[strlen(commands[i].source) - 1] == '}' &&
                commands[i].source[strlen(commands[i].source) - 2] == '}')
                printf("Variable name: %s\n", commands[i].source);
            else
            {
                printf("Value: %s\n", commands[i].source);
                continue;
            }
            if (define_variable_type(commands[i].source) == VARIABLE_INT)
            {
                printf("Value: %d\n", source_int);
            }
            else if (define_variable_type(commands[i].source) ==
                     VARIABLE_DOUBLE)
            {
                printf("Value: %f\n", source_double);
            }
            else
            {
                printf("Value: %s\n", source_string);
            }
        }
        else if (!strcmp(commands[i].cmd, "dump"))
        {
            for (k = 0; k < variable_count; k++)
            {
                printf("Variable %d:\n", k);
                printf("\tVariable name: %s\n", variables[k].name);
                if (variables[k].type == VARIABLE_STRING)
                    printf("\tVariable value: %s\n\n",
                           variables[k].valuestring);
                else if (variables[k].type == VARIABLE_INT)
                    printf("\tVariable value: %d\n\n", variables[k].valueint);
                else if (variables[k].type == VARIABLE_DOUBLE)
                    printf("\tVariable value: %f\n\n",
                           variables[k].valuedouble);
            }
        }
        else if (!strcmp(commands[i].cmd, "extract_substring"))
        {
            if (commands[i].lhs_type == VARIABLE_STRING)
                var1_index = (int)to_double(
                    get_var_object(commands[i].lhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].lhs_type == VARIABLE_INT)
                var1_index = (int)to_double(get_var_object(
                    NULL, commands[i].lhs_int, -1, VARIABLE_INT));
            else if (commands[i].lhs_type == VARIABLE_DOUBLE)
                var1_index = (int)to_double(get_var_object(
                    NULL, -1, commands[i].lhs_double, VARIABLE_DOUBLE));
            else
                var1_index = 0;

            if (commands[i].rhs_type == VARIABLE_STRING)
                var2_index = (int)to_double(
                    get_var_object(commands[i].rhs, -1, -1, VARIABLE_STRING));
            else if (commands[i].rhs_type == VARIABLE_INT)
                var2_index = (int)to_double(get_var_object(
                    NULL, commands[i].rhs_int, -1, VARIABLE_INT));
            else if (commands[i].rhs_type == VARIABLE_DOUBLE)
                var2_index = (int)to_double(get_var_object(
                    NULL, -1, commands[i].rhs_double, VARIABLE_DOUBLE));
            else
                var2_index = 0;

            if (var1_index < 0)
                var1_index = strlen(source_string) - (var1_index * -1);

            if (var2_index < 0)
                var2_index = strlen(source_string) - (var2_index * -1);

            if (var1_index > var2_index)
            {
                k = var2_index;
                var2_index = var1_index;
                var1_index = k;
            }

            assert_lhs = sdsnewlen(source_string + var1_index,
                                   var2_index - var1_index + 1);
            new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                         -1);
            sdsfree(assert_lhs);
        }
        else if (!strcmp(commands[i].cmd, "extract_regex"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING &&
                commands[i].lhs_type == VARIABLE_STRING &&
                commands[i].rhs_type == VARIABLE_INT)
            {
                assert_lhs =
                    regex_extract(commands[i].lhs, source_string,
                                  sdslen(source_string), commands[i].rhs_int);
                new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                             -1);
                sdsfree(assert_lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "get_line"))
        {
            k = 0;
            for (int j = 0; j < commands[i].lhs_int; j++)
            {
                while (source_string[k++] != '\n')
                    ;
            }
            var1_index = k;
            var2_index = k + 1;
            while (source_string[var2_index] != '\n')
                var2_index++;
            assert_lhs = sdsnewlen(source_string + var1_index,
                                   var2_index - var1_index - 1);
            new_variable(commands[i].store, VARIABLE_STRING, assert_lhs, -1,
                         -1);
            sdsfree(assert_lhs);
        }
        else if (!strcmp(commands[i].cmd, "get_line_count"))
        {
            k = 0;
            for (size_t j = 0; j < strlen(source_string); j++)
            {
                if (source_string[j] == '\n')
                    k++;
            }
            new_variable(commands[i].store, VARIABLE_INT, NULL, k, -1);
        }
        else if (!strcmp(commands[i].cmd, "length"))
        {
            if (define_variable_type(commands[i].source) != VARIABLE_STRING)
                continue;

            k = strlen(source_string);
            new_variable(commands[i].store, VARIABLE_INT, NULL, k, -1);
        }
        else if (!strcmp(commands[i].cmd, "count_substring"))
        {
            if (define_variable_type(commands[i].source) != VARIABLE_STRING ||
                commands[i].lhs_type != VARIABLE_STRING)
                continue;

            k = 0;
            endptr = source_string;
            while ((endptr = strstr(endptr, commands[i].lhs)) != NULL)
            {
                k++;
                endptr++;
            }
            new_variable(commands[i].store, VARIABLE_INT, NULL, k, -1);
        }
        else if (!strcmp(commands[i].cmd, "split"))
        {
            if (define_variable_type(commands[i].source) != VARIABLE_STRING ||
                commands[i].lhs_type != VARIABLE_STRING ||
                commands[i].rhs_type != VARIABLE_INT ||
                !strcmp(commands[i].store, ""))
                continue;

            k = 0;
            endptr = strtok(source_string, commands[i].lhs);
            while (endptr != NULL)
            {
                if (k++ == commands[i].rhs_int)
                {
                    new_variable(commands[i].store, VARIABLE_STRING, endptr, -1,
                                 -1);
                    break;
                }
                endptr = strtok(NULL, commands[i].lhs);
            }
        }
        else if (!strcmp(commands[i].cmd, "count_matches"))
        {
            if (define_variable_type(commands[i].source) != VARIABLE_STRING ||
                commands[i].lhs_type != VARIABLE_STRING ||
                !strcmp(commands[i].store, ""))
                continue;

            new_variable(commands[i].store, VARIABLE_INT, NULL,
                         regex_count(commands[i].lhs, source_string,
                                     sdslen(source_string)),
                         -1);
        }
    }
    if (fault)
    {
        reason = sdscpylen(reason, "Segmentation fault detected.", 28);
        result = 0;
    }
    else
    {
        reason = sdscatprintf(reason, "Assertion %d failed.", assert_count);
    }
    if (!ran_background)
        print_output(testcase_obj, result, &reason, &output);
    else
        print_output(testcase_obj, result, &reason, &emptyvar);
    sdsfree(testcase_obj.name);
    sdsfree(testcase_obj.description);
    sdsfree(testcase_obj.input);
    sdsfree(testcase_obj.validationtype);
    sdsfree(cmd);
    sdsfree(source_string);
    sdsfree(emptyvar);
    for (int j = 0; j < program_args_length; j++)
    {
        free(program_args[j]);
    }
    free(program_args);
    if (!ran_background)
        sdsfree(output);
    for (int j = 0; j < env_count; j++)
    {
        sdsfree(testcase_obj.environmental_values[j]);
    }
    free(testcase_obj.environmental_values);
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
        for (k = 0; k < commands[j].value_count; k++)
        {
            sdsfree(commands[j].values[k]);
        }
        if (commands[j].value_count)
            free(commands[j].values);
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

double get_source_non_char(char *source)
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
    if (index == -1 || variables[index].type == VARIABLE_STRING)
        return strtod(source, &endptr);

    if (variables[index].type == VARIABLE_INT)
        return (double)variables[index].valueint;
    else
        return variables[index].valuedouble;
}

int define_variable_type(char *varname)
{
    sds variablename;
    int i;
    int type = -1;
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
        return VARIABLE_STRING;
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
        if (fabs(var.valuedouble - floor(var.valuedouble)) < EPSILON)
            stringified_str =
                sdscatprintf(stringified_str, "%d", (int)var.valuedouble);
        else
            stringified_str =
                sdscatprintf(stringified_str, "%f", var.valuedouble);
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
