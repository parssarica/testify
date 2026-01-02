/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>

int runtests(char *json)
{
    testcase testcase_obj;
    cJSON *testcases;
    cJSON *testcase_objjson;
    sds binary_file = sdsempty();
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
        printf("Name: <%s>\n", testcase_obj.name);
        printf("Description: <%s>\n", testcase_obj.description);
        printf("Input: <%s>\n", testcase_obj.input);
        printf("Type: <%d>\n", testcase_obj.type);
        printf("Expected output: <%s>\n", testcase_obj.expectedoutput);
        printf("Not Expected output: <%s>\n", testcase_obj.notexpectedoutput);
    }
    sdsfree(binary_file);
    cJSON_Delete(testcases);
    return 0;
}
