/*************************************************************************
	> File Name: client.c
	> Author:lhx 
	> Mail:207201827@qq.com 
	> Created Time: Thu 16 Sep 2021 08:54:02 PM CST
 ************************************************************************/

#include "head.h"
#include "common.h"
#include "function.h"

char server_ip[20];
char token[20];

int server_port, msg_id, sockfd, udp_sockfd;
//文件路经
const char *config = "/home/lhx/CProject/monitor_system/client/monitor.conf";
const char *mem_persistence = "/home/lhx/CProject/monitor_system/client/persistence_files/mem_persistence";
const char *cpu_persistence = "/home/lhx/CProject/monitor_system/client/persistence_files/cpu_persistence";
const char *disk_persistence = "/home/lhx/CProject/monitor_system/client/persistence_files/disk_persistence";
const char *sys_persistence = "/home/lhx/CProject/monitor_system/client/persistence_files/sys_persistence";



int main(int argc, char **argv) {
    pthread_t rid, mid, hid;
    /*------------------------创建消息队列-----------------------*/
    key_t key = ftok("/home/lhx/CProject/monitor_system/client", 100);
    if ((msg_id = msgget(key, IPC_CREAT)) < 0) {
        perror("msgget");
        exit(1);
    }
    /*------------------------解析命令行参数-----------------------*/
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
    /*------------------------读取配置文件-----------------------*/
    if (!server_port) server_port = atoi(get_conf_value(config, "SERVER_PORT"));
    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(config, "SERVER_IP"));
    if (!strlen(token)) strcpy(token, get_conf_value(config, "TOKEN"));

    /*------------------------连接服务器-----------------------*/
    //创建udp_socket
    if ((udp_sockfd = socket_connect_udp(server_ip, server_port, (const char *) &token)) < 0) {
        perror("socket_connect_udp");
        exit(1);
    }
    //tcp连接服务端，并使用登录
    login();

    /*------------------------线程创建-----------------------*/
    //检测线程，用以进行系统资源的获取
    pthread_create(&rid, NULL, check_data_and_send_to_msg_queue, NULL);
    //建立一个线程，用以向服务端发送数据
    pthread_create(&mid, NULL, send_data_to_server, NULL);
    //建立一个线程，负责断线重连
    pthread_create(&hid, NULL, relogin, NULL);


    /*------------------------心跳机制-----------------------*/
    while (1) {
        struct monitor_msg_ds msg;
        bzero(&msg, sizeof(msg));
        recv(sockfd, (void *)&msg, sizeof(msg), 0);
        if (msg.type &PI_HEART) {
            DBG(BLUE"<x>\n" NONE);
            msg.type = PI_ACK;
            send(sockfd, (void *)&msg, sizeof(msg), 0);
        }
    }
    return 0;
}

