#include <sys/wait.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>

/* Set limits to the buffer, for storing the arguments */
#define MAXLINE 4096

/* Set limits to input arguments */
#define MAXARGS 128

/* Set the limits to store the path of target directory */
#define PATH_MAX 256

/* Set the READ_END and WRITE_END for pipe() system call*/
#define READ_END 0
#define WRITE_END 1

/* Global variables here*/
char *builtin_utilities[] = {"cd"};
char *inputFile;
char *outputFile;

char *inputFile2;
char *outputFile2;

int pipeFd[2];
bool pipeFlag;

/* Function declearation here */
void parseArguments(char *, char **, char **);
void launchShell(char **, char **, char *buf);
void cd_cmd(char *path);
void redirectInput(char *inputFile);
void redirectOutput(char *outputFile);
void _fork_without_pipeline(pid_t pid, int status, char **argv, char **argv2, char *buf);
void _fork_pipeline(pid_t pid, int status, char **argv, char **argv2, char *buf);

int main(void)
{
    char buf[MAXLINE];    //init buffer size to the MAXLINE availiable
    char *argv[MAXARGS];  //init the pointer to argv with the size of MAXARGS
    char *argv2[MAXARGS]; //init the pointer to argv with the size of MAXARGS
    char cwd[PATH_MAX];   //init the size of cwd(get the current directory) to PATH_MAX

    /* print the prompt with a welcome text with the Name and current working directory */
    printf("\n⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
    printf("\n Welcome to CS340 $ ");

    /* loops the stdin for any arguments */
    while (fgets(buf, MAXLINE, stdin) != NULL)
    {
        /* parse each argv[] into different tokens and store inside buf */
        parseArguments(buf, argv, argv2);

        if ((strcmp(argv[0], "cd")) == 0) //flaged cd token found
            cd_cmd(argv[1]);
        
        else
            launchShell(argv, argv2, buf);

        printf("\n⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
        printf("\n Welcome to CS340 $ ");
    }
    return 0;
}

/**
 * function to break input arugments into tokens
 */
void parseArguments(char *strBuf, char *inputVec[], char *inputVec2[])
{
    static char *sep = " \t\n\b"; //the arguments seperator
    char *tok_start;
    inputFile = NULL;
    inputFile2 = NULL;

    outputFile = NULL;
    outputFile2 = NULL;
    pipeFlag = false;
    tok_start = strtok(strBuf, sep); //break the *strBuf into token using strtok

    while (tok_start != NULL) //while reading the arguments
    {
        switch (*tok_start)
        {
        case '>':
            tok_start = strtok(NULL, sep); //next char to be pick
            outputFile = tok_start;
            tok_start = strtok(NULL, sep); //next char to be pick
            break;

        case '<':
            tok_start = strtok(NULL, sep); //next char to be pick
            inputFile = tok_start;
            tok_start = strtok(NULL, sep); //next char to be pick
            break;
        case '|':
            //store the next arguments/commands after '|' notations
            //hint: ->recursive the parseArgument() again to get the 2nd commands

            pipe(pipeFd);
            pipeFlag = true;
            tok_start = strtok(NULL, sep);
            *inputVec2++ = tok_start;      //store the token into the input vector
            tok_start = strtok(NULL, sep); //next char to be pick

            while (tok_start != NULL)
            {
                if ((strcmp(tok_start, ">")) == 0)
                {
                    tok_start = strtok(NULL, sep); //next char to be pick
                    outputFile2 = tok_start;
                    tok_start = strtok(NULL, sep); //next char to be pick
                }
                else
                {
                    *inputVec2++ = tok_start;
                    tok_start = strtok(NULL, sep); //next char to be pick
                }
            }
            break;

        default:
            *inputVec++ = tok_start;       //store the token into the input V=vector
            tok_start = strtok(NULL, sep); //next char to be pick
        }
        *inputVec = NULL;
    }
}

/**
 * init any non built-in functions 
 * eg, ls [-arg] ...
 */
void launchShell(char **argv, char **argv2, char *buf)
{
    pid_t pid;
    int status;
    if (pipeFlag == true)
        _fork_pipeline(pid, status, argv, argv2, buf);
    else
        _fork_without_pipeline(pid, status, argv, argv2, buf);
}

/**
 * Simple cd commend for built-in
 * return the path if exist, else return to HOME 
 * 
 */
void cd_cmd(char *path)
{
    if (path)
    {
        if (chdir(path) != 0)
            perror("chdir err");
    }
    else
    {
        // assume the "HOME" environment is exist
        if (chdir(getenv("HOME")))
            perror("chdir: Cannot find HOME environment in the list. ");
    }
}

void redirectOutput(char *outputFile)
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

    printf("\n-----------line 184-----------\n");
    if (dup2(fd, STDOUT_FILENO) == -1)
        fprintf(stderr, "dup2() failed 187");
    close(fd);
}

void redirectInput(char *inputFile)
{
    int fd;
    int openFlags = O_RDONLY;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; /* rw-rw-rw- */

    if ((fd = open(inputFile, openFlags, filePerms)) == -1)
    {
        perror("Could not open input file.");
        exit(0);
    }
    if (dup2(fd, STDIN_FILENO) == -1)
        fprintf(stderr, "dup2 failed 204");
    close(fd);
}

void _fork_without_pipeline(pid_t pid, int status, char **argv, char **argv2, char *buf)
{
    if ((pid = fork()) == -1)
        err(1, "fork error");
    else if (pid == 0) //fork() successful
    {                  /* child */
        if (outputFile != NULL)
            redirectOutput(outputFile);

        if (inputFile != NULL)
            redirectInput(inputFile);

        execvp(argv[0], argv);
        err(127, "couldn't execute: %s", argv[0]);
    }

    /* parent */
    if ((pid = waitpid(pid, &status, 0)) == -1)
        err(1, "waitpid error");
}

void _fork_pipeline(pid_t pid, int status, char **argv, char **argv2, char *buf)
{
    if ((pid = fork()) == 0)
    {
        if (dup2(pipeFd[WRITE_END], STDOUT_FILENO) == -1)
        {
            perror("dup2() failed 232");
        }
        close(pipeFd[READ_END]);
        if (outputFile != NULL)
            redirectOutput(outputFile);

        if (inputFile != NULL)
            redirectInput(inputFile);
        execvp(argv[0], argv);
        err(127, "couldn't execute: %s", argv[0]);

    } //2nd Child process

    else if ((pid = fork()) == 0)
    {
        dup2(pipeFd[READ_END], STDIN_FILENO);
        close(pipeFd[WRITE_END]);
        if (outputFile2 != NULL)
            redirectOutput(outputFile2);

        if (inputFile2 != NULL)
            redirectInput(inputFile2);
        execvp(argv2[0], argv2);
        err(127, "couldn't execute: %s", argv2[0]);
    }

    close(pipeFd[READ_END]);
    close(pipeFd[WRITE_END]);

    if ((pid = waitpid(pid, &status, 0)) == -1)
        err(1, "waitpid error");
}