/*
Pars SARICA <pars@parssarica.com>
*/

#define _XOPEN_SOURCE 600
#include "sds.h"
#include "testify.h"
#include <fcntl.h>
#include <poll.h>
#include <pty.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

char **make_env(sds *env_vars, int env_count, int *memorysize)
{
    int count;
    char **env;
    int i;
    int j;
    for (count = 0; environ[count]; count++)
        ;

    env = malloc((count + env_count + 1) * sizeof(char *));
    for (i = 0; i < count; i++)
    {
        env[i] = strdup(environ[i]);
    }

    for (j = count; j < count + env_count; j++)
        env[j] = strdup(env_vars[j - count]);

    env[j] = NULL;
    *memorysize = count + env_count + 1;

    return env;
}

sds execute(char **process_args, char *input, int *fault, int *exitcode,
            sds *env_vars, int env_count)
{
    sds output = sdsempty();
    posix_spawn_file_actions_t actions;
    pid_t pid;
    char buf[4096];
    ssize_t n;
    int master_fd, slave_fd;
    int status;
    int memorysize;
    char **envp = make_env(env_vars, env_count, &memorysize);

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
                     envp) != 0)
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

    if (WIFEXITED(status))
    {
        *exitcode = WEXITSTATUS(status);
    }

    for (int i = 0; i < memorysize; i++)
    {
        free(envp[i]);
    }
    free(envp);
    return output;
}

process execute_background(char **process_args, sds *env_vars, int env_count)
{
    process proc = {-1, -1};
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attr;
    int memorysize;
    char **envp = make_env(env_vars, env_count, &memorysize);
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    char *slave_name;
    int slave;
    if (master < 0)
        return proc;

    grantpt(master);
    unlockpt(master);

    slave_name = ptsname(master);
    if (!slave_name)
        return proc;

    slave = open(slave_name, O_RDWR);
    if (slave < 0)
        return proc;

    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, slave, STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, slave, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, slave, STDERR_FILENO);

    posix_spawn_file_actions_addclose(&actions, master);
    posix_spawn_file_actions_addclose(&actions, slave);

    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
    posix_spawnattr_setpgroup(&attr, 0);

    if (posix_spawn(&proc.pid, process_args[0], &actions, &attr, process_args,
                    envp) != 0)
    {
        perror("posix_spawn");
        return proc;
    }

    posix_spawn_file_actions_destroy(&actions);
    close(slave);

    proc.pty_fd = master;

    for (int i = 0; i < memorysize; i++)
    {
        free(envp[i]);
    }
    free(envp);

    return proc;
}

void interact_write(process *pr, sds *input)
{
    if (sdslen(*input) > 0)
    {
        write(pr->pty_fd, *input, sdslen(*input));
    }
}

ssize_t interact_read(process *pr, char *output, size_t output_cap, int idle_ms)
{
    size_t total = 0;
    ssize_t n;
    struct pollfd pfd;
    int r;

    while (total < output_cap)
    {
        pfd = (struct pollfd){.fd = pr->pty_fd, .events = POLLIN | POLLHUP};
        r = poll(&pfd, 1, idle_ms);
        if (r == 0)
            break;

        if (r < 0)
            return -1;

        if (pfd.revents & POLLHUP)
            break;
        n = read(pr->pty_fd, output + total, output_cap - total);
        if (n <= 0)
        {
            break;
        }

        total += n;
    }

    return total;
}

void close_child(process *pr, int *fault, int *exitcode)
{
    int status;

    close(pr->pty_fd);
    waitpid(pr->pid, &status, 0);
    if (!WIFEXITED(status))
    {
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
        {
            *fault = 1;
        }
    }

    if (WIFEXITED(status))
    {
        *exitcode = WEXITSTATUS(status);
    }
}

int child_alive(process *pr, int *fault, int *exitcode)
{
    int status;
    pid_t r;

    if (pr->pid == -1)
        return 0;

    r = waitpid(pr->pid, &status, WNOHANG);

    if (r == 0)
        return 1;

    if (r == pr->pid)
    {
        if (!WIFEXITED(status))
        {
            if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
            {
                *fault = 1;
            }
        }

        if (WIFEXITED(status))
        {
            *exitcode = WEXITSTATUS(status);
        }

        pr->pid = -1;
        return 0;
    }
    return -1;
}

void kill_process(process *pr, int *fault, int *exitcode)
{
    int status;
    *fault = 0;
    *exitcode = -1;

    kill(-pr->pid, SIGTERM);

    for (int i = 0; i < 100; i++)
    {
        if (waitpid(pr->pid, &status, WNOHANG) == pr->pid)
            goto done;

        usleep(10000);
    }

    kill(-pr->pid, SIGKILL);

    for (int i = 0; i < 100; i++)
    {
        if (waitpid(pr->pid, &status, WNOHANG) == pr->pid)
            goto done;

        usleep(10000);
    }

done:
    if (!WIFEXITED(status))
    {
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
        {
            *fault = 1;
        }
    }

    if (WIFEXITED(status))
    {
        *exitcode = WEXITSTATUS(status);
    }

    close(pr->pty_fd);
    pr->pid = -1;
}
