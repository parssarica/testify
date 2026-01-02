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
    test_case.name = sdsempty();
    test_case.description = sdsempty();
    test_case.input = sdsempty();
    test_case.expectedoutput = sdsempty();
    test_case.notexpectedoutput = sdsempty();
    test_case.containingoutput = sdsempty();
    test_case.notcontainingoutput = sdsempty();
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
    }

    notexpectedoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notExpectedOutput");
    if (cJSON_IsString(notexpectedoutput) &&
        (notexpectedoutput->valuestring != NULL))
    {
        test_case.notexpectedoutput = sdscpylen(
            test_case.notexpectedoutput, notexpectedoutput->valuestring,
            strlen(notexpectedoutput->valuestring));
    }

    containingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "containingOutput");
    if (cJSON_IsString(containingoutput) &&
        (containingoutput->valuestring != NULL))
    {
        test_case.containingoutput =
            sdscpylen(test_case.containingoutput, containingoutput->valuestring,
                      strlen(containingoutput->valuestring));
    }

    notcontainingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notContainingOutput");
    if (cJSON_IsString(notcontainingoutput) &&
        (notcontainingoutput->valuestring != NULL))
    {
        test_case.notcontainingoutput = sdscpylen(
            test_case.notcontainingoutput, notcontainingoutput->valuestring,
            strlen(notcontainingoutput->valuestring));
    }

    return test_case;
}
