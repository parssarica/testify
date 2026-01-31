/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include "testify.h"
#include <cjson/cJSON.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

variable *variables;
int variable_count = 0;
int env_count = 0;
int fault = 0;
int exitcode = -1;
char **program_args;
testcase testcase_obj;
process pr;

int complex_test(cJSON *testcase_json)
{
    command *commands;
    command statement_cmd;
    int program_args_length;
    sds output = sdsempty();
    sds reason = sdsempty();
    cJSON *testcase_input_str;
    cJSON *env_var;
    cJSON *complex_command;
    cJSON *pipeline_json;
    cJSON *command_cmd_json;
    cJSON *statement_command;
    cJSON *loop_variable;
    sds cmd = sdsempty();
    sds source_string = sdsempty();
    sds emptyvar = sdsempty();
    int source_int = 0;
    double source_double = 0;
    int commandcount = 0;
    int result = 1;
    int ran_background = 0;
    int assert_count = 0;
    int i;
    int k;
    int old_result;
    int loop_variable_index = -1;

    variable_count = 0;
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
                                    testcase_json, "environmentalVariables"))
    {
        env_count++;
    }

    testcase_obj.environmental_values = malloc(sizeof(sds *) * env_count);
    i = 0;
    cJSON_ArrayForEach(env_var, cJSON_GetObjectItemCaseSensitive(
                                    testcase_json, "environmentalVariables"))
    {
        if (cJSON_IsString(env_var) && (env_var->valuestring != NULL))
        {
            testcase_obj.environmental_values[i] = sdsnew(env_var->valuestring);
            i++;
        }
    }

    commandcount = 0;
    i = 0;
    pipeline_json = cJSON_GetObjectItemCaseSensitive(testcase_json, "pipeline");
    cJSON_ArrayForEach(complex_command, pipeline_json) { commandcount++; }

    commands = malloc(sizeof(command) * commandcount);
    cJSON_ArrayForEach(complex_command, cJSON_GetObjectItemCaseSensitive(
                                            testcase_json, "pipeline"))
    {
        commands[i] = parse_command(complex_command);
        i++;
    }

    testcase_obj.exitcode = exitcode;

    for (i = 0; i < commandcount; i++)
    {
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_STRING ||
             !strcmp(commands[i].source, "output")))
            source_string = sdscpy(source_string,
                                   get_source_str(commands[i].source, output));
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_INT))
            source_int = get_source_int(commands[i].source);
        if (commands[i].source != NULL &&
            (define_variable_type(commands[i].source) == VARIABLE_DOUBLE))
            source_double = get_source_double(commands[i].source);

        if (!strcmp(commands[i].cmd, "if"))
        {
            command_cmd_json = get_nth_json(pipeline_json, i);
            old_result = result;
            run_command(parse_command(cJSON_GetObjectItemCaseSensitive(
                            command_cmd_json, "condition")),
                        &result, source_string, source_int, source_double,
                        &output, &assert_count);
            assert_count--;
            if (result)
            {
                result = old_result;
                cJSON_ArrayForEach(
                    statement_command,
                    cJSON_GetObjectItemCaseSensitive(command_cmd_json, "then"))
                {
                    statement_cmd = parse_command(statement_command);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                             VARIABLE_STRING ||
                         !strcmp(statement_cmd.source, "output")))
                        source_string = sdscpy(
                            source_string,
                            get_source_str(statement_cmd.source, output));
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_INT))
                        source_int = get_source_int(statement_cmd.source);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_DOUBLE))
                        source_double = get_source_double(statement_cmd.source);

                    run_command(statement_cmd, &result, source_string,
                                source_int, source_double, &output,
                                &assert_count);
                }
            }
            else
            {
                result = old_result;
                cJSON_ArrayForEach(
                    statement_command,
                    cJSON_GetObjectItemCaseSensitive(command_cmd_json, "else"))
                {
                    statement_cmd = parse_command(statement_command);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                             VARIABLE_STRING ||
                         !strcmp(statement_cmd.source, "output")))
                        source_string = sdscpy(
                            source_string,
                            get_source_str(statement_cmd.source, output));
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_INT))
                        source_int = get_source_int(statement_cmd.source);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_DOUBLE))
                        source_double = get_source_double(statement_cmd.source);

                    run_command(statement_cmd, &result, source_string,
                                source_int, source_double, &output,
                                &assert_count);
                }
            }
        }
        else if (!strcmp(commands[i].cmd, "loop"))
        {
            command_cmd_json = get_nth_json(pipeline_json, i);
            k = atoi(source_string);
            loop_variable = cJSON_GetObjectItemCaseSensitive(command_cmd_json,
                                                             "indexVariable");
            if (cJSON_IsString(loop_variable) &&
                (loop_variable->valuestring != NULL))
            {
                loop_variable_index = variable_count;
                new_variable(loop_variable->valuestring, VARIABLE_INT, NULL, 0,
                             -1);
            }

            for (int j = 0; j < k; j++)
            {
                cJSON_ArrayForEach(
                    statement_command,
                    cJSON_GetObjectItemCaseSensitive(command_cmd_json, "steps"))
                {
                    statement_cmd = parse_command(statement_command);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                             VARIABLE_STRING ||
                         !strcmp(statement_cmd.source, "output")))
                        source_string = sdscpy(
                            source_string,
                            get_source_str(statement_cmd.source, output));
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_INT))
                        source_int = get_source_int(statement_cmd.source);
                    if (statement_cmd.source != NULL &&
                        (define_variable_type(statement_cmd.source) ==
                         VARIABLE_DOUBLE))
                        source_double = get_source_double(statement_cmd.source);

                    if (!strcmp(statement_cmd.cmd, "break"))
                    {
                        j = k;
                        free_command(&statement_cmd);
                        break;
                    }
                    else if (!strcmp(statement_cmd.cmd, "continue"))
                    {
                        free_command(&statement_cmd);
                        break;
                    }
                    run_command(statement_cmd, &result, source_string,
                                source_int, source_double, &output,
                                &assert_count);
                }
                if (loop_variable_index != -1)
                    variables[loop_variable_index].valueint++;
            }
            loop_variable_index = -1;
        }
        run_command(commands[i], &result, source_string, source_int,
                    source_double, &output, &assert_count);
    }
    if (fault)
    {
        reason = sdscpylen(reason, "Segmentation fault detected.", 28);
        result = 0;
    }
    else
    {
        reason = sdscatprintf(reason, "Assertion %d failed.", assert_count);
    }
    if (!ran_background)
        print_output(testcase_obj, result, &reason, &output);
    else
        print_output(testcase_obj, result, &reason, &emptyvar);
    sdsfree(testcase_obj.name);
    sdsfree(testcase_obj.description);
    sdsfree(testcase_obj.input);
    sdsfree(testcase_obj.validationtype);
    sdsfree(cmd);
    sdsfree(source_string);
    sdsfree(emptyvar);
    for (int j = 0; j < program_args_length; j++)
    {
        free(program_args[j]);
    }
    free(program_args);
    sdsfree(output);
    for (int j = 0; j < env_count; j++)
    {
        sdsfree(testcase_obj.environmental_values[j]);
    }
    free(testcase_obj.environmental_values);
    for (int j = 0; j < variable_count; j++)
    {
        sdsfree(variables[j].name);
        sdsfree(variables[j].valuestring);
    }
    if (variable_count > 0)
        free(variables);

    free(commands);

    return result;
}

