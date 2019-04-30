//
// Created by hilla on 4/29/19.
//

/*
 * hilla halevi
 * 208953083
 */

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <stdio.h>
#include <memory.h>
#include <sys/stat.h>

//TODO ex31 check for similar bad
//TODO make sure no items left  behind
//TODO arrange


#define FILE_DEAFULT "hello.out"
#define MU_OUTPUT "my output.txt"
#define RUN "./hello.out"
#define EXPECTED_NUM_OF_COMMAND_ARGS 2
#define MAX_LINE_LENGTH 150
#define MAX_FILE_LENGTH 3*150
#define LS_COMMAND "/bin/ls"
#define PRINT_ERROR_EXIT write(2, "Error in system call\n", strlen("Error in system call\n")); exit(-1);
#define GREAT_JOB 100
#define SIMILAR 80
#define DIFFERENRT 60
#define TIMEOUT 40
#define COMPILE_ERROR 20
#define NO_FILES 0

//struct with the confo details
typedef struct confDetails {
    char directory_path[MAX_LINE_LENGTH];//the folder with the user names folders
    char input_path[MAX_LINE_LENGTH]; //the input path
    char right_output_path[MAX_LINE_LENGTH]; //the output path
} confDetails;


//struct with data for execute
typedef struct ToExe {
    int status;
    char userName[MAX_LINE_LENGTH];
    char path[MAX_LINE_LENGTH];
    char name[MAX_LINE_LENGTH];
} ToExe;


//find c file in folder
struct ToExe FindCfile(char *name, char *father);

// execute (a.out)
void ExecuteC(ToExe *exe, int fd_result, confDetails *confDetails1);

// write on result file the correct result
void WriteResult(int fd, char *name, int grade, int key);

// return reason for grade by some key
char *RetReason(int key);

//read a file into array
void readfile(char *fileName, char *lines) {
    int fd;
    ssize_t ret;
    char buf[MAX_LINE_LENGTH * 3];
    if ((fd = open(fileName, O_RDONLY)) < 0) {
        PRINT_ERROR_EXIT

    } else {
        while ((ret = read(fd, buf, sizeof(buf) - 1)) > 0) {
            buf[ret] = 0x00;
            if (strcmp(buf, "\n") == 0) {
                //empty line
                continue;
            }
            strcpy(lines, buf);
        }
        close(fd);
    }
}

//read conf details
void readLines(char *file, confDetails *confDetails1) {
    //TODO validate exictence of 3 lines!
    char lines[MAX_FILE_LENGTH];
    readfile(file, lines);
    char *split = strtok(lines, "\n");
    stpcpy(confDetails1->directory_path, split);
    split = strtok(NULL, "\n");
    stpcpy(confDetails1->input_path, split);
    split = strtok(NULL, "\n");
    stpcpy(confDetails1->right_output_path, split);


}

//get number of files
int getNumberofFiles(char *path) {
    int file_count = 0;
    DIR *dirp;
    struct dirent *entry;

    if ((dirp = opendir(path)) == NULL) {
        PRINT_ERROR_EXIT
    }
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_DIR) { /* If the entry is a directory */
            file_count++;
        }
    }
    closedir(dirp);
    return file_count - 2;
}

//get users
char *getUsers(char *directory_path) {
    char *fileName = "users.txt";
    int fd; //new file descriptor
    //open txt file - users list
    if ((fd = open(fileName, O_CREAT | O_TRUNC | O_RDWR, 0777)) < 0)//0644
    {
        PRINT_ERROR_EXIT
    }

    dup2(fd, 1);
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        execl(LS_COMMAND, LS_COMMAND, directory_path, NULL);//ls confDetails.directory_path
        //case of fail:
        PRINT_ERROR_EXIT
    } else {
        wait(&status);
        //return to the regular state of printing
        int one = dup(1);
        int two = dup(fd);
        dup2(one, 1);
        dup2(two, fd);

        if (close(fd) == -1) {
            PRINT_ERROR_EXIT //close failed
        }
        return fileName;


    }
}

