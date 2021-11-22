/*************************************************************************
	> File Name: common.h
	> Author:lhx 
	> Mail:207201827@qq.com 
	> Created Time: Thu 16 Sep 2021 09:54:36 AM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
int make_nonblock(int fd);
int make_block(int fd);
int socket_create(int port);
int socket_create_udp(int port);
int socket_connect(const char *ip, int port);
int recv_file_from_socket(int sockfd, char *name, char *dir);
int send_file_to_socket(int confd, char *name);
char *get_conf_value(const char *filename, const char *key);
int accept_udp(int listener, int port);
int socket_connect_udp(const char *ip, int port, struct login_request *loginRequest);
void add_to_reactor(int fd, int epollfd);

#endif
