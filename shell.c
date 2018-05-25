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
#include <fcntl.h>

/* Set limits to the buffer, for storing the arguments */
#define MAXLINE 4096

/* Set limits to input arguments */
#define MAXARGS 128

/* Set the limits to store the path of target directory */
#define PATH_MAX 256

/* Global variables here*/
char *builtin_utilities[] = {"cd"};
char *inputFile;
char *outputFile;

/* Function declearation here */
void parseArguments     (char *, char **);
void launchShell        (char **argv, char *buf);
void cd_cmd             (char *path);
void redirectInput      (char *inputFile);
void redirectOutput     (char *outputFile);
void print_prompt();

int main(void)
{
    char buf[MAXLINE];   //init buffer size to the MAXLINE availiable
    char *argv[MAXARGS]; //init the pointer to argv with the size of MAXARGS
    char cwd[PATH_MAX];  //init the size of cwd(get the current directory) to PATH_MAX
    system("clear");
    print_prompt();

    /* print the prompt with a welcome text with the Name and current working directory */
    printf("\n⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
    printf("\n Welcome to CS340 $ ");

    /* loops the stdin for any arguments */
    while (fgets(buf, MAXLINE, stdin) != NULL)
    {
        /* parse each argv[] into different tokens and store inside buf */
        parseArguments(buf, argv);
        int i;
        if ((strcmp(argv[0], "cd")) == 0) //flaged cd token found
            cd_cmd(argv[1]);

        else
        {
            printf("-----Found non-built-in Flag-----\n");
            launchShell(argv, buf);
        }

        printf("\n⚡ %s  ↔%s", getenv("HOSTNAME"), getcwd(cwd, sizeof(cwd)));
        printf("\n Welcome to CS340 $ ");
    }
    return 0;
}

/**
 * function to break input arugments into tokens
 */
void parseArguments(char *strBuf, char *inputVec[])
{
    static char *sep = " \t\n\b"; //the arguments seperator
    char *tok_start;
    inputFile = NULL;
    outputFile = NULL;
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
void launchShell(char **argv, char *buf)
{
    pid_t pid;
    int status;
    if ((pid = fork()) == -1)   
        err(1, "fork error");
    else if (pid == 0)          //fork() successful
    { /* child */
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

/**
 * Simple cd commend for built-in
 * return the path if exist, else return to HOME 
 * 
 */
void cd_cmd(char *path)
{
    printf("-----Found cd Flag-----\n");
    if (path)
    {
        if (chdir(path) != 0)
            perror("chdir err");
    }
    else
    {
        if (chdir(getenv("HOME")))
            perror("chdir err");
    }
}


void redirectOutput(char *outputFile)
{
    printf("-----Found output redirect Flag-----\n");
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
        fprintf(stderr, "dup2() failed");
    close(fd);
}

void redirectInput(char *inputFile)
{
    printf("-----Found input redirect Flag-----\n");    
    int fd;
    int openFlags = O_RDONLY;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; /* rw-rw-rw- */

    if ((fd = open(inputFile, openFlags, filePerms)) == -1)
    {
        perror("Could not open input file.");
        exit(0);
    }

    if (dup2(fd, STDIN_FILENO) == -1)
        fprintf(stderr, "dup2 failed");
    close(fd);
}



void print_prompt()
{
    printf("\n************************************************************************");
    printf("\n\n\n\t**** CS 340 ****");
    printf("\n\n\t- Self Study for System Programming -");
    printf("\n\n\t- Minimal Shell | Sokrattanak Utdorm Em -");
    printf("\n\n\t- Instructor; Lior Kadosh -");
    printf("\n\n\n\n************************************************************************");
}

void help_cmd()
{
    
}
