#include "head.h"
#include "function.h"
#include  <map>
#define MAXEVENTS 5
extern char token[TOKEN_LEN];
extern std::map<int ,struct client_ds *> clients;
extern int epollfd, server_listen, udp_epollfd, port;
extern SQL_CONN_POOL *sq_conn_poll;


void deal_with_monitor_msg(struct monitor_msg_ds * msg, MYSQL *mysql) {
    if (msg->type == SYS_MEM) {
        cJSON *mem_data;
        mem_data = cJSON_Parse(msg->buff);
        cJSON *now_time = cJSON_GetObjectItem(mem_data, "now_time");
        cJSON *total_mem_value = cJSON_GetObjectItem(mem_data, "total_mem_value");
        cJSON *left_mem_value = cJSON_GetObjectItem(mem_data, "left_mem_value");
        cJSON *mem_usage_rate = cJSON_GetObjectItem(mem_data, "mem_usage_rate");
        cJSON *mem_prediction_rate = cJSON_GetObjectItem(mem_data, "mem_prediction_rate");
        char sql[500] = {0};
        strcat(sql, "INSERT INTO monitor_mem_data (now_time, total_mem_value, left_mem_value, mem_usage_rate, mem_prediction_rate) VALUES ('");
        strcat(sql, now_time->valuestring); strcat(sql, "', '");
        strcat(sql, total_mem_value->valuestring); strcat(sql, "', '");
        strcat(sql, left_mem_value->valuestring); strcat(sql, "', '");
        strcat(sql, mem_usage_rate->valuestring); strcat(sql, "', '");
        strcat(sql, mem_prediction_rate->valuestring); strcat(sql, "')");
        DBG(YELLOW"%s\n", sql);
        if (mysql_real_query(mysql, sql, strlen(sql))) {
            //
            DBG(RED"error in mysql_real_query mem\n");
        }
    } else if (msg->type == SYS_CPU) {
        cJSON *mem_data;
        mem_data = cJSON_Parse(msg->buff);
        cJSON *now_time = cJSON_GetObjectItem(mem_data, "now_time");
        cJSON *load_avg_1 = cJSON_GetObjectItem(mem_data, "load_avg_1");
        cJSON *load_avg_2 = cJSON_GetObjectItem(mem_data, "load_avg_2");
        cJSON *load_avg_3 = cJSON_GetObjectItem(mem_data, "load_avg_3");
        cJSON *utilization = cJSON_GetObjectItem(mem_data, "utilization");
        cJSON *temperature = cJSON_GetObjectItem(mem_data, "temperature");
        cJSON *warning = cJSON_GetObjectItem(mem_data, "warning");
        char sql[500] = {0};
        strcat(sql, "INSERT INTO monitor_cpu_data (now_time, load_avg_1, load_avg_2, load_avg_3, utilization, temperature, warning) VALUES ('");
        strcat(sql, now_time->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_1->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_2->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_3->valuestring); strcat(sql, "', '");
        strcat(sql, utilization->valuestring); strcat(sql, "', '");
        strcat(sql, temperature->valuestring); strcat(sql, "', '");
        strcat(sql, warning->valuestring); strcat(sql, "')");
        DBG(YELLOW"%s\n", sql);
        if (mysql_real_query(mysql, sql, strlen(sql))) {
            DBG(RED"error in mysql_real_query cpu\n");
        }
    } else if (msg->type == SYS_DISK) {
        cJSON *mem_data;
        mem_data = cJSON_Parse(msg->buff);
        cJSON *now_time = cJSON_GetObjectItem(mem_data, "now_time");
        cJSON *disk_size = cJSON_GetObjectItem(mem_data, "disk_size");
        cJSON *used_space = cJSON_GetObjectItem(mem_data, "used_space");
        cJSON *avail_space = cJSON_GetObjectItem(mem_data, "avail_space");
        cJSON *utilization = cJSON_GetObjectItem(mem_data, "utilization");
        //生成sql语句
        char sql[500] = {0};
        strcat(sql, "INSERT INTO monitor_disk_data (now_time, disk_size, used_space, avail_space, utilization) VALUES ('");
        strcat(sql, now_time->valuestring); strcat(sql, "', '");
        strcat(sql, disk_size->valuestring); strcat(sql, "', '");
        strcat(sql, used_space->valuestring); strcat(sql, "', '");
        strcat(sql, avail_space->valuestring); strcat(sql, "', '");
        strcat(sql, utilization->valuestring); strcat(sql, "')");
        DBG(YELLOW"%s\n", sql);
        if (mysql_real_query(mysql, sql, strlen(sql))) {
            DBG(RED"error in mysql_real_query disk\n");
        }
    } else if (msg->type == SYS_SYS) {
        cJSON *mem_data;
        mem_data = cJSON_Parse(msg->buff);
        cJSON *now_time = cJSON_GetObjectItem(mem_data, "now_time");
        cJSON *hostname = cJSON_GetObjectItem(mem_data, "hostname");
        cJSON *os_version = cJSON_GetObjectItem(mem_data, "os_version");
        cJSON *kernel_version = cJSON_GetObjectItem(mem_data, "kernel_version");
        cJSON *running_time = cJSON_GetObjectItem(mem_data, "running_time");
        cJSON *load_avg_1 = cJSON_GetObjectItem(mem_data, "load_avg_1");
        cJSON *load_avg_2 = cJSON_GetObjectItem(mem_data, "load_avg_2");
        cJSON *load_avg_3 = cJSON_GetObjectItem(mem_data, "load_avg_3");
        cJSON *disk_size = cJSON_GetObjectItem(mem_data, "disk_size");
        cJSON *disk_utilization = cJSON_GetObjectItem(mem_data, "disk_utilization");
        cJSON *mem_size = cJSON_GetObjectItem(mem_data, "mem_size");
        cJSON *mem_utilization = cJSON_GetObjectItem(mem_data, "mem_utilization");
        cJSON *disk_stat = cJSON_GetObjectItem(mem_data, "disk_stat");
        cJSON *mem_stat = cJSON_GetObjectItem(mem_data, "mem_stat");
        cJSON *cpu_stat = cJSON_GetObjectItem(mem_data, "cpu_stat");
        char sql[500] = {0};
        //生成sql语句
        strcat(sql, "INSERT INTO monitor_sys_data (now_time, hostname, os_version, kernel_version, running_time, load_avg_1, load_avg_2, load_avg_3, disk_size, disk_utilization, mem_size, mem_utilization, disk_stat, mem_stat, cpu_stat) VALUES ('");
        strcat(sql, now_time->valuestring); strcat(sql, "', '");
        strcat(sql, hostname->valuestring); strcat(sql, "', '");
        strcat(sql, os_version->valuestring); strcat(sql, "', '");
        strcat(sql, kernel_version->valuestring); strcat(sql, "', '");
        strcat(sql, running_time->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_1->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_2->valuestring); strcat(sql, "', '");
        strcat(sql, load_avg_3->valuestring); strcat(sql, "', '");
        strcat(sql, disk_size->valuestring); strcat(sql, "', '");
        strcat(sql, disk_utilization->valuestring); strcat(sql, "', '");
        strcat(sql, mem_size->valuestring); strcat(sql, "', '");
        strcat(sql, mem_utilization->valuestring); strcat(sql, "', '");
        strcat(sql, disk_stat->valuestring); strcat(sql, "', '");
        strcat(sql, mem_stat->valuestring); strcat(sql, "', '");
        strcat(sql, cpu_stat->valuestring); strcat(sql, "')");
        DBG(YELLOW"%s\n", sql);
        if (mysql_real_query(mysql, sql, strlen(sql))) {
            DBG(RED"error in mysql_real_query sys\n");
        }
    }
}

