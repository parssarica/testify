/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
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
    const cJSON *validationtype;
    const cJSON *input;
    const cJSON *type;
    const cJSON *expectedoutput;
    const cJSON *notexpectedoutput;
    const cJSON *containingoutput;
    const cJSON *notcontainingoutput;
    const cJSON *exitcodesmaller;
    const cJSON *exitcodeequals;
    const cJSON *exitcodegreater;
    const cJSON *matchregex;
    const cJSON *notmatchregex;
    const cJSON *timeout;
    const cJSON *i;
    int count = 0;
    int j;
    test_case.name = sdsempty();
    test_case.description = sdsempty();
    test_case.input = sdsempty();
    test_case.validationtype = sdsempty();
    test_case.expectedoutputgiven = 0;
    test_case.notexpectedoutputgiven = 0;
    test_case.containingoutputgiven = 0;
    test_case.notcontainingoutputgiven = 0;
    test_case.exitcodesmallergiven = 0;
    test_case.exitcodeequalsgiven = 0;
    test_case.exitcodegreatergiven = 0;
    test_case.matchregexgiven = 0;
    test_case.notmatchregexgiven = 0;
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

    validationtype =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "validationType");
    if (cJSON_IsString(validationtype) && (validationtype->valuestring != NULL))
    {
        test_case.validationtype =
            sdscpylen(test_case.validationtype, validationtype->valuestring,
                      strlen(validationtype->valuestring));
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
        test_case.expectedoutput = malloc(sizeof(output));
        test_case.expectedoutput->count = 1;
        test_case.expectedoutput->outputs = malloc(sizeof(sds));
        test_case.expectedoutput->outputs[0] =
            sdsnew(expectedoutput->valuestring);
        test_case.expectedoutputgiven = 1;
    }
    else if (cJSON_IsArray(expectedoutput))
    {
        count = 0;
        cJSON_ArrayForEach(i, expectedoutput) { count++; }
        test_case.expectedoutput = malloc(sizeof(output));
        test_case.expectedoutput->count = count;
        test_case.expectedoutput->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.expectedoutputgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, expectedoutput)
        {
            test_case.expectedoutput->outputs[j++] = sdsnew(i->valuestring);
        }
    }

    notexpectedoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notExpectedOutput");
    if (cJSON_IsString(notexpectedoutput) &&
        (notexpectedoutput->valuestring != NULL))
    {
        test_case.notexpectedoutput = malloc(sizeof(testcase));
        test_case.notexpectedoutput->count = 1;
        test_case.notexpectedoutput->outputs = malloc(sizeof(sds));
        test_case.notexpectedoutput->outputs[0] =
            sdsnew(notexpectedoutput->valuestring);
        test_case.notexpectedoutputgiven = 1;
    }
    else if (cJSON_IsArray(notexpectedoutput))
    {
        count = 0;
        cJSON_ArrayForEach(i, notexpectedoutput) { count++; }
        test_case.notexpectedoutput = malloc(sizeof(output));
        test_case.notexpectedoutput->count = count;
        test_case.notexpectedoutput->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.notexpectedoutputgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, notexpectedoutput)
        {
            test_case.notexpectedoutput->outputs[j++] = sdsnew(i->valuestring);
        }
    }

    containingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "containingOutput");
    if (cJSON_IsString(containingoutput) &&
        (containingoutput->valuestring != NULL))
    {
        test_case.containingoutput = malloc(sizeof(testcase));
        test_case.containingoutput->count = 1;
        test_case.containingoutput->outputs = malloc(sizeof(sds));
        test_case.containingoutput->outputs[0] =
            sdsnew(containingoutput->valuestring);
        test_case.containingoutputgiven = 1;
    }
    else if (cJSON_IsArray(containingoutput))
    {
        count = 0;
        cJSON_ArrayForEach(i, containingoutput) { count++; }
        test_case.containingoutput = malloc(sizeof(output));
        test_case.containingoutput->count = count;
        test_case.containingoutput->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.containingoutputgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, containingoutput)
        {
            test_case.containingoutput->outputs[j++] = sdsnew(i->valuestring);
        }
    }

    notcontainingoutput =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notContainingOutput");
    if (cJSON_IsString(notcontainingoutput) &&
        (notcontainingoutput->valuestring != NULL))
    {
        test_case.notcontainingoutput = malloc(sizeof(testcase));
        test_case.notcontainingoutput->count = 1;
        test_case.notcontainingoutput->outputs = malloc(sizeof(sds));
        test_case.notcontainingoutput->outputs[0] =
            sdsnew(notcontainingoutput->valuestring);
        test_case.notcontainingoutputgiven = 1;
    }
    else if (cJSON_IsArray(notcontainingoutput))
    {
        count = 0;
        cJSON_ArrayForEach(i, notcontainingoutput) { count++; }
        test_case.notcontainingoutput = malloc(sizeof(output));
        test_case.notcontainingoutput->count = count;
        test_case.notcontainingoutput->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.notcontainingoutputgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, notcontainingoutput)
        {
            test_case.notcontainingoutput->outputs[j++] =
                sdsnew(i->valuestring);
        }
    }

    matchregex = cJSON_GetObjectItemCaseSensitive(testcase_obj, "matchRegex");
    if (cJSON_IsString(matchregex) && (matchregex->valuestring != NULL))
    {
        test_case.matchregex = malloc(sizeof(testcase));
        test_case.matchregex->count = 1;
        test_case.matchregex->outputs = malloc(sizeof(sds));
        test_case.matchregex->outputs[0] = sdsnew(matchregex->valuestring);
        test_case.matchregexgiven = 1;
    }
    else if (cJSON_IsArray(matchregex))
    {
        count = 0;
        cJSON_ArrayForEach(i, matchregex) { count++; }
        test_case.matchregex = malloc(sizeof(output));
        test_case.matchregex->count = count;
        test_case.matchregex->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.matchregexgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, matchregex)
        {
            test_case.matchregex->outputs[j++] = sdsnew(i->valuestring);
        }
    }

    notmatchregex =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "notMatchRegex");
    if (cJSON_IsString(notmatchregex) && (notmatchregex->valuestring != NULL))
    {
        test_case.notmatchregex = malloc(sizeof(testcase));
        test_case.notmatchregex->count = 1;
        test_case.notmatchregex->outputs = malloc(sizeof(sds));
        test_case.notmatchregex->outputs[0] =
            sdsnew(notmatchregex->valuestring);
        test_case.notmatchregexgiven = 1;
    }
    else if (cJSON_IsArray(notmatchregex))
    {
        count = 0;
        cJSON_ArrayForEach(i, notmatchregex) { count++; }
        test_case.notmatchregex = malloc(sizeof(output));
        test_case.notmatchregex->count = count;
        test_case.notmatchregex->outputs = malloc(sizeof(sds) * count);
        if (count > 0)
            test_case.notmatchregexgiven = 1;
        j = 0;
        cJSON_ArrayForEach(i, notmatchregex)
        {
            test_case.notmatchregex->outputs[j++] = sdsnew(i->valuestring);
        }
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

    exitcodesmaller =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "exitCodeSmaller");
    if (cJSON_IsNumber(exitcodesmaller))
    {
        test_case.exitcodesmaller = exitcodesmaller->valuedouble;
        test_case.exitcodesmallergiven = 1;
    }

    exitcodeequals =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "exitCodeEquals");
    if (cJSON_IsNumber(exitcodeequals))
    {
        test_case.exitcodeequals = exitcodeequals->valuedouble;
        test_case.exitcodeequalsgiven = 1;
    }

    exitcodegreater =
        cJSON_GetObjectItemCaseSensitive(testcase_obj, "exitCodeGreater");
    if (cJSON_IsNumber(exitcodegreater))
    {
        test_case.exitcodegreater = exitcodegreater->valuedouble;
        test_case.exitcodegreatergiven = 1;
    }

    return test_case;
}
