/*
Pars SARICA <pars@parssarica.com>
*/

#include "sds.h"
#include <cjson/cJSON.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *get_input(char *msg)
{
    char *input_str = malloc(1024);
    printf("%s", msg);
    fgets(input_str, 1024, stdin);
    input_str[strstr(input_str, "\n") - input_str] = 0;
    return input_str;
}

void create_json()
{
    cJSON *json = cJSON_CreateObject();
    if (json == NULL)
        goto end;
    cJSON *binary_name_json = NULL;
    cJSON *testcases = NULL;
    cJSON *testcase_obj = NULL;
    cJSON *type_json = NULL;
    cJSON *testcase_name_json = NULL;
    cJSON *testcase_description_json = NULL;
    cJSON *testcase_input_json = NULL;
    cJSON *check_str_json = NULL;
    cJSON *cmdargs = NULL;
    cJSON *cmdarg = NULL;
    char *binary_name = "";
    char *testcase_name = "";
    char *testcase_description = "";
    char *testcase_input = "";
    char *check_str = "";
    char *cmdlineargs = "";
    char *filename = "";
    char *result;
    int fd;
    int type;
    int check_type;
    int c;
    while (1)
    {
        binary_name = get_input("Binary name> ");
        if (strcmp(binary_name, ""))
            break;
        free(binary_name);
    }
    binary_name_json = cJSON_CreateString(binary_name);
    if (binary_name_json == NULL)
        goto end;
    free(binary_name);
    cJSON_AddItemToObject(json, "binary", binary_name_json);
    testcases = cJSON_CreateArray();
    if (testcases == NULL)
        goto end;

    cJSON_AddItemToObject(json, "testcases", testcases);
    do
    {
        testcase_obj = cJSON_CreateObject();
        if (testcase_obj == NULL)
            goto end;
        cJSON_AddItemToArray(testcases, testcase_obj);
        printf("1) Normal\n2) Complex\n");
        type = -1;
        while (1)
        {
            printf("Test case type> ");
            type = getchar();
            if (type == '\n')
                continue;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            if (type == 49 || type == 50)
            {
                type -= 48;
                break;
            }
        }
        type_json = cJSON_CreateNumber(type);
        if (type_json == NULL)
            goto end;

        cJSON_AddItemToObject(testcase_obj, "type", type_json);
        testcase_name = get_input("Test case name> ");
        testcase_name_json = cJSON_CreateString(testcase_name);
        if (testcase_name_json == NULL)
            goto end;
        cJSON_AddItemToObject(testcase_obj, "name", testcase_name_json);
        free(testcase_name);

        testcase_description = get_input("Test case description> ");
        testcase_description_json = cJSON_CreateString(testcase_description);
        if (testcase_description_json == NULL)
            goto end;
        cJSON_AddItemToObject(testcase_obj, "description",
                              testcase_description_json);
        free(testcase_description);

        testcase_input = get_input("Test case STDIN input> ");
        testcase_input_json = cJSON_CreateString(testcase_input);
        if (testcase_input_json == NULL)
            goto end;
        cJSON_AddItemToObject(testcase_obj, "input", testcase_input_json);
        free(testcase_input);

        printf("1) Expected output\n");
        printf("2) Containing output\n");
        printf("3) Not expected output\n");
        printf("4) Not containing output\n");
        check_type = -1;
        while (1)
        {
            printf("Which one> ");
            check_type = getchar();
            if (check_type == '\n')
                continue;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            if (check_type == 49 || check_type == 50)
            {
                check_type -= 48;
                break;
            }
        }
        if (check_type == 1)
            check_str = get_input("Expected output> ");
        if (check_type == 2)
            check_str = get_input("Containing output> ");
        if (check_type == 3)
            check_str = get_input("Not expected output> ");
        if (check_type == 4)
            check_str = get_input("Not containing output> ");

        check_str_json = cJSON_CreateString(check_str);
        if (check_str_json == NULL)
            goto end;
        free(check_str);
        if (check_type == 1)
            cJSON_AddItemToObject(testcase_obj, "expectedOutput",
                                  check_str_json);
        if (check_type == 2)
            cJSON_AddItemToObject(testcase_obj, "containingOutput",
                                  check_str_json);
        if (check_type == 3)
            cJSON_AddItemToObject(testcase_obj, "notExpectedOutput",
                                  check_str_json);
        if (check_type == 4)
            cJSON_AddItemToObject(testcase_obj, "notContainingOutput",
                                  check_str_json);
        cmdargs = cJSON_CreateArray();
        if (cmdargs == NULL)
            goto end;

        cJSON_AddItemToObject(testcase_obj, "commandArgs", cmdargs);
        while (1)
        {
            cmdlineargs = get_input("Command line arguments> ");
            if (!strcmp(cmdlineargs, ""))
                break;

            cmdarg = cJSON_CreateString(cmdlineargs);
            if (cmdarg == NULL)
                goto end;

            cJSON_AddItemToArray(cmdargs, cmdarg);
            free(cmdlineargs);
        }
        printf("Press any key to continue or press 'c' to continue. ");
        c = getchar();
        if (c == (int)'C' || c == (int)'c')
            break;
    } while (1);

    result = cJSON_Print(json);
    if (result == NULL)
    {
        fprintf(stderr, "Unable to create the JSON object.\n");
    }

    while ((c = getchar()) != '\n' && c != EOF)
        ;
    filename = get_input("Filename (leave empty for default)> ");
    if (!strcmp(filename, ""))
    {
        free(filename);
        filename = malloc(15);
        strlcpy(filename, "testcases.json", 15);
    }
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        goto end;
    }
    write(fd, result, strlen(result));
    free(filename);
    free(result);
    close(fd);
end:
    cJSON_Delete(json);
}
