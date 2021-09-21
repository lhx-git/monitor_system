/*************************************************************************
	> File Name: monitor.h
	> Author:lhx 
	> Mail:207201827@qq.com 
	> Created Time: Thu 16 Sep 2021 09:31:55 AM CST
 ************************************************************************/

#ifndef _MONITOR_H
#define _MONITOR_H

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

struct monitor_msg_ds {
    long type;
    char buff[200];
};


struct client_ds {
    int sockfd;
    int isonline;
    struct sockaddr_in addr;
};
void heart_beat(int signum);
void check_for_mem();
void *do_login(void *arg);
void *work_on_reactor(void *arg);
void *do_work(void *arg);
void *do_msg_queue(void *arg);
#endif
