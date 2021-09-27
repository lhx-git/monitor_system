#ifndef _THREAD_POLL_H
#define _THREAD_POLL_H
typedef struct task_queue {
    int *data;
    int head, tail;
    pthread_mutex_t task_lock;
    pthread_cond_t task_cond;
} task_queue;
task_queue* task_queue_init();
int is_empty(task_queue *tp);
int length(task_queue *tp);
int front(task_queue *tp);
int push(task_queue *tp, int fd);
int pop(task_queue *tp);
void clear(task_queue *tp);
const int task_queue_size = 1000;
#endif