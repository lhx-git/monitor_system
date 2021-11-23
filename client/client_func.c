//
// Created by lhx on 9/18/21.
//

#include "head.h"


extern char mem_buf[100];
extern char cpu_buf[100];
extern char disk_buf[100];
extern char sys_buf[200];
extern int msg_id, sockfd, per_fd, server_port, check_for_relogin, relogin_num, udp_sockfd, udp_server_port;
extern char *mem_persistence;
extern char *cpu_persistence;
extern char *disk_persistence;
extern char *sys_persistence;
extern int mem_lines;
extern int cpu_lines;
extern int disk_lines;
extern int sys_lines;
extern pthread_cond_t cond;
extern pthread_mutex_t mutex;
char server_ip[20], token[100];


void try_to_relogin() {
    pthread_mutex_lock(&mutex);
    DBG(RED"<check_for_relogin> : %d\n"NONE, check_for_relogin);
    if (check_for_relogin == 0) {
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    } else {
        check_for_relogin--;
        pthread_mutex_unlock(&mutex);
    }
    return ;
}

void check_for_mem() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/Memlog.sh 50", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    //todo
    memset(mem_buf, 0, 100);
    int n;
    if ((n = fread(mem_buf, 1, 100, fp)) <= 0) {
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
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




void *do_work(void *arg) {
    while (1) {
        check_for_mem();
        check_for_disk();
        check_for_cpu();
        check_for_sys();
        sleep(5);
        struct monitor_msg_ds msg;
        msg.type = SYS_MEM;
        strcpy(msg.buff, mem_buf);
        if (msgsnd(msg_id, (void *)&msg, sizeof(msg), 0) < 0) {
            perror("msgsnd");
            exit(1);
        };
        msg.type = SYS_CPU;
        strcpy(msg.buff, cpu_buf);
        if (msgsnd(msg_id, (void *)&msg, sizeof(msg), 0) < 0) {
            perror("msgsnd");
            exit(1);
        };
        msg.type = SYS_DISK;
        strcpy(msg.buff, disk_buf);
        if (msgsnd(msg_id, (void *)&msg, sizeof(msg), 0) < 0) {
            perror("msgsnd");
            exit(1);
        };
        msg.type = SYS_SYS;
        strcpy(msg.buff, sys_buf);
        if (msgsnd(msg_id, (void *)&msg, sizeof(msg), 0) < 0) {
            perror("msgsnd");
            exit(1);
        };
    }
}

void *do_msg_queue(void *arg) {
    struct monitor_msg_ds m;
    while (1) {
        memset(&m, 0, sizeof(struct monitor_msg_ds));
        if (msgrcv(msg_id, &m, sizeof(struct monitor_msg_ds), 0, 0) < 0) {
            perror("msgrcv");
            exit(1);
        }
        if (m.type == (long)SYS_MEM) {
            //检查数据可靠性
            if (strlen(m.buff) == 0) {
                DBG(RED"mem msg is empty!!\n"NONE);
                exit(1);
            } else {
                DBG(BLUE"<Mem res> : %s\n"NONE, mem_buf);
            }
            //将数据构造成json对象
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "total_mem_value", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "left_mem_value", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_usage_rate", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_prediction_rate", strtok(NULL, delim));
            str = cJSON_Print(mem_data);
            //DBG(BLUE"%s", str);
            struct monitor_msg_ds msg;
            msg.type = SYS_MEM;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                //DBG(RED"sockfd = %d\n", sockfd);
                per_fd = open_file(mem_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write mem persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                //DBG(RED"per_fd = %d", per_fd);
                close(per_fd);
                try_to_relogin();
            }
        } else if (m.type == (long)SYS_CPU) {
            //检查数据可靠性
            if (strlen(m.buff) == 0) {
                DBG(RED"cpu msg is empty!!\n"NONE);
                exit(1);
            } else {
                DBG(BLUE"<CPU res> : %s\n"NONE, cpu_buf);
            }
            //将数据构造成json对象
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "load_avg_1", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "load_avg_2", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "load_avg_3", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "utilization", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "temperature", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "warning", strtok(NULL, delim));
            str = cJSON_Print(mem_data);
            //DBG(BLUE"%s", str);
            struct monitor_msg_ds msg;
            msg.type = SYS_CPU;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                per_fd = open_file(cpu_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write cpu persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                close(per_fd);
            }
        }else if (m.type == (long)SYS_DISK) {
            //检查数据可靠性
            if (strlen(m.buff) == 0) {
                DBG(RED"disk msg is empty!!\n"NONE);
                exit(1);
            } else {
                DBG(BLUE"<Disk res> : %s\n"NONE, disk_buf);
            }
            //将数据构造成json对象
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "disk_size", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "used_space", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "avail_space", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "utilization", strtok(NULL, delim));
            str = cJSON_Print(mem_data);
            //DBG(BLUE"%s", str);
            struct monitor_msg_ds msg;
            msg.type = SYS_DISK;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                per_fd = open_file(disk_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write disk persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                close(per_fd);
            }
        }else if (m.type == (long)SYS_SYS) {
            //检查数据可靠性
            if (strlen(m.buff) == 0) {
                DBG(RED"sys msg is empty!!\n"NONE);
                exit(1);
            } else {
                DBG(BLUE"<SYS res> : %s\n"NONE, sys_buf);
            }
            //将数据构造成json对象
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "hostname", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "os_version", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "kernel_version", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "running_time", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "load_avg_1", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "load_avg_2", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "load_avg_3", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "disk_size", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "disk_utilization", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_size", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_utilization", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "disk_stat", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_stat", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "cpu_stat", strtok(NULL, delim));
            str = cJSON_Print(mem_data);
            //DBG(BLUE"%s", str);
            struct monitor_msg_ds msg;
            msg.type = SYS_SYS;
            strcpy(msg.buff, str);
            cJSON *disk_stat = cJSON_GetObjectItem(mem_data, "disk_stat");
            cJSON *mem_stat = cJSON_GetObjectItem(mem_data, "mem_stat");
            cJSON *cpu_stat = cJSON_GetObjectItem(mem_data, "cpu_stat");
            //紧急数据，udp发送
            if (strcmp(disk_stat->valuestring,"warning") == 0 || strcmp(mem_stat->valuestring,"warning") == 0 || strcmp(cpu_stat->valuestring,"warning") == 0) {
                send(udp_sockfd, (void *)&msg, sizeof(msg), 0);
            } else {
                if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                    per_fd = open_file(sys_persistence);
                    if (write(per_fd, str, strlen(str)) < 0) {
                        perror("write sys persistence file");
                        exit(1);
                    }
                    write(per_fd, "\n", 1);
                    close(per_fd);
                }
            }
        }
    }
}


