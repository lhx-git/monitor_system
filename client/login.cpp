//
// Created by lhx on 12/13/21.
//
#include "head.h"
#include "function.h"
extern char server_ip[20], token[TOKEN_LEN];
extern int server_port, sockfd;
int relogin_num = 3;//发生3次 数据发送失败问题时，触发重新连接
int check_for_relogin = relogin_num;//当check_for_relogin = 0时，重新连接

//表示数据经过JSON序列化后的，在文件中占多少行
int mem_lines = 7;
int cpu_lines = 9;
int disk_lines = 7;
int sys_lines = 17;

//文件路经
extern char *mem_persistence;
extern char *cpu_persistence;
extern char *disk_persistence;
extern char *sys_persistence;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;



//与服务器建立tcp链接，并发送token进行验证
int login() {
    if ((sockfd = socket_connect(server_ip, server_port)) < 0) {
        perror("socket_connect");
        exit(1);
    }

    if (send(sockfd, token, strlen(token), 0) < 0) {//发送token
        close(sockfd);
        perror("send token");
        return 0;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {   //防止无限等待，使用select完成计时器功能
        fprintf(stderr, RED"Server donot response for token!\n" NONE);
        close(sockfd);
        return 0;
    }

    int ack = 0;
    if (recv(sockfd, &ack, sizeof(ack), 0) < 0) {//接收服务器的响应
        close(sockfd);
        perror("recv ack");
        return 0;
    }
    if (ack != 1) {
        fprintf(stderr, "Server Response Error!\n" NONE);
        return 0;
    }
    return 1;
}

void *relogin(void *arg) {
    while (1) {
        DBG(YELLOW"relogin thread was arise\n");
        pthread_mutex_lock(&mutex);
        while (check_for_relogin > 0) {   //当check_for_relogin > 0时，该线程挂起。
            pthread_cond_wait(&cond, &mutex);
        }
        DBG(RED"relogin\n");
        check_for_relogin = relogin_num;
        //close(sockfd);
        //sleep(7);//close(sockfd)后需要等几秒，否则会出现broken pipe错误
        if (login() > 0) {  //断线重连
            do_with_file(SYS_MEM, mem_lines, mem_persistence, sockfd);
            do_with_file(SYS_CPU, cpu_lines, cpu_persistence, sockfd);
            do_with_file(SYS_DISK, disk_lines, disk_persistence, sockfd);
            do_with_file(SYS_SYS, sys_lines, sys_persistence, sockfd);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void try_to_relogin() {
    pthread_mutex_lock(&mutex);
    DBG(RED"<check_for_relogin> : %d\n" NONE, check_for_relogin);
    if (check_for_relogin == 0) {  //当check_for_relogin == 0， 唤醒relogin线程，进行重连
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    } else {
        check_for_relogin--;
        pthread_mutex_unlock(&mutex);
    }
    return ;
}

