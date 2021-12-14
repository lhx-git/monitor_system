//
// Created by lhx on 12/13/21.
//
#include "head.h"
#include "function.h"
//读文件，将文件内从新打包成消息重新发送，最后清空文件。
void do_with_file(int msg_type, int msg_lines, const char *filename, int sockfd) {
    //找了半天的bug！！！！！
    // 由于缓存temp设置太小，导致出现stack smashing detected错误。
    char a[MSG_BUFF_SIZE];
    FILE *fp = fopen(filename, "r");
    char temp[MSG_BUFF_SIZE];

    int lines = msg_lines;
    while (fgets(a, MSG_BUFF_SIZE, fp) != NULL) {
        lines--;
        strncat(temp, a,MSG_BUFF_SIZE);
        if (lines == 0) {
            lines = msg_lines;
            struct monitor_msg_ds msg;
            msg.type = msg_type;
            strncpy(msg.buff, temp, MSG_BUFF_SIZE);
            //如果msg内是空的，就不发送给服务器了
            if (strlen(msg.buff) == 0) break;
            memset(temp, 0, MSG_BUFF_SIZE);
            if (send(sockfd, (void *)&msg, sizeof (msg), 0) < 0) {
                perror("send mem message");
            }
        }
    }
    fclose(fp);
    //读完文件后，将文件清空。
    empty_file(filename);
}

int open_file(const char *filename) {
    int fd;
    //以可读可写可添加的方式，打开一个文件，如果文件不存在，创建此文件
    if ((fd = open(filename, O_CREAT | O_RDWR | O_APPEND, 0666)) < 0) {
        perror("persistence file open");
        exit(1);
    }
    return fd;
}

void empty_file(const char *filename) {
    //以“w“的方式打开文件相当与清空文件。
    //不能使用系统调用open（w），不会清空文件。
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
        perror("empty file");
        exit(1);
    }
    fclose(fp);
}