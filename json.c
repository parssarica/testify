/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>

static const char *get_error()
{
    const char *error = cJSON_GetErrorPtr();
    if (error != NULL)
        return error;
    else
        return NULL;
}

char *get_binary_json(char *json_string)
{
    cJSON *json = cJSON_Parse(json_string);
    cJSON *binary;
    if (!json)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        goto end;
    }
    binary = cJSON_GetObjectItemCaseSensitive(json, "binary");
    if (cJSON_IsString(binary) && (binary->valuestring != NULL))
    {
        return binary->valuestring;
    }
    cJSON_Delete(json);
end:
    return NULL;
}

testcase parse_testcase(cJSON *testcase_obj)
{
    testcase test_case;
    const cJSON *name;
    const cJSON *description;
    const cJSON *input;
    const cJSON *type;
    const cJSON *expectedoutput;
    name = cJSON_GetObjectItemCaseSensitive(testcase_obj, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        test_case.name = sdsnew(name->valuestring);
    }

    description = cJSON_GetObjectItemCaseSensitive(testcase_obj, "description");
    if (cJSON_IsString(description) && (description->valuestring != NULL))
    {
        test_case.description = sdsnew(description->valuestring);
    }

    input = cJSON_GetObjectItemCaseSensitive(testcase_obj, "input");
    if (cJSON_IsString(input) && (input->valuestring != NULL))
    {
        test_case.input = sdsnew(input->valuestring);
    }

    type = cJSON_GetObjectItemCaseSensitive(testcase_obj, "type");
    if (cJSON_IsString(type) && (type->valuestring != NULL))
    {
        test_case.type = sdsnew(type->valuestring);
    }

    expectedoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "expectedOutput");
    if (cJSON_IsString(expectedoutput) && (expectedoutput->valuestring != NULL))
    {
        test_case.expectedoutput = sdsnew(expectedoutput->valuestring);
    }

    return test_case;
}
