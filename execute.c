/*
Pars SARICA <pars@parssarica.com>
*/

#define _POSIX_C_SOURCE 200809L
#include "sds.h"
#include "testify.h"
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

sds execute(char *process_args[], char *input)
{
    sds output = sdsempty();
    posix_spawn_file_actions_t actions;
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    char buf[4096];
    ssize_t n;

    pipe(stdin_pipe);
    pipe(stdout_pipe);

    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, stdin_pipe[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, stdout_pipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, stdin_pipe[1]);
    posix_spawn_file_actions_addclose(&actions, stdout_pipe[0]);

    if (posix_spawnp(&pid, process_args[0], &actions, NULL, process_args,
                     environ) != 0)
    {
        perror("posix_spawn");
        exit(1);
    }

    posix_spawn_file_actions_destroy(&actions);

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    write(stdin_pipe[1], input, strlen(input));
    close(stdin_pipe[1]);

    while ((n = read(stdout_pipe[0], buf, sizeof(buf) - 1)) > 0)
    {
        buf[n] = 0;
        output = sdscatlen(output, buf, strlen(buf));
    }

    close(stdout_pipe[0]);
    waitpid(pid, NULL, 0);

    return output;
}
