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
    int source_int = 0;
    double source_double = 0;
    int commandcount = 0;
    int result;
    int i;
    int env_count;
    int k;
    int64_t duration = 0;

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
        }
        else
        {
            commands[i].lhs = sdsempty();
        }

        rhs_json = cJSON_GetObjectItemCaseSensitive(complex_command, "rhs");
        if (cJSON_IsString(rhs_json) && (rhs_json->valuestring != NULL))
        {
            commands[i].rhs = sdsnew(rhs_json->valuestring);
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
        }
        else if (!strcmp(commands[i].cmd, "extract_char"))
        {
            if (define_variable_type(commands[i].source) == VARIABLE_STRING)
            {
                if (strlen(source_string) > commands[i].index)
                    extract_char_character[0] = source_string[i];

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
    }
    result = 1;
    testcase_obj.duration = duration;
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

    sds varname = sdsnewlen(source + 2, strlen(source) - 4);
    int i;
    int index = -1;
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
        return NULL;
    return variables[index].valuestring;
}

int get_source_int(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname = sdsnewlen(source + 2, strlen(source) - 4);
    int i;
    int index = -1;
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
        return -1;
    return variables[index].valueint;
}

double get_source_double(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname = sdsnewlen(source + 2, strlen(source) - 4);
    int i;
    int index = -1;
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
        return -1;
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
    return type;
}
