
#include "head.h"

int make_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) return -1;
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}
int make_block(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) return -1;
    flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

int socket_create(int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int));
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
        return -1;
    }
    if (listen(sockfd, 8) < 0) {
        return -1;
    }
    return sockfd;
}

int socket_create_udp(int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        return -1;
    }
    DBG("sockfd %d", sockfd);
    struct sockaddr_in addr;
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int));
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
        return -1;
    }
    return sockfd;
}

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

int recv_file_from_socket(int sockfd, char *name, char *dir) {
    char path[1024] = {0};
    sprintf(path, "%s/%s", dir, name);
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        close(fd);
        return -1;
    }
    while (1) {
        char buff[512] = {0};
        int rsize = recv(sockfd, buff, sizeof(buff), 0);
        if (rsize <= 0) {
            close(fd);
            return rsize;
        }
        int nwrite = write(fd, buff, rsize);
        if (nwrite != rsize) {
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

int send_file_to_socket(int confd, char *name){
    int fd;
    if ((fd = open(name, O_RDONLY)) < 0) {
        return -1;
    }
    while  (1) {
        char buff[512] = {0};
        int rsize = read(fd, buff, sizeof(buff));
        if (rsize <= 0) {
            close(fd);
            close(confd);
            DBG(YELLOW"<Debug>"NONE" : After sent!\n");
            return rsize;
        }
        send(confd, buff, rsize, 0);
    }
}

char *get_conf_value(const char *filename, const char *key) {
    bzero(conf_ans, sizeof(conf_ans));
    FILE *fp;
    char *line = NULL, *sub = NULL;
    ssize_t len = 0, nread = 0;
    if (filename == NULL || key == NULL) {
        return NULL;
    }
    if ((fp = fopen(filename, "r")) == NULL) {
        return NULL;
    }

    while ((nread = getline(&line, &len, fp)) != -1) {
        if ((sub = strstr(line, key)) == NULL) continue;
        if (line[strlen(key)] == '=' && sub == line) {
            strcpy(conf_ans, line + strlen(key) + 1);
            if (conf_ans[strlen(conf_ans) - 1] == '\n')
                conf_ans[strlen(conf_ans) - 1] = '\0';
        }
    }
    free(line);
    fclose(fp);
    return conf_ans;
}


int socket_udp() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("udp socket");
        exit(1);
    }
    return sockfd;
}

int check_user(struct login_request *loginRequest) {
    return 1;
}

int accept_udp(int listener, int port) {
    int sockfd;
    struct login_request loginRequest;
    struct login_response loginResponse;
    bzero(&loginRequest, sizeof(loginRequest));
    bzero(&loginResponse, sizeof(loginResponse));
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    socklen_t len = sizeof(client);
    int ret = recvfrom(listener, (void *)&loginRequest, sizeof(loginRequest), 0, (struct sockaddr *)&client, &len);
    if (ret != sizeof(loginRequest) || !check_user(&loginRequest)) {
        DBG(RED"<loginRequest Error>"NONE"%s:%d...\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        return -1;
    }
    DBG(L_BLUE"<loginRequest recived>"NONE"%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    loginResponse.ack = UDP_RES;
    if ((sockfd = socket_create_udp(port)) < 0) {
        return -1;
    }
    int retval = connect(sockfd, (struct sockaddr *)&client, len);
    if (retval < 0) {
        perror("connect retval");
    }

    if (send(sockfd, (void *)&loginResponse, sizeof(loginResponse), 0) < 0) {
        perror("send");
    }
    DBG(L_BLUE"<loginResponse sent>"NONE"%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    DBG(GREEN"<Connect Success>"NONE"%s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    //connected
    return sockfd;
}

int socket_connect_udp(const char *ip, int port, struct login_request *loginRequest) {
    //struct udp_data_ds msg;
    struct login_response loginResponse;
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
    send(sockfd, (void *)loginRequest, sizeof(struct login_request), 0);
    DBG(L_BLUE"<loginRequest Sent>"NONE"%s:%d...\n", ip, port);
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
        DBG(RED"<loginRequest TimeOut>"NONE"%s:%d...\n", ip, port);
        return -1;
    }
    int ret = recv(sockfd, (void *)&loginResponse, sizeof(loginResponse), 0);
    if (ret != sizeof(loginResponse) || (loginResponse.ack & UDP_RES) == 0) {
        DBG(RED"<loginResponse Error>"NONE"%s:%d...\n", ip, port);
        return -1;
    }
    DBG(GREEN"<Connected>"NONE"%s:%d\n", ip, port);
    return sockfd;
}


void add_to_reactor(int fd, int epollfd) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return ;
    }
}
