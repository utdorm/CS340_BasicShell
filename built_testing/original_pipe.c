#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef READ
#define READ 0
#endif

#ifndef WRITE
#define WRITE 1
#endif

void clearArgumentContainer (int argumentContainer[]);

int main() {
    /* professor-supplied variables for commands and command parsing */
    char *iPath, *oPath, *argv[20], buf[80], n, *p;
    int m, status, inword, continu;

    int start[20];

    /* flags for redirection (note: C does not have type bool; using integer value 0 or 1) */
    int inputRedirectFlag, outputRedirectFlag;

    /* variables for piping */
    int count, pipes;
    pid_t pid;

    /* pipes */
    int l_pipe[2], r_pipe[2];

    /* required container for handling arguments */
    int argumentContainer[20] = { 0 };

    while (1) {

        inword = m = continu = count = pipes = pid = 0;

        p = buf;

        /* redirection flags reset */
        inputRedirectFlag = outputRedirectFlag = 0;

        /* shell prompt */
        printf("\nshhh> ");

        /* command parsing */
        while ((n = getchar()) != '\n' || continu)
        {
            if (n == ' ') {
                if (inword)
                {
                    inword = 0;
                    *p++ = 0;
                }
            }
            else if (n == '\n')
                continu = 0;
            else if (n == '\\' && !inword)
                continu = 1;
            else {
                if (!inword)
                {
                    inword = 1;
                    argv[m++] = p;
                    *p++ = n;
                }
                else
                    *p++ = n;
            }
        } /* end of command parsing */

        *p++ = 0;
        argv[m] = 0;

        /* user wishes to terminate program */
        if (strcmp(argv[0], "exit") == 0)
            exit(0);

        /* manage redirection */
        while (argv[count] != 0) {
            if (strcmp(argv[count], "|") == 0) {
                argv[count] = 0;
                argumentContainer[pipes + 1] = count + 1;
                ++pipes;
            }
            else if (strcmp(argv[count], "<") == 0) {
                iPath = strdup(argv[count + 1]); /* copy string argument (file string) */
                argv[count] = 0;
                argv[count + 1] = 0;
                inputRedirectFlag = 1;
            }
            else if (strcmp(argv[count], ">") == 0) {
                oPath = strdup(argv[count + 1]); /* copy string argument (file string) */
                argv[count] = 0;
                argv[count + 1] = 0;
                outputRedirectFlag = 1;
            }
            else {
                argumentContainer[count] = count;
            }

            ++count;
        } /* end of redirection management */

        /* execute commands [<= in for-loop; n pipes require n+1 processes] */
        for (int index = 0; index <= pipes; ++index) {
            if (pipes > 0 && index != pipes) { /* if user has entered multiple commands with '|' */
                pipe(r_pipe); /* no pipe(l_pipe); r_pipe becomes next child's l_pipe */
            }

            /* switch-statement for command execution */
            switch (pid = fork()) {
                /* fork() error */
                case -1: perror("fork failed");
                         break;

                case 0: /* child process manages redirection and executes */
                       if ((index == 0) && (inputRedirectFlag == 1)) {
                           int inputFileDescriptor = open(iPath, O_RDONLY , 0400);
                           if (inputFileDescriptor == -1) {
                               perror("input file failed to open\n");
                               return(EXIT_FAILURE);
                           }
                           close(READ);
                           dup(inputFileDescriptor);
                           close(inputFileDescriptor);
                       } /* end of input redirection management */

                       if ((index == pipes) && (outputRedirectFlag == 1)) {
                           int outputFileDescriptor = open(oPath, O_WRONLY | O_CREAT, 0755);
                           if (outputFileDescriptor < 0) {
                               perror("output file failed to open\n");
                               return(EXIT_FAILURE);
                           }
                           close(WRITE);
                           dup(outputFileDescriptor);
                           close(outputFileDescriptor);
                       } /* end of output redirection management */

                        /* manage pipes if (a) first child process, (b) middle child process, or (c) third/final child process */
                        if (pipes > 0) {
                            if (index == 0){ /* first child process */
                                close(WRITE);
                                dup(r_pipe[WRITE]);
                                close(r_pipe[WRITE]);
                                close(r_pipe[READ]);
                            }
                            else if (index < pipes) { /* middle child process */
                                close(READ);
                                dup(l_pipe[READ]);
                                close(l_pipe[READ]);
                                close(l_pipe[WRITE]);
                                close(WRITE);
                                dup(r_pipe[WRITE]);
                                close(r_pipe[READ]);
                                close(r_pipe[WRITE]);
                            }
                            else { /* third/final child process */
                                close(READ);
                                dup(l_pipe[READ]);
                                close(l_pipe[READ]);
                                close(l_pipe[WRITE]);
                            }
                        }

                       /* execute command */
                       execvp(argv[argumentContainer[index]], &argv[argumentContainer[index]]);

                       /* if execvp() fails */
                       perror("execution of command failed\n");

                       break;

                default: /* parent process manages the pipes for child process(es) */
                        if (index > 0) {
                            close(l_pipe[READ]);
                            close(l_pipe[WRITE]);
                        }
                        l_pipe[READ] = r_pipe[READ];
                        l_pipe[WRITE] = r_pipe[WRITE];

                        /* parent waits for child process to complete */
                        wait(&status);

                        break;
            } /* end of switch-statement for command execution */
        } /* end of loop for all pipes */

        // clear all executed commands
        for (int i = 0; i < 20; ++i) {
            argv[i] = 0;
        }
    }
}

void clearArgumentContainer (int argumentContainer[]){
    // clear argument container
    for (int i = 0; i < 20; ++i) {
        argumentContainer[i] = 0;
    }
}