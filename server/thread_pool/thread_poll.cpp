#include "head.h"

task_queue* task_queue_init(int task_queue_size) {
    task_queue *tp = (task_queue *)malloc(sizeof(task_queue));
    tp->task_queue_size = task_queue_size;
    DBG(GREEN"初始化任务队列大小 = %d\n" NONE, tp->task_queue_size);
    tp->data = (struct monitor_msg_ds *)malloc(sizeof(struct monitor_msg_ds) * tp->task_queue_size);
    pthread_mutex_init(&(tp->task_lock), NULL);
    pthread_cond_init(&(tp->task_cond), NULL);
    tp->head = tp->tail = 0;
    tp->cur_num = 0;
    return tp;
}

int is_empty(task_queue *tp) {
    int res = tp->head == tp->tail;
    return res;
}

int length(task_queue *tp) {
    int len = (tp->tail - tp->head + tp->task_queue_size) % tp->task_queue_size;
    return len;
}


int push(task_queue *tp, void *arg) {
    //todo this can't get lock
    pthread_mutex_lock(&(tp->task_lock));
    if (tp->head == (tp->tail + 1) % tp->task_queue_size) {
        pthread_mutex_unlock(&(tp->task_lock));
        return -1;
    }
    struct monitor_msg_ds *msg = (struct monitor_msg_ds *)arg;
    tp->data[tp->tail] = *msg;
    tp->cur_num++;
    tp->tail = (tp->tail + 1) % tp->task_queue_size;
    pthread_mutex_unlock(&(tp->task_lock));
    pthread_cond_broadcast(&(tp->task_cond));
    return 1;
}

void* pop(task_queue *tp) {
    pthread_mutex_lock(&(tp->task_lock));
    //判断是否满足执行条件，不满足执行条件，调用pthread_cond_wait阻塞。
    //如果两个或两个以上的进程同时访问队列，需要使用while。考虑一种情况，有两个线程A,B。A先获得互斥锁，然后执行，释放所。然后B获得互斥锁，但是资源没了，B
    //有两个选择，1.访问空的资源，2.继续等待。继续等待就需要使用while。
    //如果只有一个线程访问队列，可以使用if
    while (is_empty(tp)) {
        pthread_cond_wait(&(tp->task_cond), &(tp->task_lock));
    }
    tp->cur_num--;
    struct monitor_msg_ds *msg = &tp->data[tp->head];
    tp->head = (tp->head + 1) % tp->task_queue_size;
    pthread_mutex_unlock(&(tp->task_lock));
    return msg;
}

void clear(task_queue *tp) {
    if (tp == NULL) return ;
    pthread_cond_destroy(&(tp->task_cond));
    pthread_mutex_destroy(&(tp->task_lock));
    free(tp->data);
    free(tp);
    return ;
}