/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int runtests(char *json)
{
    testcase testcase_obj;
    cJSON *testcases;
    cJSON *testcase_objjson;
    cJSON *testcase_input_str;
    sds binary_file = sdsempty();
    sds output;
    sds reason = sdsempty();
    char **program_args;
    int i;
    int passed = 0;
    int failed = 0;
    int testcase_count;
    int testcase_counter;
    int fault;
    int64_t start_date;
    int64_t end_date;
    struct timespec ts;
    struct timespec ts2;
    get_binary_json(&binary_file, json);
    testcase_count = get_testcase_count(json);
    testcases = cJSON_Parse(json);
    if (!testcases)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return 1;
    }

    testcase_counter = 1;
    cJSON_ArrayForEach(testcase_objjson,
                       cJSON_GetObjectItemCaseSensitive(testcases, "testcases"))
    {
        testcase_obj = parse_testcase(testcase_objjson);
        program_args = malloc(sizeof(char **));
        program_args[0] = strdup(binary_file);
        i = 1;
        cJSON_ArrayForEach(
            testcase_input_str,
            cJSON_GetObjectItemCaseSensitive(testcase_objjson, "commandArgs"))
        {
            program_args = realloc(program_args, sizeof(char **) * ++i);
            if (cJSON_IsString(testcase_input_str) &&
                (testcase_input_str->valuestring != NULL))
            {
                program_args[i - 1] = strdup(testcase_input_str->valuestring);
            }
        }
        program_args =
            realloc(program_args, sizeof(char **) * ++i + sizeof(NULL));
        program_args[i] = NULL;
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
        {
            start_date = ((int64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
        }
        else
        {
            perror("clock_gettime");
            start_date = 0;
        }
        output = execute(program_args, testcase_obj.input, &fault);
        if (clock_gettime(CLOCK_REALTIME, &ts2) == 0)
        {
            end_date = ((int64_t)ts2.tv_sec * 1000) + (ts2.tv_nsec / 1000000);
        }
        else
        {
            perror("clock_gettime");
            end_date = 0;
        }
        if (passed_or_not(output, testcase_obj, fault, &reason,
                          end_date - start_date))
        {
            printf("Test (%d/%d) %s - \033[1m\033[92mPASSED\033[0m\n",
                   testcase_counter, testcase_count, testcase_obj.name);
            passed++;
        }
        else
        {
            if (args.reason)
            {
                printf("Test (%d/%d) %s - \033[1m\033[91mFAILED\033[0m - "
                       "Reason: %s\n",
                       testcase_counter, testcase_count, testcase_obj.name,
                       reason);
            }
            else
            {
                printf("Test (%d/%d) %s - \033[1m\033[91mFAILED\033[0m\n",
                       testcase_counter, testcase_count, testcase_obj.name);
            }
            failed++;
        }
        for (int j = 0; j < i + 1; j++)
        {
            free(program_args[j]);
        }
        free(program_args);
        sdsfree(output);
        sdsfree(testcase_obj.expectedoutput);
        sdsfree(testcase_obj.notexpectedoutput);
        sdsfree(testcase_obj.containingoutput);
        sdsfree(testcase_obj.notcontainingoutput);
        sdsfree(testcase_obj.name);
        sdsfree(testcase_obj.description);
        sdsfree(testcase_obj.input);
        testcase_counter++;
    }
    printf("\n────────────────────────────────────────────\n");
    printf("Pass rate:\n\t%f%% - %d / %d\n",
           (((double)passed / ((double)passed + (double)failed)) * 100), passed,
           passed + failed);
    sdsfree(reason);
    sdsfree(binary_file);
    cJSON_Delete(testcases);
    return 0;
}

int passed_or_not(char *output, testcase testcase_obj, int fault, sds *reason,
                  int64_t duration)
{
    if (fault)
    {
        *reason = sdscpylen(*reason, "Segmentation fault detected.", 28);
        return 0;
    }

    if (duration != 0 && testcase_obj.timeout != -1 &&
        duration > testcase_obj.timeout)
    {
        *reason = sdscpylen(*reason, "Timeout expired.", 16);
        return 0;
    }

    if (testcase_obj.expectedoutputgiven)
    {
        if (strcmp(output, testcase_obj.expectedoutput) == 0)
        {
            return 1;
        }
        else
        {
            *reason = sdscpylen(*reason,
                                "Program output was not exacty same with the "
                                "output written in JSON file.",
                                72);
            return 0;
        }
    }
    else if (testcase_obj.containingoutputgiven)
    {
        if (strstr(output, testcase_obj.containingoutput) != NULL)
        {
            return 1;
        }
        else
        {
            *reason = sdscpylen(*reason,
                                "Program output was not containing the string "
                                "written in JSON file.",
                                66);
            return 0;
        }
    }
    else if (testcase_obj.notexpectedoutputgiven)
    {
        if (strcmp(output, testcase_obj.notexpectedoutput) != 0)
        {
            return 1;
        }
        else
        {
            *reason = sdscpylen(*reason,
                                "Program output was exactly the same with the "
                                "output written in JSON file.",
                                73);
            return 0;
        }
    }
    else if (testcase_obj.notcontainingoutputgiven)
    {
        if (strstr(output, testcase_obj.notcontainingoutput) == NULL)
        {
            return 1;
        }
        else
        {
            *reason = sdscpylen(*reason,
                                "Program output was containing the string "
                                "written in JSON file.",
                                62);
            return 0;
        }
    }

    return 1;
}
