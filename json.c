/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <string.h>

const char *get_error()
{
    const char *error = cJSON_GetErrorPtr();
    if (error != NULL)
        return error;
    else
        return "";
}

void get_binary_json(sds *binary_file, char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    cJSON *binary;
    if (!json)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return;
    }
    binary = cJSON_GetObjectItemCaseSensitive(json, "binary");
    if (cJSON_IsString(binary) && (binary->valuestring != NULL))
    {
        *binary_file = sdscpylen(*binary_file, binary->valuestring,
                                 strlen(binary->valuestring));
    }
    cJSON_Delete(json);
}

void get_timeout_json(char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    cJSON *timeout;
    if (!json)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return;
    }

    timeout = cJSON_GetObjectItemCaseSensitive(json, "timeout");
    if (cJSON_IsNumber(timeout))
    {
        args.timeout = timeout->valuedouble;
    }
    cJSON_Delete(json);
}

int get_testcase_count(char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    cJSON *testcases;
    cJSON *i;
    int count = 0;
    if (!json)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return -1;
    }
    testcases = cJSON_GetObjectItemCaseSensitive(json, "testcases");
    cJSON_ArrayForEach(i, testcases) { count++; }
    cJSON_Delete(json);
    return count;
}

testcase parse_testcase(cJSON *testcase_obj)
{
    testcase test_case;
    const cJSON *name;
    const cJSON *description;
    const cJSON *input;
    const cJSON *type;
    const cJSON *expectedoutput;
    const cJSON *notexpectedoutput;
    const cJSON *containingoutput;
    const cJSON *notcontainingoutput;
    const cJSON *timeout;
    test_case.name = sdsempty();
    test_case.description = sdsempty();
    test_case.input = sdsempty();
    test_case.expectedoutput = sdsempty();
    test_case.notexpectedoutput = sdsempty();
    test_case.containingoutput = sdsempty();
    test_case.notcontainingoutput = sdsempty();
    test_case.expectedoutputgiven = 0;
    test_case.notexpectedoutputgiven = 0;
    test_case.containingoutputgiven = 0;
    test_case.notcontainingoutputgiven = 0;
    test_case.timeout = -1;
    type = cJSON_GetObjectItemCaseSensitive(testcase_obj, "type");
    if (cJSON_IsNumber(type))
    {
        test_case.type = type->valuedouble;
    }

    name = cJSON_GetObjectItemCaseSensitive(testcase_obj, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        test_case.name = sdscpylen(test_case.name, name->valuestring,
                                   strlen(name->valuestring));
    }

    description = cJSON_GetObjectItemCaseSensitive(testcase_obj, "description");
    if (cJSON_IsString(description) && (description->valuestring != NULL))
    {
        test_case.description =
            sdscpylen(test_case.description, description->valuestring,
                      strlen(description->valuestring));
    }

    input = cJSON_GetObjectItemCaseSensitive(testcase_obj, "input");
    if (cJSON_IsString(input) && (input->valuestring != NULL))
    {
        test_case.input = sdscpylen(test_case.input, input->valuestring,
                                    strlen(input->valuestring));
    }

    expectedoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "expectedOutput");
    if (cJSON_IsString(expectedoutput) && (expectedoutput->valuestring != NULL))
    {
        test_case.expectedoutput =
            sdscpylen(test_case.expectedoutput, expectedoutput->valuestring,
                      strlen(expectedoutput->valuestring));
        test_case.expectedoutputgiven = 1;
    }

    notexpectedoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notExpectedOutput");
    if (cJSON_IsString(notexpectedoutput) &&
        (notexpectedoutput->valuestring != NULL))
    {
        test_case.notexpectedoutput = sdscpylen(
            test_case.notexpectedoutput, notexpectedoutput->valuestring,
            strlen(notexpectedoutput->valuestring));
        test_case.notexpectedoutputgiven = 1;
    }

    containingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "containingOutput");
    if (cJSON_IsString(containingoutput) &&
        (containingoutput->valuestring != NULL))
    {
        test_case.containingoutput =
            sdscpylen(test_case.containingoutput, containingoutput->valuestring,
                      strlen(containingoutput->valuestring));
        test_case.containingoutputgiven = 1;
    }

    notcontainingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notContainingOutput");
    if (cJSON_IsString(notcontainingoutput) &&
        (notcontainingoutput->valuestring != NULL))
    {
        test_case.notcontainingoutput = sdscpylen(
            test_case.notcontainingoutput, notcontainingoutput->valuestring,
            strlen(notcontainingoutput->valuestring));
        test_case.notcontainingoutputgiven = 1;
    }

    timeout = cJSON_GetObjectItemCaseSensitive(testcase_obj, "timeout");
    if (cJSON_IsNumber(timeout))
    {
        test_case.timeout = timeout->valuedouble;
    }

    if (test_case.timeout == -1)
    {
        test_case.timeout = args.timeout;
    }

    return test_case;
}
