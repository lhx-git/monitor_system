/*************************************************************************
	> File Name: monitor.h
	> Author:lhx 
	> Mail:207201827@qq.com 
	> Created Time: Thu 16 Sep 2021 09:31:55 AM CST
 ************************************************************************/

#ifndef _MONITOR_H
#define _MONITOR_H


#include <mysql/mysql.h>

#define max(a, b) ( ((a) > (b)) ? (a) : (b) )

#ifndef _R
#define DBG(fmt, args...) printf(fmt, ##args)
#else 
#define DBG(fmt, args...)
#endif 

//比特掩码
#define PI_HEART 0x01
#define PI_ACK 0x02

//定义PINFIN不应应0x03,因为0x01 & 0x02 = 0x03
#define PI_FIN 0x04


#define SYS_MEM 0x04
#define SYS_CPU 0x08
#define SYS_DISK 0x10
#define SYS_SYS 0x20


#define UDP_REQ 0x40
#define UDP_RES 0x80

#define MSG_BUFF_SIZE 500

struct monitor_msg_ds {
    long type;
    char buff[MSG_BUFF_SIZE];
};

struct login_request {
    char token[20];
};
struct login_response {
    int ack;
    char msg[256];
};


struct client_ds {
    int sockfd;
    int isonline;
    struct sockaddr_in addr;
};
void heart_beat(int signum);
void check_for_mem();
void check_for_cpu();
void check_for_disk();
void check_for_sys();
void *do_task(void *arg);
void *do_login(void *arg);
void *work_on_reactor(void *arg);
void *do_work(void *arg);
void *do_msg_queue(void *arg);
void *heart_beat_from_client(void *arg);
int open_file(char *filename);
void deal_with_monitor_msg(struct monitor_msg_ds * msg, MYSQL *mysql);
void empty_file(char *filename);
void do_with_file(int msg_type, int msg_lines, char *filename);
void* udp_deal_with (void *udp_server_listen);
int relogin();
#endif
