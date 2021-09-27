#include "head.h"

task_queue* task_queue_init() {
    task_queue *tp = (task_queue *)malloc(sizeof(task_queue));
    tp->data = (int *)malloc(sizeof(int) * task_queue_size);
    pthread_mutex_init(&(tp->task_lock), NULL);
    pthread_cond_init(&(tp->task_cond), NULL);
    tp->head = tp->tail = 0;
    return tp;
}

int is_empty(task_queue *tp) {
    return tp->head == tp->tail;
}

int length(task_queue *tp) {
    return (tp->tail - tp->head + task_queue_size) % task_queue_size;
}

int front(task_queue *tp) {
    if (is_empty(tp)) return -1;
    return tp->data[tp->head];
}

int push(task_queue *tp, int fd) {
    pthread_mutex_lock(&tp->task_lock);
    if (tp->head == (tp->tail + 1) % task_queue_size) {
        pthread_mutex_unlock(&tp->task_lock);
        return -1;
    }
    tp->data[tp->tail] = fd;
    tp->tail = (tp->tail + 1) % task_queue_size;
    pthread_mutex_unlock(&tp->task_lock);
    pthread_cond_broadcast(&tp->task_cond);
    return 1;
}

int pop(task_queue *tp) {
    pthread_mutex_lock(&tp->task_lock);
    while (is_empty(tp)) {
        pthread_cond_wait(&tp->task_cond, &tp->task_lock);
    }
    if (is_empty(tp)) {
        pthread_mutex_unlock(&tp->task_lock);
        return -1;
    }
    tp->head = (tp->head + 1) % task_queue_size;
    pthread_mutex_unlock(&tp->task_lock);
    return 1;
}

void clear(task_queue *tp) {
    if (tp == NULL) return ;
    pthread_cond_destroy(&(tp->task_cond));
    pthread_mutex_destroy(&(tp->task_lock));
    free(tp->data);
    free(tp);
    return ;
}