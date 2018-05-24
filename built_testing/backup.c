#include <sys/wait.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXLINE 4096
#define MAXARGS 128
#define PATH_MAX 256

void parseArgs(char *, char **);
void non_builtin_commend(char **argv, char *buf);
void changeDir(char *path);


int main(void)
{
    char buf[MAXLINE];
    char *argv[MAXARGS];
    char cwd[PATH_MAX];

        // printf("⚡ %s  ↔%s", getenv("HOSTNAME"), getenv("PWD"));
    printf("⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));

    printf("\n Welcome to CS340 %♿ ");

    while (fgets(buf, MAXLINE, stdin) != NULL)
    {
        parseArgs(buf, argv);
        if ((strcmp(argv[0], "changeDir")) == 0)
        {
            changeDir(argv[1]);
        }
        
        else if ((strcmp(argv[0], ">") == 0))
        {
            /**
             * Exceute the output redirection
             */
        }

        else if ((strcmp(argv[0], "<") == 0))
        {
            /**
             * Exceute the input redirection
             */
        }
        else
        {
            non_builtin_commend(argv, buf);
        }
        // printf("⚡ %s  ↔%s", getenv("HOSTNAME"), getenv("PWD"));
        printf("⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
        printf("\n Welcome to CS340 %♿ ");
    }
    exit(0);
}

void parseArgs(char *str, char *vec[])
{
    static char *sep = " \t\n";
    char *tok_start;

    tok_start = strtok(str, sep);
    while (tok_start)
    {
        *vec++ = tok_start;
        tok_start = strtok(NULL, sep);
    }
    *vec = NULL;
}

void non_builtin_commend(char **argv, char *buf)
{
    pid_t pid;
    int status;
    if ((pid = fork()) == -1)
        err(1, "fork error");
    else if (pid == 0)
    { /* child */
        execvp(argv[0], argv);
        err(127, "couldn't execute: %s", buf);
    }

    /* parent */
    if ((pid = waitpid(pid, &status, 0)) == -1)
        err(1, "waitpid error");
}

void changeDir(char *path)
{
    if (path)
    {
        chdir(path);
    }
    else
    {
        chdir(getenv("HOME"));
    }
}
