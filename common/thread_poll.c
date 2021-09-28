#include "head.h"

//mistake
//i execute pthread_mutex_lock after i has already get lock.
task_queue* task_queue_init(int task_queue_size) {
    task_queue *tp = (task_queue *)malloc(sizeof(task_queue));
    tp->task_queue_size = task_queue_size;
    DBG(GREEN"init task que with size = %d\n", tp->task_queue_size);
    tp->data = (int *)malloc(sizeof(int) * tp->task_queue_size);
    pthread_mutex_init(&(tp->task_lock), NULL);
    pthread_cond_init(&(tp->task_cond), NULL);
    tp->head = tp->tail = 0;
    tp->cur_num = 0;
    return tp;
}

int is_empty(task_queue *tp) {
    //pthread_mutex_lock(&tp->task_lock);
    int res = tp->head == tp->tail;
    //pthread_mutex_unlock(&tp->task_lock);
    return res;
}

int length(task_queue *tp) {
    //pthread_mutex_lock(&tp->task_lock);
    int len = (tp->tail - tp->head + tp->task_queue_size) % tp->task_queue_size;
    //pthread_mutex_unlock(&tp->task_lock);
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
    DBG(RED"<PUSH>\n");
    tp->tail = (tp->tail + 1) % tp->task_queue_size;
    pthread_mutex_unlock(&(tp->task_lock));
    pthread_cond_broadcast(&(tp->task_cond));
    return 1;
}

void* pop(task_queue *tp) {
    pthread_mutex_lock(&(tp->task_lock));
    while (is_empty(tp)) {
        pthread_cond_wait(&(tp->task_cond), &(tp->task_lock));
    }
    if (is_empty(tp)) {
        pthread_mutex_unlock(&(tp->task_lock));
        return NULL;
    }
    tp->cur_num--;
    struct monitor_msg_ds *msg = &tp->data[tp->head];
    DBG(RED"<POP>\n");
    tp->head = (tp->head + 1) % tp->task_queue_size;
    pthread_mutex_unlock(&(tp->task_lock));
    return msg;
}

void clear(task_queue *tp) {
    if (tp == NULL) return ;
    //pthread_cond_destroy(&(tp->task_cond));
    //pthread_mutex_destroy(&(tp->task_lock));
    free(tp->data);
    free(tp);
    return ;
}