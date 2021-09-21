//
// Created by lhx on 9/18/21.
//

#include "head.h"


extern char mem_buf[100];
extern int msg_id, sockfd, per_fd;
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
                DBG(BLUE"%ld\n", strlen(str));
                DBG(BLUE"%ld\n", sizeof(str));
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write mem persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
            }
            //如果发送不成功
            //数据持久化->文件
        }
    }
}