#include "head.h"
#include "function.h"
#include <map>

const char* config = "/home/lhx/CProject/monitor_system/server/monitor.conf";//配置文件
char token[TOKEN_LEN];//token
int port;//端口号
SQL_CONN_POOL *sq_conn_poll;//连接池
std::map<int ,struct client_ds *> clients;//客户端数据

int thread_num;//线程数量
int task_queue_size;//任务队列大小

//tcp
int epollfd, server_listen;
//udp
int udp_epollfd, udp_server_listen;

int main(int argc, char **argv) {
    /*------------------------解析命令行参数-----------------------*/
    int opt;
    pthread_t login_tid, reactor_tid, udp_deal_tid;
    while ((opt = getopt(argc, argv, "p:t:T:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                strcpy(token, optarg);
                break;
            case 'T':
                thread_num = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage : %s -p port -t token -T thread_num", argv[0]);
        }
    }

    /*------------------------读取配置文件-----------------------*/
    if (!port) port = atoi(get_conf_value(config, "PORT"));
    if (!thread_num) thread_num = atoi(get_conf_value(config, "TRHEAD_NUM"));
    if (!strlen(token)) strcpy(token, get_conf_value(config, "TOKEN"));
    if (!task_queue_size) task_queue_size = atoi(get_conf_value(config, "TASK_QUEUE_SIZE"));

    /*------------------------信号量设置-----------------------*/
    //设置SIGALRM信号量的handler函数为 heart_beat
    signal(SIGALRM, heart_beat);
    //每隔1秒产生一个时钟信号
    set_alarm(1, 3);

    /*------------------------TCP-----------------------*/
    //open tcp listen
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
    /*------------------------数据库连接池初始化-----------------------*/
    //初始化并连接数据库
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library\n");
        exit(1);
    }
    //创建连接池
    sq_conn_poll = sql_pool_create(10, "127.0.0.0.1", 3306, "mysql", "root", "12345678");
    if (sq_conn_poll != NULL) {
        DBG(YELLOW"数据库连接池创建成功\n" NONE);
    } else {
        perror("sql_pool_create");
        exit(1);
    }
    //创建一个负责管理客户端登录的线程、验证客户端发送的密码后者Token正确性
    pthread_create(&login_tid, NULL, do_login, (void *)&server_listen);

    /*------------------------线程池初始化-----------------------*/
    //初始化任务队列
    task_queue *tq = task_queue_init(task_queue_size);
    //创建工作线程
    pthread_t threads[thread_num];
    for (int i = 0; i < thread_num; i++) {
        pthread_create(&threads[i], NULL, do_task, tq);
    }
    DBG(YELLOW"成功创建%d个工作线程.\n" NONE, thread_num);

    /*------------------------TCP主反应堆线程-----------------------*/
    //创建一个主反应堆线程、通过epoll_wait来监听所有的客户端连接、然后根据消息类型放入各自的消息队列
    pthread_create(&reactor_tid, NULL, do_dispatch, tq);

    /*------------------------UDP-----------------------*/
    if ((udp_server_listen = socket_create_udp(port)) < 0) {
        perror("listener");
        exit(1);
    }
    DBG(YELLOW"UDP套接字udp_listener创建成功.\n" NONE);
    if ((udp_epollfd = epoll_create(1)) < 0) {
        perror("udp_epoll_create");
        exit(1);
    }
    if (add_to_reactor(udp_server_listen, udp_epollfd) < 0) {
        perror("udp_epoll_ctl");
        exit(1);
    }
    DBG(YELLOW"创建udp主反应堆，并将udp_listener加入主反应堆.\n" NONE);
    DBG(YELLOW"ready to server!\n" NONE);

    /*------------------------UDP紧急数据处理线程-----------------------*/
    //创建一个管理udp数据包的线程
    int data[2] = {udp_server_listen, port};
    pthread_create(&udp_deal_tid, NULL, udp_deal_with, (void *)data);

    while (1) {
        sleep(10);
    }
    mysql_library_end();
    return 0;
 }
