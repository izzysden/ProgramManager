#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define MAX_STR 1000

typedef struct swInfo {
    char * name;
    char * restart_count;
    char * start_time;
    char * reason;
    int restart;
}
swInfo;

typedef struct swParam {
    char * swArgv[4];
}
swParam;

typedef struct swStatus {
    pid_t pids[11];
    pid_t deadPid;
    int noOfArgv;
    swInfo sw_info[11];
    swParam sw_param[11];
}
swStatus;

swStatus mySwInfo;

char * rtrim(const char * s) {
    while (isspace( * s) || !isprint( * s)) ++s;
    return strdup(s);
}

char * ltrim(const char * s) {
    char * r = strdup(s);
    if (r != NULL) {
        char * fr = r + strlen(s) - 1;
        while ((isspace( * fr) || !isprint( * fr) || * fr == 0) && fr >= r) --fr;
        *++fr = 0;
    }
    return r;
}

char * trim(const char * s) {
    char * r = rtrim(s);
    char * f = ltrim(r);
    free(r);
    return f;
}

void readFileList() {
    FILE * fp;
    fp = fopen("./FileList.txt", "r");
    char str[MAX_STR];
    int swno = 0;
    while (fgets(str, MAX_STR, fp)) {
        int n = 0;
        char * result;
        result = strtok(str, ";");
        strcpy(mySwInfo.sw_param[swno].swArgv[n], result);
        strcpy(mySwInfo.sw_param[swno].swArgv[n], trim(mySwInfo.sw_param[swno].swArgv[n]));
        while (result != NULL) {
            n++;
            char * result;
            result = strtok(NULL, ";");
            if (result != NULL) {
                strcpy(mySwInfo.sw_param[swno].swArgv[n], result);
                strcpy(mySwInfo.sw_param[swno].swArgv[n], trim(mySwInfo.sw_param[swno].swArgv[n]));
            } else
                break;
        }
        mySwInfo.noOfArgv = swno;
        swno++;
    }
    fclose(fp);
}

char * gettime() {
  struct timeval tv;
  gettimeofday( & tv, NULL);
  time_t t = (time_t) tv.tv_sec;
  struct tm * ptm = localtime( & t);
  static char str[1024];
  sprintf(str, "%04d.%02d.%02d %02d:%02d:%02d.%03d ",
    ptm -> tm_year + 1900, ptm -> tm_mon + 1, ptm -> tm_mday,
    ptm -> tm_hour, ptm -> tm_min, ptm -> tm_sec, (int) tv.tv_usec / 1000);
  return str;
}

void restartSwBlock(int index) {
    pid_t pid;
    pid = fork();
    if (pid > 0) {
        mySwInfo.pids[index] = pid;
        strcpy(mySwInfo.sw_info[index].start_time, gettime());
    } else if (pid == 0) {
        char path[30] = "./";
        strcat(path, mySwInfo.sw_param[index].swArgv[0]);
        execl(path, mySwInfo.sw_param[index].swArgv[0],
            mySwInfo.sw_param[index].swArgv[1],
            mySwInfo.sw_param[index].swArgv[2],
            mySwInfo.sw_param[index].swArgv[3], NULL);
    }
}

int findIndex() {
    for (int i = 0; i <= mySwInfo.noOfArgv; i++) {
        if (mySwInfo.deadPid == mySwInfo.pids[i]) {
            return i;
        }
    }
    return -1;
}

void logInfo(swInfo * list) {
    mkdir("./log", 0755);
    chdir("./log");
    FILE * fp;
    fp = fopen("restart.txt", "a");
    fprintf(fp, "Name: %s\tStart Time: %s\tReason: %s\tRestart Count: %s\n",
        list -> name, list -> start_time, list -> reason, list -> restart_count);
    fclose(fp);
    chdir("../");
}

void showInfo() {
    char param[10];
    printf(" _______________________________________________________________________________________________________ \n");
    printf("|       Name       | Restart count |        Start time       |                  Reason                  |\n");
    for (int i = 0; i <= mySwInfo.noOfArgv; i++) {
        printf("| %16s |", mySwInfo.sw_info[i].name);
        printf(" %13s |", mySwInfo.sw_info[i].restart_count);
        printf(" %s|", mySwInfo.sw_info[i].start_time);
        printf(" %40s |\n", mySwInfo.sw_info[i].reason);
    }
    
    printf("|__________________|_______________|_________________________|__________________________________________|\n");
}

void initSwStatus() {
    mySwInfo.noOfArgv = 0;
    mySwInfo.deadPid = 0;
    for (int i = 0; i < 11; i++) {
        mySwInfo.pids[i] = 0;
        mySwInfo.sw_info[i].name = (char * ) malloc(sizeof(char) * 20);
        mySwInfo.sw_info[i].reason = (char * ) malloc(sizeof(char) * 50);
        mySwInfo.sw_info[i].restart_count = (char * ) malloc(sizeof(char) * 20);
        mySwInfo.sw_info[i].restart = 0;
        mySwInfo.sw_info[i].start_time = (char * ) malloc(sizeof(char) * 30);
        mySwInfo.sw_param[i].swArgv[0] = (char * ) malloc(sizeof(char) * 20);
        mySwInfo.sw_param[i].swArgv[1] = (char * ) malloc(sizeof(char) * 20);
        mySwInfo.sw_param[i].swArgv[2] = (char * ) malloc(sizeof(char) * 20);
        mySwInfo.sw_param[i].swArgv[3] = (char * ) malloc(sizeof(char) * 20);
    }
}

void initSwBlock() {
    pid_t pid;
    for (int i = 0; i <= mySwInfo.noOfArgv; i++) {
        strcpy(mySwInfo.sw_info[i].name, mySwInfo.sw_param[i].swArgv[0]);
        sprintf(mySwInfo.sw_info[i].reason, "Init.");
        sprintf(mySwInfo.sw_info[i].restart_count, "%d", 0);
        strcpy(mySwInfo.sw_info[i].start_time, gettime());
        logInfo( & (mySwInfo.sw_info[i]));
        showInfo();
        pid = fork();
        if (pid > 0) {
            mySwInfo.pids[i] = pid;
        } else if (pid == 0) {
            char path[30] = "./";
            strcat(path, mySwInfo.sw_param[i].swArgv[0]);
            execl(path, mySwInfo.sw_param[i].swArgv[0],
                mySwInfo.sw_param[i].swArgv[1],
                mySwInfo.sw_param[i].swArgv[2],
                mySwInfo.sw_param[i].swArgv[3], NULL);
        }
    }
}

int main() {
    int status;
    char exit_code[50];
    initSwStatus();
    readFileList();
    initSwBlock();
    while (1) {
        mySwInfo.deadPid = waitpid(-1, & status, 0);
        if (mySwInfo.deadPid != -1) {
            int index = findIndex();
            mySwInfo.sw_info[index].restart++;
            sprintf(mySwInfo.sw_info[index].restart_count, "%d", mySwInfo.sw_info[index].restart);
            if (WIFEXITED(status)) {
                sprintf(exit_code, "Exit(%d)", WEXITSTATUS(status));
                strcpy(mySwInfo.sw_info[index].reason, exit_code);
            } else if (WIFSIGNALED(status)) {
                sprintf(exit_code, "Signal(%d, %s)", WTERMSIG(status), strsignal(WTERMSIG(status)));
                strcpy(mySwInfo.sw_info[index].reason, exit_code);
            }
            restartSwBlock(index);
            logInfo( & (mySwInfo.sw_info[index]));
            showInfo();
        }
    }
}