//find a c file for specific user
ToExe FindCfile(char *name, char *father) {
    char *path = malloc(strlen(father) + strlen(name) + 10);
    strcpy(path, father);
    strcat(path, "/");
    strcat(path, name);
    DIR *dirp;
    struct dirent *entry;

    if ((dirp = opendir(path)) == NULL) {
        PRINT_ERROR_EXIT
    }
    struct ToExe toExe;
    toExe.name[0] = '\0';
    toExe.path[0] = '\0';
    toExe.userName[0] = '\0';
    toExe.status = 1;
    strcat(toExe.userName, name);
    while ((entry = readdir(dirp)) != NULL) {
        char *dot = strrchr(entry->d_name, '.');
        if (dot && strcmp(dot, ".c") == 0) {
            //its a C file!
            char copmlete[strlen(path) + strlen(entry->d_name) + 10];
            copmlete[0] = '\0';
            strcpy(copmlete, path);
            strcat(copmlete, "/");
            strcat(copmlete, entry->d_name);
            toExe.path[0] = '\0';
            strcpy(toExe.path, copmlete);
            strcat(toExe.name, entry->d_name);
            closedir(dirp);
            return toExe;

        }
        struct stat chackStat;
        if (S_ISDIR(chackStat.st_mode)) {
            /* If the entry is a directory */
            return FindCfile(entry->d_name, father);

        }
    }
    //NO_C_FILE
    toExe.status = -1;
    closedir(dirp);
    return toExe;
}

//compile
void compile(ToExe *toExe) {
    if (toExe->status == -1) {
        return;
    }
    int status;
    int pid = fork();
    if (pid == -1) {
        PRINT_ERROR_EXIT;
    }
    if (pid == 0) {

        //compiling the c file - giving it the user name
        char *gcc[] = {"gcc", "-o", FILE_DEAFULT, toExe->path, NULL};
        if (execvp("gcc", gcc) < 0) {


            PRINT_ERROR_EXIT//execlp FAILED

        }
    } else {
        wait(&status);
        int ret = WEXITSTATUS(status);//the return from gcc
        if (ret == 1) {//gcc failed!
            //COMPILATION_ERROR
            toExe->status = -2;
            return;
        }


    }
}

// write on result file the correct result
void WriteResult(int fd, char *name, int grade, int key) {

    if (grade < 0) {//not allow negative grade
        grade = 0;
    }
    char srtGrade[4];
    sprintf(srtGrade, "%d", grade);
    srtGrade[3] = '\0';
    char data[26 + strlen(name)];
    data[0] = '\0';
    strcpy(data, name);
    strcat(data, ",");
    strcat(data, srtGrade);
    strcat(data, ",");
    //the reason text
    strcat(data, RetReason(key));

    //write the data on result file
    int a = (int) write(fd, &data, strlen(data));
    if (a == -1) {
        PRINT_ERROR_EXIT //WRITE FAILED
    }
}

// return reason for grade by some key
char *RetReason(int key) {
    switch (key) {
        case 1:
            return "NO_C_FILE\n\0";
        case 2:
            return "COMPILATION_ERROR\n\0";
        case 3:
            return "TIMEOUT\n\0";
        case 4:
            return "BAD_OUTPUT\n\0";
        case 5:
            return "SIMILLAR_OUTPUT\n\0";
        case 6:
            return "GREAT_JOB\n\0";

        default:
            break;
    }
    return "";
}

//cmp
void Compare(confDetails *confDetails1, int fd_result, char *name) {
    int pid = fork();
    int status = 0;
    int key = 0;
    int grade = 0;
    if (pid == -1) {
        unlink(MU_OUTPUT);
        PRINT_ERROR_EXIT;
    }

    if (pid == 0) {
        int a = execl("cmp.out","./cmp.out", MU_OUTPUT, confDetails1->right_output_path,NULL);
        //char *args[] = {"cmp.out","./cmp.out",MU_OUTPUT, confDetails1->right_output_path, NULL};
        if (a < 0) {
            unlink(MU_OUTPUT);
            PRINT_ERROR_EXIT //A.OUT FAILED
        }
    } else {
        wait(&status);
        // unlink(myOut_f);
        int ret = WEXITSTATUS(status);//the return value from comparing
        switch (ret) {
            case 3:
                key = 4;
                grade = DIFFERENRT; //different
                break;
            case 2:
                key = 5;
                grade = SIMILAR; //similar
                break;
            case 1:
                key = 6;
                grade = GREAT_JOB; //same
                break;
            default:
                key = 4;
                grade = DIFFERENRT; //different
                break;
                //PRINT_ERROR_EXIT

        }
        WriteResult(fd_result, name, grade, key);
        unlink(MU_OUTPUT);
    }


}

