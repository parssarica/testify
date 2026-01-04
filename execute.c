/*
Pars SARICA <pars@parssarica.com>
*/

#define _XOPEN_SOURCE 600
#include "sds.h"
#include "testify.h"
#include <pty.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

sds execute(char **process_args, char *input, int *fault)
{
    sds output = sdsempty();
    posix_spawn_file_actions_t actions;
    pid_t pid;
    char buf[4096];
    ssize_t n;
    int master_fd, slave_fd;
    int status;

    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) == -1)
    {
        perror("openpty");
        exit(1);
    }

    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, slave_fd, STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, slave_fd, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, slave_fd, STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, master_fd);

    if (posix_spawnp(&pid, process_args[0], &actions, NULL, process_args,
                     environ) != 0)
    {
        perror("posix_spawn");
        exit(1);
    }

    posix_spawn_file_actions_destroy(&actions);
    close(slave_fd);

    write(master_fd, input, strlen(input));

    while ((n = read(master_fd, buf, sizeof(buf))) > 0)
    {
        buf[n] = 0;
        output = sdscatlen(output, buf, strlen(buf));
    }

    close(master_fd);
    waitpid(pid, &status, 0);
    *fault = 0;
    if (!WIFEXITED(status))
    {
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
        {
            *fault = 1;
        }
    }

    return output;
}
