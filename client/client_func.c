//
// Created by lhx on 9/18/21.
//

#include "head.h"


extern char mem_buf[100];
extern int msg_id, sockfd, per_fd, server_port, check_for_relogin, relogin_num;
extern char *mem_persistence;
extern pthread_cond_t cond;
extern pthread_mutex_t mutex;
char server_ip[20], token[100];


void check_for_mem() {
    FILE *fp = NULL;
    if ((fp = popen("bash /home/lhx/CProject/monitor_system/script/Memlog.sh 50", "r")) == NULL) {
        fprintf(stderr, "popen Error\n");
        exit(1);
    }
    memset(mem_buf, 0, sizeof(mem_buf));
    int n;
    if ((n = fread(mem_buf, 1, 100, fp)) <= 0) {
        fprintf(stderr, "fread Error\n");
        exit(1);
    }
    pclose(fp);
    //由于两个线程可能同时修改check_for_relogin，所以用mutex
    pthread_mutex_lock(&mutex);
    DBG(GREEN"<check_for_relogin> : %d\n", check_for_relogin);
    if (check_for_relogin == 0) {
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
        return ;
    } else {
        check_for_relogin--;
    }
    pthread_mutex_unlock(&mutex);
}

void *do_work(void *arg) {
    while (1) {
        check_for_mem();
        sleep(5);
        struct monitor_msg_ds msg;
        msg.type = SYS_MEM;
        strcpy(msg.buff, mem_buf);
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
            strcpy(now_time,strtok(mem_buf, delim));
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
                //perror("send mem message");
                DBG(RED"sockfd = %d\n", sockfd);
                per_fd = open_file(mem_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write mem persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                DBG(RED"per_fd = %d", per_fd);
                close(per_fd);
            }
        }
    }
}
//todo 条件变量使用可能有问题，只在第一次的时候触发了。之后就算满足条件，check_for_relogin=0，也不会执行relogin
void *heart_beat_from_client(void *arg) {
    pthread_mutex_lock(&mutex);
    //当check_for_relogin > 0时，该线程挂起。
    while (check_for_relogin > 0) {//循环作用是当check_for_relogin！=0 时 线程意外被唤醒
        pthread_cond_wait(&cond, &mutex);
    }
    DBG(RED"relogin\n");
    check_for_relogin = relogin_num;
    sockfd++;
    sleep(7);
    if (relogin() > 0) {
        do_with_file(mem_persistence);
    }
    pthread_mutex_unlock(&mutex);
}

int relogin() {
    //如果原来的sockfd是可以发送数据的，说明与服务器仍保持链接。不需要relogin
    if ((sockfd = socket_connect(server_ip, server_port)) < 0) {
        perror("socket_connect");
        exit(1);
    }
    //send
    char buff[500] = {0};
    DBG(RED"TOKEN = %s", token);
    DBG(RED"new sockfd = %d", sockfd);
    if (send(sockfd, token, strlen(token), 0) < 0) {
        close(sockfd);
        perror("send token");
        return 0;
    }

    //防止无限等待
    /*struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
        fprintf(stderr, RED"Server donot response for token!\n"NONE);
        close(sockfd);
        return 0;
    }*/
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


void do_with_file(char *filename) {
    //读文件，将文件内从从新打包成消息重新发送，最后清空文件。
    //找了半天的bug！！！！！
    // 由于缓存temp设置太小，导致出现stack smashing detected错误。
    char a[500];
    FILE *fp = fopen(filename, "r");
    char temp[500];
    int mem_lines = 7;
    while (fgets(a, 500, fp) != NULL) {
        mem_lines--;
        strncat(temp, a,500);
        if (mem_lines == 0) {
            mem_lines = 7;
            struct monitor_msg_ds msg;
            msg.type = SYS_MEM;
            strncpy(msg.buff, temp, 500);
            DBG(RED"%s\n", temp);
            DBG(GREEN"%ld\n", strlen(temp));
            memset(temp, 0, 500);
            DBG(YELLOW"%s\n", temp);
            DBG(YELLOW"%ld\n", strlen(temp));
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                perror("send mem message");
                /*per_fd = open_file(mem_persistence);
                if (write(per_fd, temp, strlen(temp)) < 0) {
                    perror("write mem persistence file");
                    exit(1);
                }
                close(per_fd);*/
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


//todo文件并没有被清空
void empty_file(char *filename) {
    int fd;
    //以“w“的方式打开文件相当与清空文件。
    if ((fd = open(filename, O_WRONLY)) < 0) {
        perror("empty file");
        exit(1);
    }
    close(fd);
}