
#include "head.h"


//-------------------------------------------TCP-------------------------------------------
int socket_create(int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){//创建一个 TCP socket
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    //SO_REUSEADDR是让端口释放后立即就可以被再次使用。
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int));
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) { //绑定端口
        return -1;
    }
    if (listen(sockfd, 8) < 0) {  //开始监听，设置连接队列大小为8
        return -1;
    }
    return sockfd;
}

//客户端连接 对应ip、port的服务器进程。
int socket_connect(const char *ip, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    return sockfd;
}

//-------------------------------------------UDP-------------------------------------------
int socket_create_udp(int port) {//创建一个 udp socket，绑定端口
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){//创建一个 udp socket
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int));
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {//绑定端口
        return -1;
    }
    return sockfd;
}

int socket_udp() {//创建一个 udp socket，不绑定端口
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    return sockfd;
}

int check_user(const char *token, int len) {
    return 1;
}

//udp也可以调用connect函数达到面向连接
//如果不调用connect函数，udp发送数据通过 sendto、recvfrom
//调用connect函数，udp发送数据可以通过 send recv
//udp调用connect函数最主要的目的只是记录对端的ip地址和端口，并让内核检查是否存在错误（例如端口不可达等）

int accept_udp(int listener, int port) {
    int sockfd;
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    socklen_t len = sizeof(client);

    char *token[TOKEN_LEN];
    int ret = recvfrom(listener, (void *)&token, TOKEN_LEN, 0, (struct sockaddr *)&client, &len);//接收token
    if (ret != TOKEN_LEN || !check_user(reinterpret_cast<const char *>(token), TOKEN_LEN)) {
        DBG(RED"<UDPLoginRequest Error>" NONE "%s:%d...\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        return -1;
    }
    DBG(L_BLUE"<UDPLoginRequest recived>" NONE "%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    int res = UDP_RES;
    if ((sockfd = socket_create_udp(port)) < 0) {
        return -1;
    }
    int retval = connect(sockfd, (struct sockaddr *)&client, len);
    if (retval < 0) {
        perror("connect retval");
    }

    if (send(sockfd, (void *)&res, sizeof(int), 0) < 0) {//返回udp-res
        perror("send");
    }
    DBG(L_BLUE"<UDPLoginRequest sent>" NONE"%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    DBG(GREEN"<Connect Success>" NONE"%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    return sockfd;
}

int socket_connect_udp(const char *ip, int port, const char *token) {
    int sockfd;
    if ((sockfd = socket_udp()) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    //发送token
    send(sockfd, (void *)token, TOKEN_LEN, 0);
    DBG(L_BLUE"<UDPLoginRequest Sent>" NONE"%s:%d...\n", ip, port);
    //使用select 计时器
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    //设置select timeout参数为5秒钟，5秒钟之后 如果select没有检测到任何就绪的文件描述符，select返回-1
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
        DBG(RED"<UDPLoginRequest TimeOut>" NONE"%s:%d...\n", ip, port);
        return -1;
    }
    //接受udp-res
    int res;
    int ret = recv(sockfd, (void *)&res, sizeof(int), 0);
    if (ret != sizeof(int) || res != UDP_RES) {
        DBG(RED"<UDPLoginRequest Error>" NONE"%s:%d...\n", ip, port);
        return -1;
    }
    DBG(GREEN"<Connected>" NONE"%s:%d\n", ip, port);
    return sockfd;
}



//-------------------------------------------读取配置文件-------------------------------------------
char conf_buf[512];
char *get_conf_value(const char* filename, const char *key) {
    bzero(conf_buf, sizeof(conf_buf));
    FILE *fp;
    char *line = NULL, *sub = NULL;
    ssize_t len = 0, nread = 0;
    if (filename == NULL || key == NULL) {
        return NULL;
    }
    if ((fp = fopen(filename, "r")) == NULL) {
        return NULL;
    }

    while ((nread = getline(&line, reinterpret_cast<size_t *>(&len), fp)) != -1) {
        if ((sub = strstr(line, key)) == NULL) continue;
        if (line[strlen(key)] == '=' && sub == line) {
            strcpy(conf_buf, line + strlen(key) + 1);
            if (conf_buf[strlen(conf_buf) - 1] == '\n')
                conf_buf[strlen(conf_buf) - 1] = '\0';
        }
    }
    free(line);
    fclose(fp);
    return conf_buf;
}





