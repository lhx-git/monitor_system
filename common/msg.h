
#ifndef _MONITOR_H
#define _MONITOR_H

/*------------------------TCP消息类型------------------------*/
//心跳消息
#define PI_HEART 0x01
#define PI_ACK 0x02
//数据消息
#define SYS_MEM 0x04
#define SYS_CPU 0x08
#define SYS_DISK 0x10
#define SYS_SYS 0x20
/*------------------------UDP消息类型------------------------*/
#define UDP_RES 0x80

/*------------------------buffer大小------------------------*/
#define MSG_BUFF_SIZE 500
#define TOKEN_LEN 20


/*------------------------消息结构体------------------------*/
struct monitor_msg_ds {
    long type;  //消息类型
    char buff[MSG_BUFF_SIZE];  //消息数据
};

#endif
