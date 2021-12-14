//
// Created by lhx on 12/13/21.
//
#include "head.h"
#include "function.h"
char cpu_buf[100];
char mem_buf[100];
char disk_buf[100];
char sys_buf[200];

extern int sockfd;

void check_for_mem() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/Memlog.sh 50", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    memset(mem_buf, 0, 100);
    int n;
    if ((n = fread(mem_buf, 1, 100, fp)) <= 0) {
        //todo 这里有时候会出现bug
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
    //try_to_relogin();
}

void check_for_cpu() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/CPUlog.sh", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    memset(cpu_buf, 0, 100);
    int n;
    if ((n = fread(cpu_buf, 1, 100, fp)) <= 0) {
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
}

void check_for_disk() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/Disklog.sh", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    memset(disk_buf, 0, 100);
    int n;
    if ((n = fread(disk_buf, 1, 100, fp)) <= 0) {
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
}
void check_for_sys() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/Syslog.sh", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    memset(sys_buf, 0, 200);
    int n;
    if ((n = fread(sys_buf, 1, 200, fp)) <= 0) {
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
}