#include "head.h"

#define MAXEVENTS 5
extern char token[100];
extern struct client_ds* clients;
extern int epollfd, max, cur_max;

//todo finish theadpool

void *work_on_reactor(void *arg) {
    struct epoll_event ev, events[MAXEVENTS];
    for (;;) {
        int nfds = epoll_wait(epollfd, events, MAXEVENTS, -1); 
        if (nfds <= 0) {
            perror("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < nfds; i++) {
            struct monitor_msg_ds msg;
            bzero(&msg, sizeof(msg));
            int sockfd = events[i].data.fd;
            int ret = recv(events[i].data.fd, (void *)&msg, sizeof(msg), 0);
            if (ret <= 0) {
                epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(sockfd);
                continue;
            }
            if (msg.type & PI_ACK) {
                DBG(GREEN"<ACK> ack received from %s!\n", inet_ntoa(clients[sockfd].addr.sin_addr));
                //同时有两个线程在写isonline
                clients[sockfd].isonline = 5;
            } else if (msg.type & SYS_MEM) {
                //解析cJSON数据
                cJSON *mem_data;
                mem_data = cJSON_Parse(msg.buff);
                cJSON *now_time = NULL;
                cJSON *total_mem_value = NULL;
                cJSON *left_mem_value = NULL;
                cJSON *mem_usage_rate = NULL;
                cJSON *mem_prediction_rate = NULL;
                now_time = cJSON_GetObjectItem(mem_data, "now_time");
                total_mem_value = cJSON_GetObjectItem(mem_data, "total_mem_value");
                left_mem_value = cJSON_GetObjectItem(mem_data, "left_mem_value");
                mem_usage_rate = cJSON_GetObjectItem(mem_data, "mem_usage_rate");
                mem_prediction_rate = cJSON_GetObjectItem(mem_data, "mem_prediction_rate");
                DBG(BLUE"%s\n", now_time->valuestring);
                DBG(BLUE"%s\n", total_mem_value->valuestring);
                DBG(BLUE"%s\n", left_mem_value->valuestring);
                DBG(BLUE"%s\n", mem_usage_rate->valuestring);
                DBG(BLUE"%s\n", mem_prediction_rate->valuestring);
                //write to db
            }
        }
    }
}

//给所有的客戶都發送一個心跳包
//如果一个客户连续5次都没有回复，那么认为这个客户掉线了，将它从客户列表中移除，并关闭与之通信的sockfd
void heart_beat(int signum) {
    struct monitor_msg_ds msg;
    msg.type = PI_HEART;

    for (int i = 3; i <= cur_max; i++) {
        if (clients[i].isonline > 0) {
            send(clients[i].sockfd, (void *)&msg, sizeof(msg), 0);
            clients[i].isonline--;
            if (clients[i].isonline <= 0) {
                epoll_ctl(epollfd, EPOLL_CTL_DEL, clients[i].sockfd, NULL);
                close(clients[i].sockfd);
                memset(&clients[i], 0, sizeof(struct client_ds));
                DBG(PINK"<HeartFaild> : "NONE" %s is removed from clients list!\n", inet_ntoa(clients[i].addr.sin_addr));

            }
        } 
    }
}

int check_online(struct sockaddr_in *addr) {
    int flag = 0;
    for (int i = 3; i <= cur_max; i++) {
        if (addr->sin_addr.s_addr == clients[i].addr.sin_addr.s_addr) {
            flag = 1;
        }    
    }
    return flag;
}
    
void* do_login(void *arg) {
    int server_listen, sockfd;
    server_listen = *(int *)arg;
    while (1) {
        struct sockaddr_in client;
        int len = sizeof(client);
        sockfd = accept(server_listen,(struct sockaddr *)&client, &len);
        if (sockfd < 0) {
            perror("accept");
            exit(1);
        }
        cur_max = max(cur_max, sockfd);
        char token_tmp[100] = {0};
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        struct timeval tv;
        tv.tv_sec =  0;
        tv.tv_usec = 10000;
        //=0表示时间到了，一个事件也没有 <0表示出错
        if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
            close(sockfd);
            DBG(RED"<Error> : "NONE"time out for token!\n");
            continue;
        }
        if (recv(sockfd, token_tmp, sizeof(token_tmp), 0) <= 0) {
            close(sockfd);
            continue;
        }
        int ack = 1;
        //判断是否已经在线
        if (check_online(&client)) {
            ack = 0;
        }
        if (send(sockfd, (void *)&ack, sizeof(int) , 0) < 0) {
            close(sockfd);
            DBG(RED"<Error> : "NONE"send ack error!\n");
            continue;
        }
        if (ack == 0) {
            close(sockfd);
            DBG(YELLOW"<Warning> : "NONE"%s already login!\n", inet_ntoa(client.sin_addr));
            continue;
        }
        if (strcmp(token_tmp, token)) {
            close(sockfd);
            DBG(RED"<Error> : "NONE"TOKEN Error\n");
            continue;
        }
        //sockfd就是客户
        clients[sockfd].sockfd = sockfd;
        clients[sockfd].isonline = 5;
        clients[sockfd].addr = client;

        //注册到反应堆中去
        struct epoll_event ev;
        ev.data.fd = sockfd;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
        DBG(GREEN"<Reactor> : "NONE"add client to reactor!\n");
    }
}
