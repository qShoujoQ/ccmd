#ifndef _CCMD_H
#define _CCMD_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef _WIN32

#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#else // Linux headers

#include <Windows.h>

#endif // Windows headers

typedef struct _ccmd
{
    char **args;
    int argc;
} CCMD;

#ifndef _WIN32

typedef pthread_t Process;

#else // Linux Process

typedef HANDLE Process;
/* Alternatinve: "cmd /C " */
const char *SHELL = "powershell -Command ";

#endif // Windows Process and SHELL

void ccmd_unload(CCMD *ccmd);
void ccmd_append(CCMD *ccmd, ...);
void ccmd_appenda(CCMD *ccmd, const char **args, int argc);
void ccmd_log(CCMD *ccmd);

bool ccmd_run_sync(CCMD *ccmd);
Process ccmd_run_async(CCMD *ccmd);
void ccmd_await(Process process);

#ifdef CCMD_IMPLEMENTATION

char *_ccmd_construct(CCMD *ccmd)
{
    char *cmd = NULL;
    int len = 0;

    for (int i = 0; i < ccmd->argc; i++)
    {
        len += strlen(ccmd->args[i]) + 1;
    }

#ifdef _WIN32
    len += strlen(SHELL);
#endif

    cmd = malloc(sizeof(char) * len);
    memset(cmd, 0, len);

#ifdef _WIN32
    strcat(cmd, SHELL);
#endif

    for (int i = 0; i < ccmd->argc; i++)
    {
        strcat(cmd, ccmd->args[i]);
        strcat(cmd, " ");
    }

    cmd[strlen(cmd) - 1] = '\0';

    return cmd;
}

void ccmd_unload(CCMD *ccmd)
{
    for (int i = 0; i < ccmd->argc; i++)
    {
        free(ccmd->args[i]);
    }
    free(ccmd->args);
    ccmd->args = NULL;
}

void ccmd_append(CCMD *ccmd, ...)
{
    va_list args;
    va_start(args, ccmd);

    int argc = ccmd->argc;
    ccmd->args = realloc(ccmd->args, sizeof(char *) * (argc + 1));
    ccmd->args[argc] = NULL;

    char *arg = va_arg(args, char *);
    while (arg != NULL)
    {
        ccmd->args[argc] = malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(ccmd->args[argc], arg);

        argc++;
        ccmd->args = realloc(ccmd->args, sizeof(char *) * (argc + 1));
        ccmd->args[argc] = NULL;

        arg = va_arg(args, char *);
    }

    ccmd->argc = argc;
    va_end(args);
}

void ccmd_appenda(CCMD *ccmd, const char **args, int argc)
{
    for (int i = 0; i < argc; i++)
    {
        ccmd->args = realloc(ccmd->args, sizeof(char *) * (ccmd->argc + 1));
        ccmd->args[ccmd->argc] = args[i];
        ccmd->argc++;
    }
    return;
}

void ccmd_log(CCMD *ccmd)
{
    printf("{ ");
    for (int i = 0; i < ccmd->argc - 1; i++)
    {
        printf("%s; ", ccmd->args[i]);
    }
    printf("%s }\n", ccmd->args[ccmd->argc - 1]);
    return;
}

bool ccmd_run_sync(CCMD *ccmd)
{
    char *cmd = _ccmd_construct(ccmd);

#ifndef _WIN32
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        free(cmd);
        return false;
    }

    if (child_pid == 0)
    {
        char *argv[ccmd->argc + 1];

        for (int i = 0; i < ccmd->argc; i++)
        {
            argv[i] = ccmd->args[i];
        }
        argv[ccmd->argc] = NULL;

        execvp(ccmd->args[0], argv);

        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        if (waitpid(child_pid, &status, 0) == -1)
        {
            perror("waitpid");
            free(cmd);
            return false;
        }
    }

#else

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif // _WIN32 Command execution

    free(cmd);

    return true;
}

void ccmd_await(Process process)
{
#ifndef _WIN32
    int status;
    if (waitpid(process, &status, 0) == -1)
    {
        perror("waitpid");
        return;
    }
#else
    WaitForSingleObject(process, INFINITE);
    CloseHandle(process);
#endif // _WIN32
    return;
}

Process ccmd_run_async(CCMD *ccmd)
{
    Process process;
    char *cmd = _ccmd_construct(ccmd);

#ifndef _WIN32
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        return 0;
    }

    if (child_pid == 0)
    {
        char *argv[ccmd->argc + 1];

        for (int i = 0; i < ccmd->argc; i++)
        {
            argv[i] = ccmd->args[i];
        }
        argv[ccmd->argc] = NULL;

        execvp(ccmd->args[0], argv);

        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        process = child_pid;
    }
#else

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return 0;
    }

    // CloseHandle(pi.hProcess); Done after awaiting
    CloseHandle(pi.hThread);

    process = pi.hProcess;
#endif // _WIN32 Command execution

    free(cmd);

    return process;
}

#endif // Implementation

#endif // _CCMD_H