//run
int run(int fd_in, int fd_myOut, int fd_result, char *name) {
    int status = 0;
    int save_fd0 = dup(0);
    int save_fd1 = dup(1);
    int save_fd_in = dup(fd_in);
    int save_fd_out = dup(fd_myOut);
    //to print output in myOutput file && read from input file
    dup2(fd_in, 0);
    dup2(fd_myOut, 1);

    pid_t pid = fork();
    if (pid < 0) {
        unlink(MU_OUTPUT);
        PRINT_ERROR_EXIT

    }
    if (pid == 0) {
        //execute the c file
        char *args[] = {RUN, NULL};
        if (execvp(RUN, args) < 0) {
            unlink(MU_OUTPUT);
            PRINT_ERROR_EXIT //A.OUT FAILED
        }
    } else {
        //check if the program run above 5 seconds
        sleep(5);
        int f = waitpid(pid, &status, WNOHANG);

        //return to the regular state of printing
        dup2(save_fd1, 1);
        dup2(save_fd0, 0);
        dup2(save_fd_in, fd_in);
        dup2(save_fd_out, fd_myOut);

        if (f == 0) {//program NOT finish in 5 seconds
            //TIMEOUT
            kill(pid, SIGSTOP);
            unlink(MU_OUTPUT);
            WriteResult(fd_result, name, TIMEOUT, 3);
            return -1;
        }

        //close input file
        if (close(fd_in) == -1) {
            unlink(MU_OUTPUT);
            PRINT_ERROR_EXIT //CLOSE FAILED
        }
        lseek(fd_myOut, 0, SEEK_SET);
    }
    return 0;
}

//execute (a.out)
void ExecuteC(ToExe *exe, int fd_result, confDetails *confDetails1) {
    lseek(fd_result, 0, SEEK_END);
    if (exe->status < 0) {
        if (exe->status == -1) {//NO_C_FILE
            WriteResult(fd_result, exe->userName, NO_FILES, 1);
        } else if (exe->status == -2) {//COMPILATION_ERROR
            WriteResult(fd_result, exe->userName, COMPILE_ERROR, 2);
        }

    } else {
        int fd_in; //input - file descriptor
        int fd_myOut;//my output - file descriptor

        //open input file
        if ((fd_in = open(confDetails1->input_path, O_RDWR, 0444)) < 0)//0644
        {
            PRINT_ERROR_EXIT //OPEN FAILED
        }

        //open my output file
        if ((fd_myOut = open(MU_OUTPUT, O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0)//0644
        {
            PRINT_ERROR_EXIT //OPEN FAILED
        }

        if (run(fd_in, fd_myOut, fd_result, exe->userName) == -1) {
            //time out
            return;
        }

        Compare(confDetails1, fd_result, exe->userName);

    }
}

//handle each user directory
void handleUsers(char *users, confDetails *confDetails, int fd) {
    char delim[] = "\n";

    char *ptr = strtok(users, delim);

    while (ptr != NULL) {
        //ptr now holds a user name

        //looking for c file
        ToExe exe = FindCfile(ptr, confDetails->directory_path);

        //compile
        compile(&exe);


        //execute it
        ExecuteC(&exe, fd, confDetails);//run the c file

        if (exe.status > 0) {
            unlink(FILE_DEAFULT);
        }

        ptr = strtok(NULL, delim);
    }


}

int main(int argc, char *argv[]) {
   // confo details
    confDetails confDetails;
    readLines(argv[1], &confDetails);

    //users details
    char *usersFile = getUsers(confDetails.directory_path);
    int numberOfUsers = getNumberofFiles(confDetails.directory_path);
    char users[MAX_LINE_LENGTH * numberOfUsers];
    readfile(usersFile, users);
    //after read delete users.txt
    unlink(usersFile);


    //open the result file
    int fd_result;
    if ((fd_result = open("results.csv", O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0) {
        PRINT_ERROR_EXIT //open failed;
    }
    lseek(fd_result, 0, SEEK_SET);
    handleUsers(users, &confDetails, fd_result);

    return 0;
}





