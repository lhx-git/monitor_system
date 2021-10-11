
#include "head.h"
static int opt;
static int port;
char *config = "/home/lhx/CProject/monitor_system/server/monitor.conf";
char token[100];
int epollfd, max, cur_max, thread_num, server_listen;

struct client_ds *clients;



int main(int argc, char **argv) {
    int opt;
    task_queue *tq = task_queue_init(400);
    //实现守护进程
    pthread_t login_tid, reactor_tid;
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

    if ((server_listen = socket_create(port)) < 0) {
        perror("socket_create");
        exit(1);
    }

    if ((epollfd = epoll_create(1)) < 0) {
        perror("epoll_create");
        exit(1);
    }

    struct itimerval itimer;
    //每次闹钟间隔的时间 it-interval
    itimer.it_interval.tv_sec = 1;
    itimer.it_interval.tv_usec = 0;
    //距离下一次闹钟响还有多久
    itimer.it_value.tv_sec = 3;
    itimer.it_value.tv_usec = 0;

    //每隔一秒中产正一个时钟信号。
    setitimer(ITIMER_REAL, &itimer, NULL);


    pthread_create(&login_tid, NULL, do_login, (void *)&server_listen);
    pthread_create(&reactor_tid, NULL, work_on_reactor, tq);
    DBG(GREEN"ready to server!\n");

    pthread_t threads[thread_num];
    DBG(RED"create %d threads\n", thread_num);
    for (int i = 0; i < thread_num; i++) {
        pthread_create(&threads[i], NULL, do_task, tq);
    }
    DBG(RED"Finish\n");
    while (1) {
        sleep(10);
    }
    return 0;
 }
