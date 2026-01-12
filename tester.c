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

int test(cJSON *testcase_json)
{
    struct timespec ts;
    struct timespec ts2;
    testcase testcase_obj;
    int fault;
    int exitcode;
    int exitcode_shouldbe;
    int64_t start_date;
    int64_t end_date;
    int program_args_length;
    sds output;
    sds reason = sdsempty();
    char **program_args;
    cJSON *testcase_input_str;
    cJSON *env_var;
    int result;
    int i;
    int env_count;
    int64_t duration;

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
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        start_date = ((int64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    }
    else
    {
        perror("clock_gettime");
        start_date = 0;
    }
    if (env_count == 0)
        output = execute(program_args, testcase_obj.input, &fault, &exitcode,
                         args.enviromental_values, args.env_count);
    else
        output = execute(program_args, testcase_obj.input, &fault, &exitcode,
                         testcase_obj.enviromental_values, env_count);
    if (clock_gettime(CLOCK_REALTIME, &ts2) == 0)
    {
        end_date = ((int64_t)ts2.tv_sec * 1000) + (ts2.tv_nsec / 1000000);
    }
    else
    {
        perror("clock_gettime");
        end_date = 0;
    }

    duration = end_date - start_date;
    testcase_obj.exitcode = exitcode;
    if (testcase_obj.exitcodesmallergiven)
        exitcode_shouldbe = testcase_obj.exitcodesmaller;
    else if (testcase_obj.exitcodeequalsgiven)
        exitcode_shouldbe = testcase_obj.exitcodeequals;
    else if (testcase_obj.exitcodegreatergiven)
        exitcode_shouldbe = testcase_obj.exitcodegreater;
    else
        exitcode_shouldbe = 1000;

    result = passed_or_not(output, testcase_obj, fault, &reason, duration,
                           exitcode_shouldbe);
    testcase_obj.duration = duration;
    print_output(testcase_obj, result, &reason, &output);

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
    if (testcase_obj.expectedoutputgiven)
    {
        for (i = 0; i < testcase_obj.expectedoutput->count; i++)
        {
            sdsfree(testcase_obj.expectedoutput->outputs[i]);
        }
        free(testcase_obj.expectedoutput->outputs);
        free(testcase_obj.expectedoutput);
    }
    if (testcase_obj.notexpectedoutputgiven)
    {
        for (i = 0; i < testcase_obj.notexpectedoutput->count; i++)
        {
            sdsfree(testcase_obj.notexpectedoutput->outputs[i]);
        }
        free(testcase_obj.notexpectedoutput->outputs);
        free(testcase_obj.notexpectedoutput);
    }
    if (testcase_obj.containingoutputgiven)
    {
        for (i = 0; i < testcase_obj.containingoutput->count; i++)
        {
            sdsfree(testcase_obj.containingoutput->outputs[i]);
        }
        free(testcase_obj.containingoutput->outputs);
        free(testcase_obj.containingoutput);
    }
    if (testcase_obj.notcontainingoutputgiven)
    {
        for (i = 0; i < testcase_obj.notcontainingoutput->count; i++)
        {
            sdsfree(testcase_obj.notcontainingoutput->outputs[i]);
        }
        free(testcase_obj.notcontainingoutput->outputs);
        free(testcase_obj.notcontainingoutput);
    }
    if (testcase_obj.matchregexgiven)
    {
        for (i = 0; i < testcase_obj.matchregex->count; i++)
        {
            sdsfree(testcase_obj.matchregex->outputs[i]);
        }
        free(testcase_obj.matchregex->outputs);
        free(testcase_obj.matchregex);
    }
    if (testcase_obj.notmatchregexgiven)
    {
        for (i = 0; i < testcase_obj.notmatchregex->count; i++)
        {
            sdsfree(testcase_obj.notmatchregex->outputs[i]);
        }
        free(testcase_obj.notmatchregex->outputs);
        free(testcase_obj.notmatchregex);
    }
    sdsfree(testcase_obj.name);
    sdsfree(testcase_obj.description);
    sdsfree(testcase_obj.input);
    sdsfree(testcase_obj.validationtype);
    return result;
}

void print_output(testcase testcase_obj, int result, sds *reason, sds *output)
{
    int most_similar;
    int i;
    sds duration = sdsempty();
    if (testcase_obj.duration <= 100)
        duration = sdscatprintf(duration, "\033[92m(%ld ms)\033[0m",
                                testcase_obj.duration);
    else if (testcase_obj.duration > 100 && testcase_obj.duration <= 500)
        duration = sdscatprintf(duration, "\033[93m(%ld ms)\033[0m",
                                testcase_obj.duration);
    else if (testcase_obj.duration > 500 && testcase_obj.duration < 1000)
        duration = sdscatprintf(duration, "\033[91m(%ld ms)\033[0m",
                                testcase_obj.duration);
    else
        duration = sdscatprintf(duration, "\033[91m(%ld s)\033[0m",
                                (testcase_obj.duration) / 1000);
    if (result)
    {
        printf("\033[1m\033[92m[PASSED]\033[0m Test (%d/%d): "
               "\033[1m%s\033[0m %s\n",
               args.testcase_counter, args.testcase_count, testcase_obj.name,
               duration);
        printf("      \033[96mDescription: %s\033[0m\n",
               testcase_obj.description);
    }
    else
    {
        printf("\033[1m\033[91m[FAILED]\033[0m Test (%d/%d): "
               "\033[1m%s\033[0m %s\n",
               args.testcase_counter, args.testcase_count, testcase_obj.name,
               duration);
        printf("      \033[96mDescription: %s\033[0m\n",
               testcase_obj.description);
        if (args.reason)
        {
            printf("      \033[91m✘ Reason: %s\033[0m\n", *reason);
        }
        if (testcase_obj.expectedoutputgiven)
        {
            printf("      \033[93mExpected:\033[0m ");
            for (i = 0; i < testcase_obj.expectedoutput->count; i++)
            {
                replaced_print(testcase_obj.expectedoutput->outputs[i], NULL);
                if (i != testcase_obj.expectedoutput->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.notexpectedoutputgiven)
        {
            printf("      \033[93mNot expected:\033[0m ");
            for (i = 0; i < testcase_obj.notexpectedoutput->count; i++)
            {
                replaced_print(testcase_obj.notexpectedoutput->outputs[i],
                               NULL);
                if (i != testcase_obj.notexpectedoutput->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.containingoutputgiven)
        {
            printf("      \033[93mExpected containing:\033[0m ");
            for (i = 0; i < testcase_obj.containingoutput->count; i++)
            {
                replaced_print(testcase_obj.containingoutput->outputs[i], NULL);
                if (i != testcase_obj.containingoutput->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.notcontainingoutputgiven)
        {
            printf("      \033[93mNot expected containing:\033[0m ");
            for (i = 0; i < testcase_obj.notcontainingoutput->count; i++)
            {
                replaced_print(testcase_obj.notcontainingoutput->outputs[i],
                               NULL);
                if (i != testcase_obj.notcontainingoutput->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.matchregexgiven)
        {
            printf("      \033[93mNot matched regexes:\033[0m ");
            for (i = 0; i < testcase_obj.matchregex->count; i++)
            {
                replaced_print(testcase_obj.matchregex->outputs[i], NULL);
                if (i != testcase_obj.matchregex->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.notmatchregexgiven)
        {
            printf("      \033[93mMatched regexes:\033[0m ");
            for (i = 0; i < testcase_obj.notmatchregex->count; i++)
            {
                replaced_print(testcase_obj.notmatchregex->outputs[i], NULL);
                if (i != testcase_obj.notmatchregex->count - 1)
                {
                    printf(", ");
                }
            }
        }
        else if (testcase_obj.exitcodesmallergiven)
        {
            printf("      \033[93mExit code should be smaller than:\033[0m %d",
                   testcase_obj.exitcodesmaller);
        }
        else if (testcase_obj.exitcodeequalsgiven)
        {
            printf("      \033[93mExit code should be equal to:\033[0m %d",
                   testcase_obj.exitcodeequals);
        }
        else if (testcase_obj.exitcodegreatergiven)
        {
            printf("      \033[93mExit code should be greater than:\033[0m %d",
                   testcase_obj.exitcodegreater);
        }
        printf("\n      \033[93mActual:  \033[0m ");
        if (testcase_obj.expectedoutputgiven)
        {
            most_similar = 0;
            for (i = 0; i < testcase_obj.expectedoutput->count; i++)
            {
                if (strcmp(*output, testcase_obj.expectedoutput->outputs[i]) <
                    most_similar)
                    most_similar = i;
            }
            replaced_print(
                *output,
                diff(*output,
                     testcase_obj.expectedoutput->outputs[most_similar]));
        }
        else if (testcase_obj.exitcodesmallergiven ||
                 testcase_obj.exitcodeequalsgiven ||
                 testcase_obj.exitcodegreatergiven)
            printf("%d", testcase_obj.exitcode);
        else
            replaced_print(*output, NULL);
        printf("\n");
    }
    printf("\033["
           "90m────────────────────────────────────────────────────────────"
           "\033[0m\n");
    sdsfree(duration);
    sdsfree(*reason);
}

int runtests(char *json)
{
    cJSON *testcases;
    cJSON *env_var;
    cJSON *testcase_objjson;
    cJSON *type;
    int passed = 0;
    int failed = 0;
    int result;
    int env_count = 0;
    int type_val = 0;
    int i;
    args.binary_file = sdsempty();
    get_binary_json(&args.binary_file, json);
    get_timeout_json(json);
    args.testcase_count = get_testcase_count(json);
    testcases = cJSON_Parse(json);
    if (!testcases)
    {
        const char *error = get_error();
        if (error != NULL)
            fprintf(stderr, "Error before: %s\n", error);
        return 1;
    }

    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcases, "enviromentalVariables"))
    {
        env_count++;
    }

    args.enviromental_values = malloc(sizeof(sds *) * env_count);
    i = 0;
    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcases, "enviromentalVariables"))
    {
        if (cJSON_IsString(env_var) && (env_var->valuestring != NULL))
        {
            args.enviromental_values[i++] = sdsnew(env_var->valuestring);
        }
    }

    args.env_count = env_count;

    args.testcase_counter = 1;
    cJSON_ArrayForEach(testcase_objjson,
                       cJSON_GetObjectItemCaseSensitive(testcases, "testcases"))
    {
        type = cJSON_GetObjectItemCaseSensitive(testcase_objjson, "type");
        if (cJSON_IsNumber(type))
        {
            type_val = type->valuedouble;
        }
        if (type_val == 2)
            result = complex_test(testcase_objjson);
        else if (type_val == 1)
            result = test(testcase_objjson);
        else
            result = 0;
        if (result)
        {
            passed++;
        }
        else
        {
            failed++;
        }
        args.testcase_counter++;
    }
    execution_summary(passed, failed);
    for (int j = 0; j < env_count; j++)
    {
        sdsfree(args.enviromental_values[j]);
    }
    free(args.enviromental_values);
    cJSON_Delete(testcases);
    return 0;
}

int passed_or_not(char *output, testcase testcase_obj, int fault, sds *reason,
                  int64_t duration, int exitcode_shouldbe)
{
    int i = 0;
    int found = 0;
    int validation_type_or = 0;
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

    if (compare(testcase_obj.validationtype, "OR") == 0)
        validation_type_or = 1;

    if (testcase_obj.expectedoutputgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.expectedoutput->count; i++)
        {
            if (compare(output, testcase_obj.expectedoutput->outputs[i]) == 0)
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
        }
        if (!found)
        {
            *reason = sdscpylen(*reason,
                                "Program output was not exactly same with any "
                                "of the provided strings.",
                                69);
        }
        return found;
    }
    else if (testcase_obj.containingoutputgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.containingoutput->count; i++)
        {
            if (strstr(output, testcase_obj.containingoutput->outputs[i]) !=
                NULL)
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
            if (!found)
            {
                if (validation_type_or)
                {
                    *reason = sdscpylen(*reason,
                                        "Program output did not contain any of "
                                        "the provided strings.",
                                        59);
                }
                else
                {
                    *reason = sdscpylen(
                        *reason,
                        "Program output did not contain all required strings.",
                        52);
                }
            }
        }
        return found;
    }
    else if (testcase_obj.notexpectedoutputgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.notexpectedoutput->count; i++)
        {
            if (compare(output, testcase_obj.notexpectedoutput->outputs[i]) !=
                0)
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
        }
        if (!found)
            *reason = sdscpylen(*reason,
                                "Program output was exactly the same with the "
                                "output written in JSON file.",
                                73);
        return found;
    }
    else if (testcase_obj.notcontainingoutputgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.notcontainingoutput->count; i++)
        {
            if (strstr(output, testcase_obj.notcontainingoutput->outputs[i]) ==
                NULL)
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
        }
        if (!found)
        {
            if (validation_type_or)
            {
                *reason = sdscpylen(
                    *reason,
                    "Program output contained any of the provided strings.",
                    53);
            }
            else
            {
                *reason = sdscpylen(
                    *reason,
                    "Program output contained all of the provided strings.",
                    53);
            }
        }
        return found;
    }
    else if (testcase_obj.matchregexgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.matchregex->count; i++)
        {
            if (regex_pass(testcase_obj.matchregex->outputs[i], output,
                           strlen(output)))
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
        }
        if (!found)
        {
            if (validation_type_or)
            {
                *reason = sdscpylen(
                    *reason,
                    "Program output didn't match of the provided patterns.",
                    53);
            }
            else
            {
                *reason = sdscpylen(*reason,
                                    "Program output didn't matched one of the "
                                    "provided patterns.",
                                    59);
            }
        }
        return found;
    }
    else if (testcase_obj.notmatchregexgiven)
    {
        found = 0;
        for (i = 0; i < testcase_obj.notmatchregex->count; i++)
        {
            if (!regex_pass(testcase_obj.notmatchregex->outputs[i], output,
                            strlen(output)))
            {
                found = 1;
                if (compare(testcase_obj.validationtype, "OR") == 0)
                    break;
            }
            else if (compare(testcase_obj.validationtype, "AND") == 0)
            {
                found = 0;
            }
        }
        if (!found)
        {
            if (validation_type_or)
            {
                *reason = sdscpylen(
                    *reason,
                    "Program output matched one of the provided patterns.", 52);
            }
            else
            {
                *reason = sdscpylen(
                    *reason,
                    "Program output matched all of the provided patterns.", 52);
            }
        }
        return found;
    }
    else if (testcase_obj.exitcodesmallergiven)
    {
        if (!(testcase_obj.exitcode < exitcode_shouldbe))
        {
            *reason = sdscpylen(
                *reason, "Exit code was not smaller than it should be.", 44);
            return 0;
        }
    }
    else if (testcase_obj.exitcodeequalsgiven)
    {
        if (!(testcase_obj.exitcode == exitcode_shouldbe))
        {
            *reason = sdscpylen(
                *reason,
                "Exit code was not exactly same with the value it should be.",
                59);
            return 0;
        }
    }
    else if (testcase_obj.exitcodegreatergiven)
    {
        if (!(testcase_obj.exitcode > exitcode_shouldbe))
        {
            *reason = sdscpylen(
                *reason, "Exit code was not greater than it should be.", 44);
            return 0;
        }
    }

    return 1;
}
