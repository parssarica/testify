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
    get_binary_json(&binary_file, json);
    testcases = cJSON_Parse(json);
    if (!testcases)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return 1;
    }

    cJSON_ArrayForEach(testcase_objjson,
                       cJSON_GetObjectItemCaseSensitive(testcases, "testcases"))
    {
        testcase_obj = parse_testcase(testcase_objjson);
        args = malloc(sizeof(char *));
        args[0] = sdsnew(binary_file);
        i = 0;
        cJSON_ArrayForEach(
            testcase_input_str,
            cJSON_GetObjectItemCaseSensitive(testcase_objjson, "commandArgs"))
        {
            args = realloc(args, sizeof(sds) * ++i);
            if (cJSON_IsString(testcase_input_str) &&
                (testcase_input_str->valuestring != NULL))
            {
                args[i] = malloc(strlen(testcase_input_str->valuestring));
                for (int j = 0;
                     (size_t)j < strlen(testcase_input_str->valuestring); j++)
                {
                    args[i][j] = testcase_input_str->valuestring[j];
                }
            }
        }
        args = realloc(args, sizeof(char *) * ++i + sizeof(NULL));
        args[i] = NULL;
        output = execute(args, testcase_obj.input);
        printf("%s\n", output);
        sdsfree(args[0]);
        for (int j = 1; j < i + 1; j++)
        {
            free(args[j]);
        }
        free(args);
        sdsfree(output);
    }
    sdsfree(binary_file);
    cJSON_Delete(testcases);
    return 0;
}
