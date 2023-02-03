#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pa1.h"
#include "pipes.h"



int main(int argc, char **argv) {

    if (argc != 3) {
        return -1;
    }

    if (strcmp(argv[1], "-p") != 0) {
        return -1;
    }

    int number_of_child_procs = atoi(argv[2]);
    if (number_of_child_procs == 0) {
        return -1;
    }

    if (number_of_child_procs < 1 || number_of_child_procs > 10) {
        return -1;
    }

    struct pipes pipes_all[number_of_child_procs + 1][number_of_child_procs + 1];

    FILE *pi;
    pi = fopen("pipes.log", "a+");

    FILE *ev;
    ev = fopen("events.log", "a+");

    //pipes creation
    for (size_t i = 0; i < number_of_child_procs + 1; i++) {
        for (size_t j = 0; j < number_of_child_procs + 1; j++) {
            if (i != j) {
                int result_pipe = pipe(pipes_all[i][j].fd);
                fprintf(pi, "pipe opened i: %zu j: %zu\n", i, j);
                if (result_pipe == -1) {
                    return -1;
                }
            }
        }
    }

    size_t process_number = 0;

    for (size_t count = 1; count < number_of_child_procs+1; count++) {
        if (fork() == 0) {
            process_number = count;
            printf(log_started_fmt, (int ) process_number, getpid(), getppid());
            fprintf(ev, log_started_fmt, (int ) process_number, getpid(), getppid());

            //CLOSING PIPES FOR CHILD PROC
            // close pipes not working with current process
            for (size_t i = 0; i < number_of_child_procs+1; i++) {
                for (size_t j = 0; j < number_of_child_procs+1; j++) {
                    if (i != j && i != process_number && j != process_number) {
                        close(pipes_all[i][j].fd[0]);
                        close(pipes_all[i][j].fd[1]);
                        close(pipes_all[j][i].fd[0]);
                        close(pipes_all[j][i].fd[1]);
                    }
                }
            }
            for (size_t i = 0; i < number_of_child_procs+1; i++) {
                if (i != process_number) {
                    close(pipes_all[process_number][i].fd[0]);
                    close(pipes_all[i][process_number].fd[1]);
                }
            }

            //SENDING STARTED MESSAGE
            for (size_t j = 0; j < number_of_child_procs+1; j++) {
                if (process_number != j) {
                    char started_message[71];
                    sprintf(started_message, log_started_fmt, (int ) process_number, getpid(), getppid());
                    write(pipes_all[process_number][j].fd[1], started_message, 71);
                    printf("%d message printed in pipe i: %zu j: %zu\n", getpid(), process_number, j);
                }
            }

            for (size_t j = 0; j < number_of_child_procs+1; j++) wait(NULL);

            //READING STARTED MESSAGE
            for (size_t i = 1; i < number_of_child_procs+1; i++) {
                if (i != process_number) {
                    char child_receive[71];
                    printf("reading from i: %zu, j: %zu\n", i, process_number);
//                    fcntl(pipes_all[i][process_number].fd[0], F_SETFL, O_NONBLOCK);
                    read(pipes_all[i][process_number].fd[0], child_receive, 71);
                    printf("child with pid %d received message: %s from proc #%zu\n", getpid(), child_receive, i);
                }
            }

            for (size_t i = 1; i < number_of_child_procs+1; i++) wait(NULL);

            //log_received_all_started_fmt
            printf(log_received_all_started_fmt, (int ) process_number);
            fprintf(ev, log_received_all_started_fmt, (int ) process_number);

            //log_done_fmt
            char done_message[71];
            sprintf(done_message, log_done_fmt, (int) process_number);
            printf(log_done_fmt, (int) process_number);
            fprintf(ev, log_done_fmt, (int) process_number);

            //SENDING DONE MESSAGE
            for (size_t j = 0; j < number_of_child_procs + 1; j++) {
                if (j != process_number) {
                    write(pipes_all[process_number][j].fd[1], done_message, 71);
                    close(pipes_all[process_number][j].fd[1]);
//                    printf("pid %d, i: %zu, j: %zu DONE message printed\n", getpid(), i, j);
                }
            }


            for (size_t i = 1; i < number_of_child_procs + 1; i++) wait(NULL);

            //READING DONE MESSAGE
            for (size_t i = 1; i < number_of_child_procs+1; i++) {
                if (i != process_number) {
                    char child_done_receive[71];
                    printf("reading from i: %zu, j: %zu\n", i, process_number);
//                    fcntl(pipes_all[i][process_number].fd[0], F_SETFL, O_NONBLOCK);
                    read(pipes_all[i][process_number].fd[0], child_done_receive, 71);
                    close(pipes_all[i][process_number].fd[0]);
                    printf("child with pid %d received message: %s from proc #%zu\n", getpid(), child_done_receive, i);
                }
            }

            //log_received_all_done_fmt
            printf(log_received_all_done_fmt, (int ) process_number);
            fprintf(ev, log_received_all_done_fmt, (int ) process_number);

            exit(0);
        }
    }

    //CLOSING PIPES FOR PARENT PROC
    for (size_t i = 0; i < number_of_child_procs+1; i++) {
        for (size_t j = 0; j < number_of_child_procs+1; j++) {
            if (i != j && i != 0 && j != 0) {
                close(pipes_all[i][j].fd[0]);
                close(pipes_all[i][j].fd[1]);
                close(pipes_all[j][i].fd[0]);
                close(pipes_all[j][i].fd[1]);
            }
        }
    }
    for (size_t i = 0; i < number_of_child_procs+1; i++) {
        if (i != process_number) {
            close(pipes_all[process_number][i].fd[0]);
            close(pipes_all[i][process_number].fd[1]);
        }
    }

    for (size_t count = 1; count < number_of_child_procs+1; count++) wait(NULL);

    //READING STARTED MESSAGE BY PARENT
    for (size_t i = 1; i < number_of_child_procs+1; i++) {
        char receive[71];
//        fcntl(pipes_all[i][0].fd[0], F_SETFL, O_NONBLOCK);
        read(pipes_all[i][0].fd[0], receive, 71);
        printf("parent received: %s from proc #%zu\n", receive, i);
    }

    //READING DONE MESSAGE BY PARENT
    for (size_t i = 1; i < number_of_child_procs+1; i++) {
        char done_receive[71];
//        fcntl(pipes_all[i][0].fd[0], F_SETFL, O_NONBLOCK);
        read(pipes_all[i][0].fd[0], done_receive, 71);
        close(pipes_all[i][0].fd[0]);
        printf("parent received: %s from proc #%zu\n", done_receive, i);
    }

    fclose(ev);
    fclose(pi);

    return 0;
}