void *do_task(void *args) {
    //子线程脱离主线程，子线程运行结束后，自己进行资源回收。
    pthread_detach(pthread_self());
    //DBG(GREEN"thread %lu is working\nNONE", pthread_self());
    task_queue *tq = (task_queue *)args;
    MYSQL *mysql = get_db_connect(sq_conn_poll)->mysql_sock;
    while (1) {
        struct monitor_msg_ds *msg = static_cast<monitor_msg_ds *>(pop(tq));
        deal_with_monitor_msg(msg, mysql);
    }
    release_node(sq_conn_poll, reinterpret_cast<SQL_NODE *>(mysql));
}

void *do_dispatch(void *arg) {
    task_queue *tq = (task_queue *)arg;
    struct epoll_event ev, events[MAXEVENTS];
    for (;;) {
        int nfds = epoll_wait(epollfd, events, MAXEVENTS, -1); 
        if (nfds <= 0) {
            perror("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < nfds; i++) {
            int sockfd = events[i].data.fd;
            struct monitor_msg_ds msg;
            if (recv(sockfd, &msg, sizeof(msg), 0) <= 0) {
                epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
                close(sockfd);
            }
            if (msg.type & PI_ACK) {
                DBG(GREEN"<ACK> ack received from %s!\n", inet_ntoa(clients[sockfd]->addr.sin_addr));
                //同时有两个线程在写isonline
                clients[sockfd]->isonline = 5;
            }  else if (msg.type == SYS_MEM || msg.type == SYS_CPU || msg.type == SYS_DISK || msg.type == SYS_SYS) {
                push(tq, &msg);
                DBG(YELLOW"tq size = %d, msg_tpe = %ld\n", tq->cur_num, msg.type);
            }
        }
    }
}

//给所有的客戶都發送一個心跳包
//如果一个客户连续5次都没有回复，那么认为这个客户掉线了，将它从客户列表中移除，并关闭与之通信的sockfd
void heart_beat(int signum) {
    struct monitor_msg_ds msg;
    msg.type = PI_HEART;
    for (auto client : clients) {
        if (client.second->isonline > 0) {
            send(client.first, (void *)&msg, sizeof(msg), 0);
            client.second->isonline--;
            if (client.second->isonline <= 0) {
                epoll_ctl(epollfd, EPOLL_CTL_DEL, client.first, NULL);
                close(client.first);

                delete client.second;
                int tmp = client.first;
                clients.erase(tmp);
                DBG(PINK"<HeartFaild> :  %s is removed from clients list!\n", inet_ntoa(client.second->addr.sin_addr));
            }
        }
    }
}

int check_online(struct sockaddr_in *addr) {
    int flag = 0;
    for (auto client : clients) {
        if (addr->sin_addr.s_addr == client.second->addr.sin_addr.s_addr) {
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
        sockfd = accept(server_listen, (struct sockaddr *)&client, reinterpret_cast<socklen_t *>(&len));
        if (sockfd < 0) {
            perror("accept");
            exit(1);
        }

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
            DBG(RED"<Error> : time out for token!\n");
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
            DBG(RED"<Error> : send ack error!\n");
            continue;
        }
        if (ack == 0) {
            close(sockfd);
            DBG(YELLOW"<Warning> : %s already login!\n", inet_ntoa(client.sin_addr));
            continue;
        }
        if (strcmp(token_tmp, token)) {
            close(sockfd);
            DBG(RED"<Error> : TOKEN Error\n");
            continue;
        }
        //sockfd就是客户

        clients[sockfd] = new client_ds {8, client};

        //注册到反应堆中去
        add_to_reactor(sockfd, epollfd);
        DBG(GREEN"<Reactor> : add client to reactor!\n");
    }
}


void* udp_deal_with (void *data) {
    int udp_server_listen = *((int*)data + 0);
    int port = *((int*)data + 1);
    struct epoll_event events[MAXEVENTS];

    for (;;) {
        int ndfs = epoll_wait(udp_epollfd, events, MAXEVENTS, -1);
        if (ndfs <= 0) {
            perror("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < ndfs; i++) {
            int fd = events[i].data.fd;
            if (fd == udp_server_listen) {
                int sockfd = accept_udp(udp_server_listen, port);
                if (sockfd < 0) {
                    perror("accept_udp_game");
                    exit(1);
                }
                add_to_reactor(sockfd, udp_epollfd);
            } else {
                //这里只能出现SYS类型的消息
                struct monitor_msg_ds msg;
                if (recv(fd, &msg, sizeof(msg), 0) < 0) {
                    epoll_ctl(udp_epollfd, EPOLL_CTL_DEL, fd, NULL);
                }
                if (msg.type == SYS_SYS) {
                    DBG(L_RED"发现警告信息！！！\n");
                }
            }
        }
    }
}

int add_to_reactor(int fd, int epollfd) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return -1;
    }
}

//iterval : 每次闹钟间隔的时间
//next_time: 距离下一次闹钟响还有多久
void set_alarm(int interval, int next_time) {
    struct itimerval itimer;
    itimer.it_interval.tv_sec = interval;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = next_time;
    itimer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itimer, NULL);
}