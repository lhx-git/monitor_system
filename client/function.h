#ifndef MONITOR_SYSTEM_FUNCTION_H
#define MONITOR_SYSTEM_FUNCTION_H
//文件操作
int open_file(const char *filename);
void empty_file(const char *filename);
//读文件，将文件内从从新打包成消息重新发送，最后清空文件。
void do_with_file(int msg_type, int msg_lines, const char *filename, int sockfd);

//获取当前节点的内存、cpu、disk、sys信息
void check_for_mem();
void check_for_cpu();
void check_for_disk();
void check_for_sys();

//线程函数：客户端收集本地的数据，将数据打包成消息后，将消息加入本地的消息队列
void *check_data_and_send_to_msg_queue(void *arg);
//线程函数：从本地消息队列获取消息，然后发送给服务端
void *send_data_to_server(void *arg);

//客户端使用token登录服务器
int login();
//判断是否需要重连，当需要重连时，唤醒负责重连的线程
void try_to_relogin();
//线程函数：阻塞，直到需要进行重新连接时
void *relogin(void *arg);
#endif //MONITOR_SYSTEM_FUNCTION_H