void *heart_beat_from_client(void *arg) {
    //!!没有使用while循环，当然之触发一次了！！！！
    while (1) {
        DBG(YELLOW"heart_beat_from_client thread was arise\n");
        pthread_mutex_lock(&mutex);
        //当check_for_relogin > 0时，该线程挂起。
        while (check_for_relogin > 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        DBG(RED"relogin\n");
        check_for_relogin = relogin_num;
        //手动让客户端短线
        //sockfd++;
        //sleep(7);
        //断线重连
        if (relogin() > 0) {
            do_with_file(SYS_MEM, mem_lines, mem_persistence);
            do_with_file(SYS_CPU, cpu_lines, cpu_persistence);
            do_with_file(SYS_DISK, disk_lines, disk_persistence);
            do_with_file(SYS_SYS, sys_lines, sys_persistence);
        }
        pthread_mutex_unlock(&mutex);
    }

}

int relogin() {
    if ((sockfd = socket_connect(server_ip, server_port)) < 0) {
        perror("socket_connect");
        exit(1);
    }
    //send
    DBG(RED"TOKEN = %s\n", token);
    DBG(RED"new sockfd = %d\n", sockfd);
    if (send(sockfd, token, strlen(token), 0) < 0) {
        close(sockfd);
        perror("send token");
        return 0;
    }

    //防止无限等待
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
        fprintf(stderr, RED"Server donot response for token!\n"NONE);
        close(sockfd);
        return 0;
    }
    int ack = 0;
    if (recv(sockfd, &ack, sizeof(ack), 0) < 0) {
        close(sockfd);
        perror("recv ack");
        return 0;
    }
    if (ack != 1) {
        fprintf(stderr, "Server Response Error!\n"NONE);
        return 0;
    }
    return 1;
}

//读文件，将文件内从从新打包成消息重新发送，最后清空文件。
//todo 代码写的有瑕疵，如果写了新的脚本获取系统数据，这里需要更爱
void do_with_file(int msg_type, int msg_lines, char *filename) {
    //找了半天的bug！！！！！
    // 由于缓存temp设置太小，导致出现stack smashing detected错误。
    char a[MSG_BUFF_SIZE];
    FILE *fp = fopen(filename, "r");
    char temp[MSG_BUFF_SIZE];

    int lines = msg_lines;
    while (fgets(a, MSG_BUFF_SIZE, fp) != NULL) {
        lines--;
        strncat(temp, a,MSG_BUFF_SIZE);
        if (lines == 0) {
            lines = msg_lines;
            struct monitor_msg_ds msg;
            msg.type = msg_type;
            strncpy(msg.buff, temp, MSG_BUFF_SIZE);
            //如果msg内是空的，就不发送给服务器了
            if (strlen(msg.buff) == 0) break;
            memset(temp, 0, MSG_BUFF_SIZE);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                perror("send mem message");
            }
        }
    }
    fclose(fp);
    //读完文件后，将文件清空。
    empty_file(mem_persistence);
}

int open_file(char *filename) {
    int fd;
    if ((fd = open(filename, O_CREAT | O_RDWR | O_APPEND, 0666)) < 0) {
        perror("persistence file open");
        exit(1);
    }
    return fd;
}

void empty_file(char *filename) {
    //以“w“的方式打开文件相当与清空文件。
    //不能使用系统调用open（w），不会清空文件。
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
        perror("empty file");
        exit(1);
    }
    fclose(fp);
}