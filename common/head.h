/*#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int socket_create(int port);

int socket_connect(const char *ip, int port);*/


/*************************************************************************
	> File Name: head.h
	> Author: suyelu
	> Mail: suyelu@126.com
	> Created Time: Sat 03 Jul 2021 04:37:40 PM CST
 ************************************************************************/

#ifndef _HEAD_H
#define _HEAD_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/epoll.h>
//#include <ncurses.h>
#include <locale.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/msg.h>
#include "color.h"
#include "monitor.h"
#include "common.h"
#include <mysql/mysql.h>
#include "cJSON.h"
#include "thread_poll.h"
//#include "datatype.h"
//#include "file_transfer.h"
//#include "wechat.h"
//#include "wechat_ui.h"
//#include "sem.h"
//#include "pihealth.h"

char conf_ans[512];
#endif
