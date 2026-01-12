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

int complex_test(cJSON *testcase_json)
{
    struct timespec ts;
    struct timespec ts2;
    testcase testcase_obj;
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
    sds cmd = sdsempty();
    int result;
    int i;
    int env_count;
    int64_t duration = 0;

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

    cJSON_ArrayForEach(complex_command, cJSON_GetObjectItemCaseSensitive(
                                            testcase_json, "pipeline"))
    {
        cmd_json = cJSON_GetObjectItemCaseSensitive(complex_command, "cmd");
        if (cJSON_IsString(cmd_json) && (cmd_json->valuestring != NULL))
        {
            cmd = sdscpylen(cmd, cmd_json->valuestring,
                            strlen(cmd_json->valuestring));
        }
        if (!strcmp(cmd, "run"))
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
    }

    testcase_obj.exitcode = exitcode;
printoutput:
    result = 1;
    testcase_obj.duration = duration;
    print_output(testcase_obj, result, &reason, &output);
exit:
    sdsfree(testcase_obj.name);
    sdsfree(testcase_obj.description);
    sdsfree(testcase_obj.input);
    sdsfree(testcase_obj.validationtype);
    sdsfree(cmd);
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

    return result;
}
