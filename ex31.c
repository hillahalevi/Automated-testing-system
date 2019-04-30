//
// Created by hilla on 4/23/19.
//

// Hilla Halevi
// 208953083
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#define PRINT_ERROR_EXIT write(2, "Error in system call\n", strlen("Error in system call\n")); exit(-1);
#define FORK_ERROR perror("fork error - system call failure");
#define EXPECTED_NUM_OF_COMMAND_ARGS 3
#define CLOSE_FILES close(fd[0]); close(fd[1]);

typedef struct readDetails {
    ssize_t read_char;
    char buffer;
} readDetails;

//checks if files are the same
int same_files(char *argv1, char *argv2);

//checks if files are similar
int similar_files(char *argv1, char *argv2);

void skip_until_letter(int *fd, readDetails *details);

void try_2_open(char *file_name, int *fd, int place);


/**
 * @param argc num of args
 * @param argv the args
 * @return 1 - identical, 2 - different, 3 -  similar, -1 - stderr
 */
int main(int argc, char *argv[]) {
    if (argc != EXPECTED_NUM_OF_COMMAND_ARGS) {
        //illegal number of arguments
        PRINT_ERROR_EXIT
    }
    if (same_files(argv[1], argv[2])) {
        //same files!
        return 1;
    } else if (similar_files(argv[1], argv[2])) {
        //similar files!
        return 3;
    }
    // neither
    return 2;
}

/**
 * check if two files are the same - make use of the execvp command
 * @param argv1
 * @param argv2
 * @return 1/0
 */
int same_files(char *argv1, char *argv2) {
    // param list for execvp
    char *cmp_command[EXPECTED_NUM_OF_COMMAND_ARGS + 1] = {"cmp", argv1, argv2, NULL};
    int cmp_result = -1;
    pid_t pid;
    if ((pid = fork()) == -1)
        FORK_ERROR
    else if (pid == 0) { //child process
        // use execvp - to execute the cmp command
        execvp(cmp_command[0], cmp_command);
        PRINT_ERROR_EXIT  //only if execvp fails - shouldn't get here
    } else { // parent
        wait(&cmp_result);
    }
    //files same - cmp returns 0, otherwise returns 1
    if (!cmp_result) {
        return 1;
    }
    return 0;
}


/**
 * check if teo files are similar
 * @param argv1
 * @param argv2
 * @return 1/0
 */
int similar_files(char *argv1, char *argv2) {
    if (same_files(argv1, argv2))
        return 1;
    // file descriptor
    int fd[2];

    /**
     * try to gain access to files if fails - exits
     * else fd will hold the places to open
     */
    try_2_open(argv1, fd, 0);
    try_2_open(argv2, fd, 1);

    readDetails readDetails1;
    readDetails readDetails2;
    while (1) {

        //skip whitespaces - buffer should hole the next letter to compare
        skip_until_letter(fd, &readDetails1);
        skip_until_letter(fd, &readDetails2);


        if (readDetails1.read_char == 0 && readDetails2.read_char == 0) {
            //end of both files with no difference - close and similar
            CLOSE_FILES
            return 1;
        } else if (readDetails1.read_char != readDetails2.read_char) {
            // case that one file is finished but the other is not -
            CLOSE_FILES
            return 0;
        }

        /**
         * compare lower case of the letter-if its not a letter - there will be no harm
         */
        if (tolower(readDetails1.buffer) == tolower(readDetails2.buffer)) {
            continue;
        } else {
            // char is different, close and not similar
            CLOSE_FILES
            return 0;
        }
    }
}

/**
 * skip until reach of actual letter
 * update the details of the file with the read info
 * @param fd
 * @param details
 */
void skip_until_letter(int *fd, readDetails *details) {
    char file_buffer;
    ssize_t read_char;
    do {
        read_char = read(fd[0], &file_buffer, 1);
        if (read_char == -1) { PRINT_ERROR_EXIT }
        if (file_buffer != '\n' && file_buffer != ' ')
            break;
    } while (read_char != 0);
    details->buffer = file_buffer;
    details->read_char = read_char;
}

/**
 * tries to  open file
 * @param file_name
 * @param fd
 * @param place
 */
void try_2_open(char *file_name, int *fd, int place) {
    if (access(file_name, F_OK) == 0) {
        // place in 0
        fd[place] = open(file_name, O_RDONLY);
    } else {
        PRINT_ERROR_EXIT
    }

}
