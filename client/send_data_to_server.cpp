//
// Created by lhx on 12/13/21.
//

#include "head.h"
#include "function.h"
//与服务端连接的 sockfd
extern int sockfd; //TCP
extern int udp_sockfd;//UDP

extern int msg_id;//消息队列的id
//buffer
extern char mem_buf[100];
extern char cpu_buf[100];
extern char disk_buf[100];
extern char sys_buf[200];
//文件路经
extern char *mem_persistence;
extern char *cpu_persistence;
extern char *disk_persistence;
extern char *sys_persistence;


//执行脚本文件，将获取到的数据封装为monitor_msg_ds， 放入消息队列
void *check_data_and_send_to_msg_queue(void *arg) {
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

/*从消息队列中取出消息，将消息封装为Json对象，将Json对象序列化为字符串，发送给服务端*/
void *send_data_to_server(void *arg) {
    struct monitor_msg_ds m;
    while (1) {
        memset(&m, 0, sizeof(struct monitor_msg_ds));
        if (msgrcv(msg_id, &m, sizeof(struct monitor_msg_ds), 0, 0) < 0) {//从消息队列中取出消息
            perror("msgrcv");
            exit(1);
        }
        if (m.type == (long)SYS_MEM) {
            //检查数据可靠性
            if (strlen(m.buff) == 0) {
                DBG(RED"mem msg is empty!!\n" NONE);
                exit(1);
            } else {
                DBG(BLUE"<Mem res> : %s" NONE, mem_buf);
            }

            cJSON *mem_data;  //将数据构造成json对象
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            const char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "total_mem_value", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "left_mem_value", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_usage_rate", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "mem_prediction_rate", strtok(NULL, delim));
            str = cJSON_Print(mem_data);//将Json对象序列化为字符串， 发送给服务端
            struct monitor_msg_ds msg;
            msg.type = SYS_MEM;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {//
                int per_fd = open_file(mem_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write mem persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                close(per_fd);
                //向服务端发送数据失败时，
                try_to_relogin();
            }
        } else if (m.type == (long)SYS_CPU) {
            if (strlen(m.buff) == 0) {
                DBG(RED"cpu msg is empty!!\n" NONE);
                exit(1);
            } else {
                DBG(BLUE"<CPU res> : %s" NONE, cpu_buf);
            }
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            const char *delim = " \n";
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
            struct monitor_msg_ds msg;
            msg.type = SYS_CPU;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                int per_fd = open_file(cpu_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write cpu persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                close(per_fd);
            }
        }else if (m.type == (long)SYS_DISK) {
            if (strlen(m.buff) == 0) {
                DBG(RED"disk msg is empty!!\n" NONE);
                exit(1);
            } else {
                DBG(BLUE"<Disk res> : %s" NONE, disk_buf);
            }
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            const char *delim = " \n";
            strcpy(now_time,strtok(m.buff, delim));
            strcat(now_time, " ");
            strcat(now_time, strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "now_time", now_time);
            cJSON_AddStringToObject(mem_data, "disk_size", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "used_space", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "avail_space", strtok(NULL, delim));
            cJSON_AddStringToObject(mem_data, "utilization", strtok(NULL, delim));
            str = cJSON_Print(mem_data);
            struct monitor_msg_ds msg;
            msg.type = SYS_DISK;
            strcpy(msg.buff, str);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                int per_fd = open_file(disk_persistence);
                if (write(per_fd, str, strlen(str)) < 0) {
                    perror("write disk persistence file");
                    exit(1);
                }
                write(per_fd, "\n", 1);
                close(per_fd);
            }
        }else if (m.type == (long)SYS_SYS) {
            if (strlen(m.buff) == 0) {
                DBG(RED"sys msg is empty!!\n" NONE);
                exit(1);
            } else {
                DBG(BLUE"<SYS res> : %s" NONE, sys_buf);
            }
            cJSON *mem_data;
            mem_data = cJSON_CreateObject();
            char *str;
            char now_time[30] = {0};
            const char *delim = " \n";
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
                    int per_fd = open_file(sys_persistence);
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