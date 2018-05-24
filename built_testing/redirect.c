#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>

/* Set limits to the buffer, for storing the arguments */
#define MAXLINE 4096

/* Set limits to input arguments */
#define MAXARGS 128

/* Set the limits to store the path of target directory */
#define PATH_MAX 256

/** testing parse arguments **/

void init(char **argv, char *outputFile, char *inputfd);

int main()
{
    pid_t pid;
    char buf[MAXLINE];
    char *start_tok;
    char *sep = " \t\n";
    char **strbuf = malloc(80 * sizeof(char *));
    char **argv = malloc(80 * sizeof(char *));
    char *temp;
    char cwd[PATH_MAX]; //init the size of cwd(get the current directory) to PATH_MAX
    char *outputFile = NULL;
    char *inputFile = NULL;
    int i, j, status;

    while (1)
    {
        printf("\n⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
        printf("\n Welcome to CS340 $ ");
        fflush(stderr);
        if (fgets(buf, MAXLINE, stdin) == NULL)
            break;
        i = 0;
        char *outputFile = NULL;
        char *inputFile = NULL;
        
        //parse arguments for file output
        while (1)
        {
            start_tok = strtok((i == 0) ? buf : NULL, sep);
            if (start_tok == NULL)
                break;
            strbuf[i++] = start_tok;
        }
        strbuf[i] = NULL;

        if (i == 0)
            continue;
        j = 0;
        i = 0;

        while (1)
        {
            temp = strbuf[i++];
            if (temp == NULL)
                break;
            switch (*temp)
            {
            case '>':
                if (temp[1] == 0)
                    temp = strbuf[i++];

                else
                    ++temp;
                outputFile = temp;
                break;

            default:
                argv[j++] = temp;
                break;
            }
        }
        argv[j] = NULL;
        init(argv, outputFile, inputFile);
    }
    exit(EXIT_SUCCESS);
}

void init(char **argv, char *outputFile, char *inputfd)
{

    pid_t pid;
    int status;
    if ((pid = fork()) == -1)
        err(1, "fork error");
    else if (pid == 0) //fork() successful
    {                  /* child */
        if (outputFile != NULL)
        {
            int fd;
            int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
            mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                               S_IROTH | S_IWOTH; /* rw-rw-rw- */
                               
            if ((fd = open(outputFile, openFlags, filePerms)) == -1)
            {
                perror("couldn't open output file.");
                exit(0);
            }

            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "dup2 failed");
            }
            close(fd);
        }
        execvp(argv[0], argv);
        err(127, "couldn't execute: %s", argv[0]);
    }

    /* parent */
    if ((pid = waitpid(pid, &status, 0)) == -1)
        err(1, "waitpid error");
}