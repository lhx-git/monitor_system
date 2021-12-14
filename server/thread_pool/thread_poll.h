#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
typedef struct task_queue {
    struct monitor_msg_ds *data;
    int task_queue_size;
    int head, tail;
    int cur_num;
    pthread_mutex_t task_lock;
    pthread_cond_t task_cond;
} task_queue;
task_queue* task_queue_init(int task_queue_size);
int is_empty(task_queue *tp);
int length(task_queue *tp);
int push(task_queue *tp, void *arg);
void *pop(task_queue *tp);
void clear(task_queue *tp);
#endif