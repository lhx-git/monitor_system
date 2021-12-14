//
// Created by lhx on 12/13/21.
//

#ifndef MONITOR_SYSTEM_FUNCTION_H
#define MONITOR_SYSTEM_FUNCTION_H
class MYSQL;

//worker线程函数：从任务队列中获取任务，并处理
void *do_task(void *arg);
//负责验证客户端的登录的线程函数：登录成功的客户端的sockfd被加入到 epoll实例中。
void *do_login(void *arg);
//dispatcher线程函数：通过epoll 获取客户端发来的消息，根据消息类型，做不同的的处理（包含任务分发）
void *do_dispatch(void *arg);


//udp数据紧急处理线程函数
void* udp_deal_with (void *udp_server_listen);


//解析客户端发来的消息，并将数据存储到数据库中
void deal_with_monitor_msg(struct monitor_msg_ds * msg, MYSQL *mysql);
//通过信号量机制，唤醒
void heart_beat(int signum);
//fd加入epoll中
int add_to_reactor(int fd, int epollfd);
//设置闹钟
void set_alarm(int interval, int next_time);

//客户端结构体
struct client_ds {
    int isonline; //是否在线
    struct sockaddr_in addr;// 客户端ip、port
};
#endif //MONITOR_SYSTEM_FUNCTION_H
