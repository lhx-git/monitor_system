/*************************************************************************
	> File Name: client.c
	> Author:lhx 
	> Mail:207201827@qq.com 
	> Created Time: Thu 16 Sep 2021 08:54:02 PM CST
 ************************************************************************/

#include "head.h"

char server_ip[20];
int relogin_num = 3;
int server_port, msg_id, sockfd, per_fd, check_for_relogin = 3, udp_sockfd;
char token[20];
char *config = "/home/lhx/CProject/monitor_system/client/monitor.conf";
char *mem_persistence = "/home/lhx/CProject/monitor_system/client/mem_persistence";
char *cpu_persistence = "/home/lhx/CProject/monitor_system/client/cpu_persistence";
char *disk_persistence = "/home/lhx/CProject/monitor_system/client/disk_persistence";
char *sys_persistence = "/home/lhx/CProject/monitor_system/client/sys_persistence";
char cpu_buf[100];
char mem_buf[100];
char disk_buf[100];
char sys_buf[200];
int mem_lines = 7;
int cpu_lines = 9;
int disk_lines = 7;
int sys_lines = 17;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char **argv) {
    pthread_t rid, mid, hid;
    //创建消息队列
    key_t key = ftok("/home/lhx/CProject/monitor_system/client", 100);
    if ((msg_id = msgget(key, IPC_CREAT)) < 0) {
        perror("msgget");
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "s:p:t:")) != -1) {
        switch(opt) {
            case 's':
                strcpy(server_ip, optarg);
                break;
            case 't':
                strcpy(token, optarg);
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage : %s -p port -s server_ip\n", argv[0]);
                exit(1);
        }
    }
    
    if (!server_port) server_port = atoi(get_conf_value(config, "SERVER_PORT"));
    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(config, "SERVER_IP"));
    if (!strlen(token)) strcpy(token, get_conf_value(config, "TOKEN"));

    struct login_request loginRequest;
    strcpy(loginRequest.token, token);

    if ((udp_sockfd = socket_connect_udp(server_ip, server_port, &loginRequest)) < 0) {
        perror("socket_connect_udp");
        exit(1);
    }

    //tcp connect
    if ((sockfd = socket_connect(server_ip, server_port)) < 0) {
        perror("socket_connect");
        exit(1);
    }
    //send
    char buff[500] = {0};
    if (send(sockfd, token, strlen(token), 0) < 0) {
        close(sockfd);
        perror("send token");
        exit(1);
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
        fprintf(stderr, RED"Server donot response for token!\n"NONE);
        close(sockfd);
        exit(1);
    }
    int ack = 0;
    if (recv(sockfd, &ack, sizeof(ack), 0) < 0) {
        close(sockfd);
        perror("recv ack");
        exit(1);
    }
    if (ack != 1) {
        fprintf(stderr, "Server Response Error!\n"NONE);
        exit(1);
    }



    //新建一个检测线程，用以进行系统资源的获取
    pthread_create(&rid, NULL, do_work, NULL);
    pthread_create(&mid, NULL, do_msg_queue, NULL);
    pthread_create(&hid, NULL, heart_beat_from_client, NULL);


    while (1) {
        struct monitor_msg_ds msg;
        bzero(&msg, sizeof(msg));
        recv(sockfd, (void *)&msg, sizeof(msg), 0);
        if (msg.type &PI_HEART) {
            DBG(BLUE"<x>\n"NONE);
            msg.type = PI_ACK;
            send(sockfd, (void *)&msg, sizeof(msg), 0);
        }
    }
    return 0;
}