void run_command(command cmd, int *result_val, char *source_str, int source_int,
                 double source_double, sds *output_str, int *assert_count)
{
    ssize_t n;
    size_t tmp;
    int64_t duration = 0;
    FILE *fptr;
    char *endptr;
    int found;
    sds varname;
    sds varname2;
    variable *vars_tmp;
    int var1_index;
    int var2_index;
    double assert_lhs_double = 0;
    double assert_rhs_double = 0;
    sds file_content;
    char extract_char_character[2];
    struct timespec ts;
    struct timespec ts2;
    int64_t start_date;
    int64_t end_date;
    sds source_string = sdsnew(source_str);
    sds assert_lhs;
    sds assert_rhs;
    sds output = sdsnew(*output_str);
    int k;
    char output_buf[4096];
    int result = 1;

    memset(output_buf, 0, 4096);
    extract_char_character[1] = 0;
    if (!strcmp(cmd.cmd, "run"))
    {
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
        {
            start_date = ((int64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
        }
        else
        {
            perror("clock_gettime");
            start_date = 0;
        }
        if (!cmd.background)
        {
            if (env_count == 0)
            {
                sdsfree(output);
                output =
                    execute(program_args, testcase_obj.input, &fault, &exitcode,
                            args.environmental_values, args.env_count);
            }
            else
            {
                sdsfree(output);
                output =
                    execute(program_args, testcase_obj.input, &fault, &exitcode,
                            testcase_obj.environmental_values, env_count);
            }
        }
        else
        {
            if (env_count == 0)
            {
                pr = execute_background(program_args, args.environmental_values,
                                        args.env_count);
            }
            else
            {
                pr = execute_background(
                    program_args, testcase_obj.environmental_values, env_count);
            }
        }
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
        testcase_obj.duration = duration;
        if (!cmd.background)
            new_variable("output", VARIABLE_STRING, output, -1, -1);
    }
    else if (!strcmp(cmd.cmd, "send_input"))
    {
        interact_write(&pr, &source_string);
    }
    else if (!strcmp(cmd.cmd, "send_line"))
    {
        source_string = sdscatlen(source_string, "\n", 1);
        interact_write(&pr, &source_string);
    }
    else if (!strcmp(cmd.cmd, "wait_for_output"))
    {
        if (source_int)
            n = interact_read(&pr, output_buf, 4096, source_int);
        else
            n = interact_read(&pr, output_buf, 4096, 200);

        if (n <= 0)
            close_child(&pr, &fault, &exitcode);

        new_variable(cmd.store, VARIABLE_STRING, output_buf, -1, -1);
    }
    else if (!strcmp(cmd.cmd, "is_alive"))
    {
        new_variable(cmd.store, VARIABLE_INT, NULL,
                     child_alive(&pr, &fault, &exitcode), -1);
    }
    else if (!strcmp(cmd.cmd, "kill"))
    {
        kill_process(&pr, &fault, &exitcode);
    }
    else if (!strcmp(cmd.cmd, "restart"))
    {
        kill_process(&pr, &fault, &exitcode);
        fault = 0;
        exitcode = -1;
        if (env_count == 0)
            pr = execute_background(program_args, args.environmental_values,
                                    args.env_count);
        else
            pr = execute_background(
                program_args, testcase_obj.environmental_values, env_count);
    }
    else if (!strcmp(cmd.cmd, "sleep"))
    {
        if (strstr(source_string, ".") == NULL)
        {
            usleep(atoi(source_string) * 1000);
        }
        else
        {
            usleep(strtod(source_string, &endptr) * 1000);
        }
    }
    else if (!strcmp(cmd.cmd, "extract_char"))
    {
        if (define_variable_type(cmd.source) == VARIABLE_STRING)
        {
            if (strlen(source_string) > cmd.index)
                extract_char_character[0] = source_string[cmd.index];

            if (strcmp(cmd.store, ""))
                new_variable(cmd.store, VARIABLE_STRING, extract_char_character,
                             -1, -1);
        }
    }
    else if (!strcmp(cmd.cmd, "atoi"))
    {
        if (strcmp(cmd.store, ""))
        {
            k = 1;
            if (define_variable_type(cmd.source) == VARIABLE_STRING)
            {
                for (size_t j = 0; j < strlen(source_string); j++)
                {
                    if (!((size_t)(source_string[j]) >= 0x30 &&
                          (size_t)(source_string[j]) <= 0x39))
                    {
                        k = 0;
                        break;
                    }
                }
                if (k)
                {
                    new_variable(cmd.store, VARIABLE_INT, NULL,
                                 atoi(source_string), -1);
                }
            }
            else if (define_variable_type(cmd.source) == VARIABLE_DOUBLE)
            {
                new_variable(cmd.store, VARIABLE_INT, NULL, (int)source_double,
                             -1);
            }
        }
    }
    else if (!strcmp(cmd.cmd, "atof"))
    {
        if (strcmp(cmd.store, ""))
        {
            k = 1;
            if (define_variable_type(cmd.source) == VARIABLE_STRING)
            {
                for (size_t j = 0; j < strlen(source_string); j++)
                {
                    if (!(((size_t)(source_string[j]) >= 0x30 &&
                           (size_t)(source_string[j]) <= 0x39) ||
                          (size_t)(source_string[j] == 0x2e)))
                    {
                        k = 0;
                        break;
                    }
                }
                if (k)
                {
                    new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                                 strtod(source_string, &endptr));
                }
            }
            else if (define_variable_type(cmd.source) == VARIABLE_INT)
            {
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             (double)source_int);
            }
        }
    }
    else if (!strcmp(cmd.cmd, "to_string"))
    {
        if (strcmp(cmd.store, ""))
        {
            assert_lhs = sdsempty();
            if (define_variable_type(cmd.source) == VARIABLE_INT)
            {
                assert_lhs = sdscatprintf(assert_lhs, "%d", source_int);
            }
            else if (define_variable_type(cmd.source) == VARIABLE_DOUBLE)
            {
                assert_lhs = sdscatprintf(assert_lhs, "%f", source_double);
            }
            else
            {
                assert_lhs =
                    sdscpylen(assert_lhs, source_string, strlen(source_string));
            }
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "to_int"))
    {
        if (strcmp(cmd.store, "") &&
            define_variable_type(cmd.source) == VARIABLE_STRING &&
            strlen(source_string) == 1)
        {
            new_variable(cmd.store, VARIABLE_INT, NULL, (int)(source_string[0]),
                         -1);
        }
    }
    else if (!strcmp(cmd.cmd, "trim"))
    {
        if (define_variable_type(cmd.source) == VARIABLE_STRING)
        {
            assert_lhs =
                to_str(get_var_object(cmd.source, -1, -1, VARIABLE_STRING));
            sdstrim(assert_lhs, " \n\t\r");
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "upper"))
    {
        if (define_variable_type(cmd.source) == VARIABLE_STRING)
        {
            assert_lhs =
                to_str(get_var_object(cmd.source, -1, -1, VARIABLE_STRING));
            for (tmp = 0; tmp < sdslen(assert_lhs); tmp++)
            {
                if (assert_lhs[tmp] >= 0x61 && assert_lhs[tmp] <= 0x7a)
                {
                    assert_lhs[tmp] -= 0x20;
                }
            }
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "lower"))
    {
        if (define_variable_type(cmd.source) == VARIABLE_STRING)
        {
            assert_lhs =
                to_str(get_var_object(cmd.source, -1, -1, VARIABLE_STRING));
            for (tmp = 0; tmp < sdslen(assert_lhs); tmp++)
            {
                if (assert_lhs[tmp] >= 0x41 && assert_lhs[tmp] <= 0x5a)
                {
                    assert_lhs[tmp] += 0x20;
                }
            }
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "concatenate"))
    {
        if (strcmp(cmd.store, " "))
        {
            if (cmd.lhs_type == VARIABLE_STRING)
                assert_lhs =
                    to_str(get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
            else if (cmd.lhs_type == VARIABLE_INT)
                assert_lhs =
                    to_str(get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
            else if (cmd.lhs_type == VARIABLE_DOUBLE)
                assert_lhs = to_str(
                    get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));
            else
                assert_lhs = sdsempty();

            if (cmd.rhs_type == VARIABLE_STRING)
                assert_rhs =
                    to_str(get_var_object(cmd.rhs, -1, -1, VARIABLE_STRING));
            else if (cmd.rhs_type == VARIABLE_INT)
                assert_rhs =
                    to_str(get_var_object(NULL, cmd.rhs_int, -1, VARIABLE_INT));
            else if (cmd.rhs_type == VARIABLE_DOUBLE)
                assert_rhs = to_str(
                    get_var_object(NULL, -1, cmd.rhs_double, VARIABLE_DOUBLE));
            else
                assert_rhs = sdsempty();

            assert_lhs = sdscatsds(assert_lhs, assert_rhs);
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
            sdsfree(assert_rhs);
        }
    }
    else if (!strcmp(cmd.cmd, "min"))
    {
        if (cmd.value_count > 0)
        {
            assert_lhs_double = get_source_non_char(cmd.values[0]);
            for (int j = 0; j < cmd.value_count; j++)
            {
                if (assert_lhs_double > get_source_non_char(cmd.values[j]))
                {
                    assert_lhs_double = get_source_non_char(cmd.values[j]);
                }
            }
            new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                         assert_lhs_double);
        }
    }
    else if (!strcmp(cmd.cmd, "max"))
    {
        if (cmd.value_count > 0)
        {
            assert_lhs_double = get_source_non_char(cmd.values[0]);
            for (int j = 0; j < cmd.value_count; j++)
            {
                if (assert_lhs_double < get_source_non_char(cmd.values[j]))
                {
                    assert_lhs_double = get_source_non_char(cmd.values[j]);
                }
            }
            new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                         assert_lhs_double);
        }
    }
    else if (!strcmp(cmd.cmd, "sum"))
    {
        if (cmd.value_count > 0)
        {
            assert_lhs_double = 0;
            for (int j = 0; j < cmd.value_count; j++)
            {
                assert_lhs_double += get_source_non_char(cmd.values[j]);
            }
            new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                         assert_lhs_double);
        }
    }
    else if (!strcmp(cmd.cmd, "increment"))
    {
        if (strcmp(cmd.store, "") &&
            (define_variable_type(cmd.source) == VARIABLE_INT ||
             define_variable_type(cmd.source) == VARIABLE_DOUBLE))
        {
            assert_lhs_double = get_source_non_char(cmd.source);
            assert_lhs_double++;
            if (define_variable_type(cmd.source) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_INT, NULL, assert_lhs_double,
                             -1);
            else
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             assert_lhs_double);
        }
    }
    else if (!strcmp(cmd.cmd, "decrement"))
    {
        if (strcmp(cmd.store, "") &&
            (define_variable_type(cmd.source) == VARIABLE_INT ||
             define_variable_type(cmd.source) == VARIABLE_DOUBLE))
        {
            assert_lhs_double = get_source_non_char(cmd.source);
            assert_lhs_double--;
            if (define_variable_type(cmd.source) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_INT, NULL, assert_lhs_double,
                             -1);
            else
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             assert_lhs_double);
        }
    }
    else if (!strcmp(cmd.cmd, "add"))
    {
        if (strcmp(cmd.store, ""))
        {
            if (define_variable_type(cmd.lhs) == VARIABLE_INT &&
                define_variable_type(cmd.rhs) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_INT, NULL,
                             get_source_int(cmd.lhs) + get_source_int(cmd.rhs),
                             -1);
            if (define_variable_type(cmd.lhs) == VARIABLE_DOUBLE &&
                define_variable_type(cmd.rhs) == VARIABLE_DOUBLE)
                new_variable(cmd.store, VARIABLE_INT, NULL, -1,
                             get_source_double(cmd.lhs) +
                                 get_source_double(cmd.rhs));
        }
    }
    else if (!strcmp(cmd.cmd, "subtract"))
    {
        if (strcmp(cmd.store, ""))
        {
            if (define_variable_type(cmd.lhs) == VARIABLE_INT &&
                define_variable_type(cmd.rhs) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_INT, NULL,
                             get_source_int(cmd.lhs) - get_source_int(cmd.rhs),
                             -1);
            if (define_variable_type(cmd.lhs) == VARIABLE_DOUBLE &&
                define_variable_type(cmd.rhs) == VARIABLE_DOUBLE)
                new_variable(cmd.store, VARIABLE_INT, NULL, -1,
                             get_source_double(cmd.lhs) -
                                 get_source_double(cmd.rhs));
        }
    }
    else if (!strcmp(cmd.cmd, "multiply"))
    {
        if (strcmp(cmd.store, ""))
        {
            if (define_variable_type(cmd.lhs) == VARIABLE_INT &&
                define_variable_type(cmd.rhs) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_INT, NULL,
                             get_source_int(cmd.lhs) * get_source_int(cmd.rhs),
                             -1);
            if (define_variable_type(cmd.lhs) == VARIABLE_DOUBLE &&
                define_variable_type(cmd.rhs) == VARIABLE_DOUBLE)
                new_variable(cmd.store, VARIABLE_INT, NULL, -1,
                             get_source_double(cmd.lhs) *
                                 get_source_double(cmd.rhs));
        }
    }
    else if (!strcmp(cmd.cmd, "divide"))
    {
        if (strcmp(cmd.store, ""))
        {
            if (define_variable_type(cmd.lhs) == VARIABLE_INT &&
                define_variable_type(cmd.rhs) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             get_source_int(cmd.lhs) / get_source_int(cmd.rhs));
            if (define_variable_type(cmd.lhs) == VARIABLE_DOUBLE &&
                define_variable_type(cmd.rhs) == VARIABLE_DOUBLE)
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             get_source_double(cmd.lhs) *
                                 get_source_double(cmd.rhs));
            if (define_variable_type(cmd.lhs) == VARIABLE_DOUBLE &&
                define_variable_type(cmd.rhs) == VARIABLE_INT)
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             get_source_double(cmd.lhs) *
                                 get_source_int(cmd.rhs));
            if (define_variable_type(cmd.lhs) == VARIABLE_INT &&
                define_variable_type(cmd.rhs) == VARIABLE_DOUBLE)
                new_variable(cmd.store, VARIABLE_DOUBLE, NULL, -1,
                             get_source_int(cmd.lhs) *
                                 get_source_double(cmd.rhs));
        }
    }
    else if (!strcmp(cmd.cmd, "mod"))
    {
        if (strcmp(cmd.store, "") &&
            define_variable_type(cmd.lhs) == VARIABLE_INT &&
            define_variable_type(cmd.rhs) == VARIABLE_INT &&
            get_source_int(cmd.rhs) != 0)
        {
            new_variable(cmd.store, VARIABLE_INT, NULL,
                         get_source_int(cmd.lhs) % get_source_int(cmd.rhs), -1);
        }
    }
    else if (!strcmp(cmd.cmd, "set"))
    {
        if (cmd.lhs_type == VARIABLE_STRING)
        {
            found = 0;
            varname = sdsempty();
            if (cmd.lhs[0] == '{' && cmd.lhs[1] == '{' &&
                cmd.lhs[strlen(cmd.lhs) - 1] == '}' &&
                cmd.lhs[strlen(cmd.lhs) - 2] == '}')
                varname = sdscpylen(varname, cmd.lhs + 2, sdslen(cmd.lhs) - 4);
            else
                varname = sdscpylen(varname, cmd.lhs, sdslen(cmd.lhs));

            for (k = 0; k < variable_count; k++)
            {
                if (!strcmp(variables[k].name, varname))
                {
                    found = 1;
                    if (variables[k].type == VARIABLE_STRING)
                    {
                        variables[k].valuestring =
                            sdscpylen(variables[k].valuestring, "", 0);
                        if (cmd.rhs_type == VARIABLE_STRING)
                            variables[k].valuestring = sdscatprintf(
                                variables[k].valuestring, "%s", cmd.rhs);
                        else if (cmd.rhs_type == VARIABLE_INT)
                            variables[k].valuestring = sdscatprintf(
                                variables[k].valuestring, "%d", cmd.rhs_int);
                        else if (cmd.rhs_type == VARIABLE_DOUBLE)
                            variables[k].valuestring = sdscatprintf(
                                variables[k].valuestring, "%f", cmd.rhs_double);
                    }
                    else if (variables[k].type == VARIABLE_INT)
                    {
                        if (cmd.rhs_type == VARIABLE_STRING)
                            variables[k].valueint = atoi(cmd.rhs);
                        else if (cmd.rhs_type == VARIABLE_INT)
                            variables[k].valueint = cmd.rhs_int;
                        else if (cmd.rhs_type == VARIABLE_DOUBLE)
                            variables[k].valueint = (int)cmd.rhs_double;
                    }
                    else if (variables[k].type == VARIABLE_DOUBLE)
                    {
                        if (cmd.rhs_type == VARIABLE_STRING)
                            variables[k].valuedouble = strtod(cmd.rhs, &endptr);
                        else if (cmd.rhs_type == VARIABLE_INT)
                            variables[k].valuedouble = cmd.rhs_double;
                        else if (cmd.rhs_type == VARIABLE_DOUBLE)
                            variables[k].valuedouble = (double)cmd.rhs_double;
                    }
                }
            }
            if (!found)
            {
                if (cmd.rhs_type == VARIABLE_STRING)
                    new_variable(cmd.lhs, VARIABLE_STRING, cmd.rhs, -1, -1);
                else if (cmd.rhs_type == VARIABLE_INT)
                    new_variable(cmd.lhs, VARIABLE_INT, NULL, cmd.rhs_int, -1);
                else if (cmd.rhs_type == VARIABLE_DOUBLE)
                    new_variable(cmd.lhs, VARIABLE_INT, NULL, -1,
                                 cmd.rhs_double);
            }
            sdsfree(varname);
        }
    }
    else if (!strcmp(cmd.cmd, "clear"))
    {
        found = 0;
        varname = sdsempty();
        if (cmd.source[0] == '{' && cmd.source[1] == '{' &&
            cmd.source[strlen(cmd.source) - 1] == '}' &&
            cmd.source[strlen(cmd.source) - 2] == '}')
            varname =
                sdscpylen(varname, cmd.source + 2, sdslen(cmd.source) - 4);
        else
            varname = sdscpylen(varname, cmd.source, sdslen(cmd.source));

        for (k = 0; k < variable_count; k++)
        {
            if (!strcmp(varname, variables[k].name))
            {
                found = 1;
                break;
            }
        }
        if (found)
        {
            vars_tmp = malloc(sizeof(variable) * (variable_count - 1));
            found = 0;
            for (int j = 0; j < variable_count; j++)
            {
                if (j == k)
                {
                    found = 1;
                    continue;
                }
                vars_tmp[j - found].name = sdsnew(variables[j].name);
                vars_tmp[j - found].valuestring =
                    sdsnew(variables[j].valuestring);
                vars_tmp[j - found].type = variables[j].type;
                vars_tmp[j - found].valueint = variables[j].valueint;
                vars_tmp[j - found].valuedouble = variables[j].valuedouble;
                vars_tmp[j - found].empty = variables[j].empty;
            }
            for (int j = 0; j < variable_count; j++)
            {
                sdsfree(variables[j].name);
                sdsfree(variables[j].valuestring);
            }
            free(variables);
            variables = vars_tmp;
            vars_tmp = NULL;
            variable_count--;
        }
        sdsfree(varname);
    }
    else if (!strcmp(cmd.cmd, "copy"))
    {
        if (!(cmd.lhs_type == VARIABLE_STRING &&
              cmd.rhs_type == VARIABLE_STRING))
            goto exit;

        var1_index = -1;
        var2_index = -1;
        varname = sdsempty();
        varname2 = sdsempty();
        if (cmd.lhs[0] == '{' && cmd.lhs[1] == '{' &&
            cmd.lhs[strlen(cmd.lhs) - 1] == '}' &&
            cmd.lhs[strlen(cmd.lhs) - 2] == '}')
            varname = sdscpylen(varname, cmd.lhs + 2, sdslen(cmd.lhs) - 4);
        else
            varname = sdscpylen(varname, cmd.lhs, sdslen(cmd.lhs));

        if (cmd.rhs[0] == '{' && cmd.rhs[1] == '{' &&
            cmd.rhs[strlen(cmd.rhs) - 1] == '}' &&
            cmd.rhs[strlen(cmd.rhs) - 2] == '}')
            varname2 = sdscpylen(varname2, cmd.rhs + 2, sdslen(cmd.rhs) - 4);
        else
            varname2 = sdscpylen(varname2, cmd.rhs, sdslen(cmd.rhs));

        for (k = 0; k < variable_count; k++)
        {
            if (!strcmp(variables[k].name, varname))
                var1_index = k;
            if (!strcmp(variables[k].name, varname2))
                var2_index = k;
        }
        if (var1_index == -1 || var2_index == -1)
        {
            sdsfree(varname);
            sdsfree(varname2);
            goto exit;
        }
        sdsfree(variables[var1_index].name);
        sdsfree(variables[var1_index].valuestring);
        variables[var1_index].name = sdsnew(variables[var2_index].name);
        variables[var1_index].valuestring =
            sdsnew(variables[var2_index].valuestring);
        variables[var1_index].type = variables[var2_index].type;
        variables[var1_index].valueint = variables[var2_index].valueint;
        variables[var1_index].valuedouble = variables[var2_index].valuedouble;
        variables[var1_index].empty = variables[var2_index].empty;
        sdsfree(varname);
        sdsfree(varname2);
    }
    else if (!strcmp(cmd.cmd, "assert_equals") ||
             !strcmp(cmd.cmd, "assert_not_equals") ||
             !strcmp(cmd.cmd, "assert_regex_match") ||
             !strcmp(cmd.cmd, "assert_not_regex_match") ||
             !strcmp(cmd.cmd, "assert_contains") ||
             !strcmp(cmd.cmd, "assert_not_contains") ||
             !strcmp(cmd.cmd, "assert_starts_with") ||
             !strcmp(cmd.cmd, "assert_not_starts_with") ||
             !strcmp(cmd.cmd, "assert_ends_with") ||
             !strcmp(cmd.cmd, "assert_not_ends_with") ||
             !strcmp(cmd.cmd, "assert_file_exists") ||
             !strcmp(cmd.cmd, "assert_file_not_exists") ||
             !strcmp(cmd.cmd, "assert_file_contains") ||
             !strcmp(cmd.cmd, "assert_file_not_contains"))
    {
        (*assert_count)++;
        if (cmd.lhs_type == VARIABLE_STRING)
            assert_lhs =
                to_str(get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
        else if (cmd.lhs_type == VARIABLE_INT)
            assert_lhs =
                to_str(get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
        else if (cmd.lhs_type == VARIABLE_DOUBLE)
            assert_lhs = to_str(
                get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));
        else
            assert_lhs = sdsempty();

        if (cmd.rhs_type == VARIABLE_STRING)
            assert_rhs =
                to_str(get_var_object(cmd.rhs, -1, -1, VARIABLE_STRING));
        else if (cmd.rhs_type == VARIABLE_INT)
            assert_rhs =
                to_str(get_var_object(NULL, cmd.rhs_int, -1, VARIABLE_INT));
        else if (cmd.rhs_type == VARIABLE_DOUBLE)
            assert_rhs = to_str(
                get_var_object(NULL, -1, cmd.rhs_double, VARIABLE_DOUBLE));
        else
            assert_rhs = sdsempty();

        if (!strcmp(cmd.cmd, "assert_equals"))
        {
            if (!strcmp(assert_lhs, assert_rhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_not_equals"))
        {
            if (strcmp(assert_lhs, assert_rhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_regex_match"))
        {
            if (regex_pass(assert_rhs, assert_lhs, sdslen(assert_lhs)))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_not_regex_match"))
        {
            if (!regex_pass(assert_rhs, assert_lhs, sdslen(assert_lhs)))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_contains"))
        {
            if (strstr(assert_lhs, assert_rhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_not_contains"))
        {
            if (!strstr(assert_lhs, assert_rhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_starts_with"))
        {
            if (!strncmp(assert_lhs, assert_rhs, strlen(assert_rhs)))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_not_starts_with"))
        {
            if (strncmp(assert_lhs, assert_rhs, strlen(assert_rhs)))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_ends_with"))
        {
            if ((!(strlen(assert_rhs) > strlen(assert_lhs))) &&
                (!(strncmp(assert_lhs + strlen(assert_lhs) - strlen(assert_rhs),
                           assert_rhs, strlen(assert_rhs)))))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_not_ends_with"))
        {
            if (!((!(strlen(assert_rhs) > strlen(assert_lhs))) &&
                  (!(strncmp(assert_lhs + strlen(assert_lhs) -
                                 strlen(assert_rhs),
                             assert_rhs, strlen(assert_rhs))))))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_file_exists"))
        {
            if (file_exists(assert_lhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_file_not_exists"))
        {
            if (!file_exists(assert_lhs))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_file_contains") ||
                 !strcmp(cmd.cmd, "assert_file_not_contains"))
        {
            if (!file_exists(assert_lhs))
            {
                result = 0;
                sdsfree(assert_lhs);
                sdsfree(assert_rhs);
                goto exit;
            }
            fptr = fopen(assert_lhs, "rt");
            if (fptr == NULL)
            {
                perror("fopen");
                result = 0;
                sdsfree(assert_lhs);
                sdsfree(assert_rhs);
                goto exit;
            }
            fseek(fptr, 0, SEEK_END);
            k = ftell(fptr);
            fseek(fptr, 0, SEEK_SET);
            if (k < 0)
            {
                perror("ftell");
                fclose(fptr);
                sdsfree(assert_lhs);
                sdsfree(assert_rhs);
                goto exit;
            }

            file_content = sdsempty();
            file_content = sdsgrowzero(file_content, k);
            fread(file_content, 1, k, fptr);
            if (!strcmp(cmd.cmd, "assert_file_contains"))
            {
                if (strstr(file_content, assert_rhs))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                if (strstr(file_content, assert_rhs) == NULL)
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            sdsfree(file_content);
            fclose(fptr);
        }

        sdsfree(assert_lhs);
        sdsfree(assert_rhs);
    }
    else if (!strcmp(cmd.cmd, "assert_less") ||
             !strcmp(cmd.cmd, "assert_greater") ||
             !strcmp(cmd.cmd, "assert_less_or_equal") ||
             !strcmp(cmd.cmd, "assert_greater_or_equal"))
    {
        (*assert_count)++;
        if (cmd.lhs_type == VARIABLE_STRING)
            assert_lhs_double =
                to_double(get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
        else if (cmd.lhs_type == VARIABLE_INT)
            assert_lhs_double =
                to_double(get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
        else if (cmd.lhs_type == VARIABLE_DOUBLE)
            assert_lhs_double = to_double(
                get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));

        if (cmd.rhs_type == VARIABLE_STRING)
            assert_rhs_double =
                to_double(get_var_object(cmd.rhs, -1, -1, VARIABLE_STRING));
        else if (cmd.rhs_type == VARIABLE_INT)
            assert_rhs_double =
                to_double(get_var_object(NULL, cmd.rhs_int, -1, VARIABLE_INT));
        else if (cmd.rhs_type == VARIABLE_DOUBLE)
            assert_rhs_double = to_double(
                get_var_object(NULL, -1, cmd.rhs_double, VARIABLE_DOUBLE));

        if (!strcmp(cmd.cmd, "assert_less"))
        {
            if (assert_lhs_double < assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_greater"))
        {
            if (assert_lhs_double > assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_less_or_equal"))
        {
            if (assert_lhs_double <= assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_greater_or_equal"))
        {
            if (assert_lhs_double >= assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
    }
    else if (!strcmp(cmd.cmd, "assert_exitcode") ||
             !strcmp(cmd.cmd, "assert_exitcode_less") ||
             !strcmp(cmd.cmd, "assert_exitcode_greater"))
    {
        (*assert_count)++;
        assert_lhs_double = exitcode;

        if (cmd.lhs_type == VARIABLE_STRING)
            assert_rhs_double =
                to_double(get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
        else if (cmd.lhs_type == VARIABLE_INT)
            assert_rhs_double =
                to_double(get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
        else if (cmd.lhs_type == VARIABLE_DOUBLE)
            assert_rhs_double = to_double(
                get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));

        if (!strcmp(cmd.cmd, "assert_exitcode"))
        {
            if (assert_lhs_double == assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_exitcode_less"))
        {
            if (assert_lhs_double < assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_exitcode_greater"))
        {
            if (assert_lhs_double > assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
    }
    else if (!strcmp(cmd.cmd, "assert_duration_less_than") ||
             !strcmp(cmd.cmd, "assert_duration_greater_than"))
    {
        (*assert_count)++;
        assert_lhs_double = duration;

        if (cmd.lhs_type == VARIABLE_STRING)
            assert_rhs_double =
                to_double(get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
        else if (cmd.lhs_type == VARIABLE_INT)
            assert_rhs_double =
                to_double(get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
        else if (cmd.lhs_type == VARIABLE_DOUBLE)
            assert_rhs_double = to_double(
                get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));

        if (!strcmp(cmd.cmd, "assert_duration_less_than"))
        {
            if (assert_lhs_double < assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
        else if (!strcmp(cmd.cmd, "assert_duration_greater_than"))
        {
            if (assert_lhs_double > assert_rhs_double)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
    }
    else if (!strcmp(cmd.cmd, "log"))
    {
        if (cmd.source[0] == '{' && cmd.source[1] == '{' &&
            cmd.source[strlen(cmd.source) - 1] == '}' &&
            cmd.source[strlen(cmd.source) - 2] == '}')
            printf("Variable name: %s\n", cmd.source);
        else
        {
            printf("Value: %s\n", cmd.source);
            goto exit;
        }
        if (define_variable_type(cmd.source) == VARIABLE_INT)
        {
            printf("Value: %d\n", source_int);
        }
        else if (define_variable_type(cmd.source) == VARIABLE_DOUBLE)
        {
            printf("Value: %f\n", source_double);
        }
        else
        {
            printf("Value: %s\n", source_string);
        }
    }
    else if (!strcmp(cmd.cmd, "dump"))
    {
        for (k = 0; k < variable_count; k++)
        {
            printf("Variable %d:\n", k);
            printf("\tVariable name: %s\n", variables[k].name);
            if (variables[k].type == VARIABLE_STRING)
                printf("\tVariable value: %s\n\n", variables[k].valuestring);
            else if (variables[k].type == VARIABLE_INT)
                printf("\tVariable value: %d\n\n", variables[k].valueint);
            else if (variables[k].type == VARIABLE_DOUBLE)
                printf("\tVariable value: %f\n\n", variables[k].valuedouble);
        }
    }
    else if (!strcmp(cmd.cmd, "extract_substring"))
    {
        if (cmd.lhs_type == VARIABLE_STRING)
            var1_index = (int)to_double(
                get_var_object(cmd.lhs, -1, -1, VARIABLE_STRING));
        else if (cmd.lhs_type == VARIABLE_INT)
            var1_index = (int)to_double(
                get_var_object(NULL, cmd.lhs_int, -1, VARIABLE_INT));
        else if (cmd.lhs_type == VARIABLE_DOUBLE)
            var1_index = (int)to_double(
                get_var_object(NULL, -1, cmd.lhs_double, VARIABLE_DOUBLE));
        else
            var1_index = 0;

        if (cmd.rhs_type == VARIABLE_STRING)
            var2_index = (int)to_double(
                get_var_object(cmd.rhs, -1, -1, VARIABLE_STRING));
        else if (cmd.rhs_type == VARIABLE_INT)
            var2_index = (int)to_double(
                get_var_object(NULL, cmd.rhs_int, -1, VARIABLE_INT));
        else if (cmd.rhs_type == VARIABLE_DOUBLE)
            var2_index = (int)to_double(
                get_var_object(NULL, -1, cmd.rhs_double, VARIABLE_DOUBLE));
        else
            var2_index = 0;

        if (var1_index < 0)
            var1_index = strlen(source_string) - (var1_index * -1);

        if (var2_index < 0)
            var2_index = strlen(source_string) - (var2_index * -1);

        if (var1_index > var2_index)
        {
            k = var2_index;
            var2_index = var1_index;
            var1_index = k;
        }

        assert_lhs =
            sdsnewlen(source_string + var1_index, var2_index - var1_index + 1);
        new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
        sdsfree(assert_lhs);
    }
    else if (!strcmp(cmd.cmd, "extract_regex"))
    {
        if (define_variable_type(cmd.source) == VARIABLE_STRING &&
            cmd.lhs_type == VARIABLE_STRING && cmd.rhs_type == VARIABLE_INT)
        {
            assert_lhs = regex_extract(cmd.lhs, source_string,
                                       sdslen(source_string), cmd.rhs_int);
            new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
            sdsfree(assert_lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "get_line"))
    {
        k = 0;
        for (int j = 0; j < cmd.lhs_int; j++)
        {
            while (source_string[k++] != '\n')
                ;
        }
        var1_index = k;
        var2_index = k + 1;
        while (source_string[var2_index] != '\n')
            var2_index++;
        assert_lhs =
            sdsnewlen(source_string + var1_index, var2_index - var1_index - 1);
        new_variable(cmd.store, VARIABLE_STRING, assert_lhs, -1, -1);
        sdsfree(assert_lhs);
    }
    else if (!strcmp(cmd.cmd, "get_line_count"))
    {
        k = 0;
        for (size_t j = 0; j < strlen(source_string); j++)
        {
            if (source_string[j] == '\n')
                k++;
        }
        new_variable(cmd.store, VARIABLE_INT, NULL, k, -1);
    }
    else if (!strcmp(cmd.cmd, "length"))
    {
        if (define_variable_type(cmd.source) != VARIABLE_STRING)
            goto exit;

        k = strlen(source_string);
        new_variable(cmd.store, VARIABLE_INT, NULL, k, -1);
    }
    else if (!strcmp(cmd.cmd, "count_substring"))
    {
        if (define_variable_type(cmd.source) != VARIABLE_STRING ||
            cmd.lhs_type != VARIABLE_STRING)
            goto exit;

        k = 0;
        endptr = source_string;
        while ((endptr = strstr(endptr, cmd.lhs)) != NULL)
        {
            k++;
            endptr++;
        }
        new_variable(cmd.store, VARIABLE_INT, NULL, k, -1);
    }
    else if (!strcmp(cmd.cmd, "split"))
    {
        if (define_variable_type(cmd.source) != VARIABLE_STRING ||
            cmd.lhs_type != VARIABLE_STRING || cmd.rhs_type != VARIABLE_INT ||
            !strcmp(cmd.store, ""))
            goto exit;

        k = 0;
        endptr = strtok(source_string, cmd.lhs);
        while (endptr != NULL)
        {
            if (k++ == cmd.rhs_int)
            {
                new_variable(cmd.store, VARIABLE_STRING, endptr, -1, -1);
                break;
            }
            endptr = strtok(NULL, cmd.lhs);
        }
    }
    else if (!strcmp(cmd.cmd, "count_matches"))
    {
        if (define_variable_type(cmd.source) != VARIABLE_STRING ||
            cmd.lhs_type != VARIABLE_STRING || !strcmp(cmd.store, ""))
            goto exit;

        new_variable(cmd.store, VARIABLE_INT, NULL,
                     regex_count(cmd.lhs, source_string, sdslen(source_string)),
                     -1);
    }

exit:
    sdsfree(source_string);

    *output_str = sdscpylen(*output_str, output, sdslen(output));
    *result_val = result;
    sdsfree(output);
    free_command(&cmd);
}

void new_variable(char *name, int type, char *valuestring, int valueint,
                  double valuedouble)
{
    if (variable_count == 0)
    {
        variables = malloc(sizeof(variable));
        variable_count++;
    }
    else
    {
        variables = realloc(variables, sizeof(variable) * ++variable_count);
    }

    variables[variable_count - 1].type = type;
    variables[variable_count - 1].name = sdsnew(name);
    variables[variable_count - 1].empty = 0;
    if (type == VARIABLE_STRING)
    {
        variables[variable_count - 1].valuestring = sdsnew(valuestring);
        variables[variable_count - 1].valueint = -1;
        variables[variable_count - 1].valuedouble = -1;
    }
    else if (type == VARIABLE_INT)
    {
        variables[variable_count - 1].valuestring = NULL;
        variables[variable_count - 1].valueint = valueint;
        variables[variable_count - 1].valuedouble = -1;
    }
    else if (type == VARIABLE_DOUBLE)
    {
        variables[variable_count - 1].valuestring = NULL;
        variables[variable_count - 1].valueint = -1;
        variables[variable_count - 1].valuedouble = valuedouble;
    }
}

char *get_source_str(char *source, char *output)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return output;

    sds varname;
    int i;
    int index = -1;
    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return source;
    return variables[index].valuestring;
}

int get_source_int(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname;
    int i;
    int index = -1;

    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }

    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return atoi(source);
    return variables[index].valueint;
}

double get_source_double(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname;
    int i;
    int index = -1;
    char *endptr;

    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }

    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1)
        return strtod(source, &endptr);
    return variables[index].valuedouble;
}

double get_source_non_char(char *source)
{
    if (!strcmp(source, "{{output}}") || !strcmp(source, "output"))
        return -1;

    sds varname;
    int i;
    int index = -1;
    char *endptr;

    if (source[0] == '{' && source[1] == '{' &&
        source[strlen(source) - 1] == '}' && source[strlen(source) - 2] == '}')
    {
        varname = sdsnewlen(source + 2, strlen(source) - 4);
    }
    else
    {
        varname = sdsnewlen(source, strlen(source));
    }

    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, varname))
        {
            index = i;
            break;
        }
    }

    sdsfree(varname);
    if (index == -1 || variables[index].type == VARIABLE_STRING)
        return strtod(source, &endptr);

    if (variables[index].type == VARIABLE_INT)
        return (double)variables[index].valueint;
    else
        return variables[index].valuedouble;
}

int define_variable_type(char *varname)
{
    sds variablename;
    int i;
    int type = -1;
    if (!strcmp(varname, ""))
        return -1;
    if (varname[0] == '{' && varname[1] == '{' &&
        varname[strlen(varname) - 1] == '}' &&
        varname[strlen(varname) - 2] == '}')
    {
        variablename = sdsnewlen(varname + 2, strlen(varname) - 4);
    }
    else
    {
        variablename = sdsnew(varname);
    }

    if (!strcmp(varname, "{{output}}") || !strcmp(varname, "output"))
    {
        sdsfree(variablename);
        return VARIABLE_STRING;
    }
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(variables[i].name, variablename))
        {
            type = variables[i].type;
        }
    }
    sdsfree(variablename);
    if (type == -1)
    {
        return VARIABLE_STRING;
    }
    return type;
}

sds to_str(variable var)
{
    sds stringified_str = sdsempty();
    if (var.type == VARIABLE_STRING)
    {
        stringified_str = sdscpylen(stringified_str, var.valuestring,
                                    strlen(var.valuestring));
    }
    else if (var.type == VARIABLE_INT)
    {
        stringified_str = sdscatprintf(stringified_str, "%d", var.valueint);
    }
    else if (var.type == VARIABLE_DOUBLE)
    {
        if (fabs(var.valuedouble - floor(var.valuedouble)) < EPSILON)
            stringified_str =
                sdscatprintf(stringified_str, "%d", (int)var.valuedouble);
        else
            stringified_str =
                sdscatprintf(stringified_str, "%f", var.valuedouble);
    }

    destroy_empty_variable(var);
    return stringified_str;
}

double to_double(variable var)
{
    double converted;
    char *end_ptr;
    if (var.type == VARIABLE_STRING)
    {
        converted = strtod(var.valuestring, &end_ptr);
        destroy_empty_variable(var);
        return converted;
    }
    else if (var.type == VARIABLE_INT)
    {
        destroy_empty_variable(var);
        return var.valueint;
    }
    else if (var.type == VARIABLE_DOUBLE)
    {
        destroy_empty_variable(var);
        return var.valuedouble;
    }

    destroy_empty_variable(var);
    return -1;
}

variable get_var_object(char *name, int name_int, double name_double, int type)
{
    variable emptyobj = {-1, NULL, NULL, -1, -1, 1};
    sds namesds;
    int found = 0;
    int i;
    int name_type;
    char *endptr;

    if (name == NULL)
    {
        if (type == VARIABLE_INT)
        {
            emptyobj.valueint = name_int;
        }
        else if (type == VARIABLE_DOUBLE)
        {
            emptyobj.valuedouble = name_double;
        }
        emptyobj.type = type;
        return emptyobj;
    }
    if (name[0] == '{' && name[1] == '{' && name[strlen(name) - 1] == '}' &&
        name[strlen(name) - 2] == '}')
        namesds = sdsnewlen(name + 2, strlen(name) - 4);
    else
        namesds = sdsnewlen(name, strlen(name));
    for (i = 0; i < variable_count; i++)
    {
        if (!strcmp(namesds, variables[i].name))
        {
            found = 1;
            break;
        }
    }

    sdsfree(namesds);
    if (found)
        return variables[i];
    else
    {
        name_type = define_variable_type(name);
        if (name_type == VARIABLE_STRING)
        {
            emptyobj.valuestring = sdsnewlen(name, strlen(name));
            emptyobj.valueint = -1;
            emptyobj.valuedouble = -1;
        }
        else if (name_type == VARIABLE_INT)
        {
            emptyobj.valuestring = sdsempty();
            emptyobj.valueint = atoi(name);
            emptyobj.valuedouble = -1;
        }
        else if (name_type == VARIABLE_DOUBLE)
        {
            emptyobj.valuestring = sdsempty();
            emptyobj.valueint = -1;
            emptyobj.valuedouble = strtod(name, &endptr);
        }
        emptyobj.type = name_type;
        return emptyobj;
    }
}

void free_command(command *cmd)
{
    int i;

    sdsfree(cmd->cmd);
    sdsfree(cmd->source);
    sdsfree(cmd->store);
    sdsfree(cmd->lhs);
    sdsfree(cmd->rhs);
    for (i = 0; i < cmd->value_count; i++)
    {
        sdsfree(cmd->values[i]);
    }
    if (cmd->value_count)
        free(cmd->values);
}
