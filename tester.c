/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int runtests(char *json)
{
    testcase testcase_obj;
    cJSON *testcases;
    cJSON *testcase_objjson;
    cJSON *testcase_input_str;
    sds binary_file = sdsempty();
    sds output;
    char **args;
    int i;
    int passed = 0;
    int failed = 0;
    int testcase_count;
    int testcase_counter;
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
        args = malloc(sizeof(char **));
        args[0] = strdup(binary_file);
        i = 1;
        cJSON_ArrayForEach(
            testcase_input_str,
            cJSON_GetObjectItemCaseSensitive(testcase_objjson, "commandArgs"))
        {
            args = realloc(args, sizeof(char **) * ++i);
            if (cJSON_IsString(testcase_input_str) &&
                (testcase_input_str->valuestring != NULL))
            {
                args[i - 1] = strdup(testcase_input_str->valuestring);
            }
        }
        args = realloc(args, sizeof(char **) * ++i + sizeof(NULL));
        args[i] = NULL;
        output = execute(args, testcase_obj.input);
        if (passed_or_not(output, testcase_obj))
        {
            printf("Test (%d/%d) %s - \033[1m\033[92mPASSED\033[0m\n",
                   testcase_counter, testcase_count, testcase_obj.name);
            passed++;
        }
        else
        {
            printf("Test (%d/%d) %s - \033[1m\033[91mFAILED\033[0m\n",
                   testcase_counter, testcase_count, testcase_obj.name);
            failed++;
        }
        for (int j = 0; j < i + 1; j++)
        {
            free(args[j]);
        }
        free(args);
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
    sdsfree(binary_file);
    cJSON_Delete(testcases);
    return 0;
}

int passed_or_not(char *output, testcase testcase_obj)
{
    if (testcase_obj.expectedoutputgiven)
    {
        if (strcmp(output, testcase_obj.expectedoutput) == 0)
        {
            return 1;
        }
        else
        {
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
            return 0;
        }
    }

    return 1;
}
