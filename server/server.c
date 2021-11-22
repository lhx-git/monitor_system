
#include "head.h"
#include "db_connection_pool.h"
static int opt;
static int port;
char *config = "/home/lhx/CProject/monitor_system/server/monitor.conf";
char token[100];
int epollfd, max, cur_max, thread_num, server_listen, udp_server_listen, udp_epollfd;
DB_CONN_POOL *db_conn_poll;

struct client_ds *clients;



int main(int argc, char **argv) {
    int opt;
    task_queue *tq = task_queue_init(400);
    //实现守护进程
    pthread_t login_tid, reactor_tid, udp_deal_tid;
    while ((opt = getopt(argc, argv, "p:t:m:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                strcpy(token, optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage : %s -p port", argv[0]);
        }
    }
    if (!port) port = atoi(get_conf_value(config, "PORT"));
    if (!max) max = atoi(get_conf_value(config, "MAX"));
    if (!thread_num) thread_num = atoi(get_conf_value(config, "TRHEAD_NUM"));
    if (!strlen(token)) strcpy(token, get_conf_value(config, "TOKEN"));
    signal(SIGALRM, heart_beat);

    clients = (struct client_ds*)calloc(max, sizeof(struct client_ds));
    //创建一个TCP Socket
    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        exit(1);
    }
    //创建一个epoll实例
    if ((epollfd = epoll_create(1)) < 0) {
        perror("epoll_create");
        exit(1);
    }
    if ((udp_server_listen = socket_create_udp(port)) < 0) {
        perror("listener");
        exit(1);
    }
    DBG(YELLOW"UDP套接字listener创建成功.\n"NONE);

    if ((udp_epollfd = epoll_create(1)) < 0) {
        perror("Main Reactor");
        exit(1);
    }
    struct epoll_event ev;
    ev.data.fd = udp_server_listen;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(udp_epollfd, EPOLL_CTL_ADD, udp_server_listen, &ev);
    if (ret < 0) {
        perror("epoll_ctl");
        exit(1);
    }
    DBG(YELLOW"创建主反应堆，并将listener加入主反应堆.\n"NONE);

    struct itimerval itimer;
    //每次闹钟间隔的时间 it-interval
    itimer.it_interval.tv_sec = 1;
    itimer.it_interval.tv_usec = 0;
    //距离下一次闹钟响还有多久
    itimer.it_value.tv_sec = 3;
    itimer.it_value.tv_usec = 0;

    //每隔一秒中产正一个时钟信号。
    setitimer(ITIMER_REAL, &itimer, NULL);

    //初始化并连接数据库
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library\n");
        exit(1);
    }
    //创建连接池
    db_conn_poll = create_conn_pool(20, "82.156.196.36", 0, "lhx", "12345678", "mysql");
    //创建一个负责管理客户端登录的线程、验证客户端发送的密码后者Token正确性
    pthread_create(&login_tid, NULL, do_login, (void *)&server_listen);
    //创建一个管理udp数据包的线程
    int data[2] = {udp_server_listen, port};
    pthread_create(&udp_deal_tid, NULL, udp_deal_with, (void *)data);
    //创建一个反应堆线程、通过poll_wait来监听所有的客户端连接、然后根据消息类型放入各自的消息队列


    pthread_create(&reactor_tid, NULL, work_on_reactor, tq);
    DBG(GREEN"ready to server!\n");
    //创建工作线程
    pthread_t threads[thread_num];
    DBG(RED"create %d threads\n", thread_num);
    for (int i = 0; i < thread_num; i++) {
        pthread_create(&threads[i], NULL, do_task, tq);
    }
    DBG(RED"Finish\n");



    while (1) {
        sleep(10);
    }
    mysql_library_end();
    return 0;
 }
