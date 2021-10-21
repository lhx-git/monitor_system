#include "mysql/mysql.h"

#define MAX_CONN_NUM 20
#define IP_LEN 20
#define USERNAME_LEN 30
#define PASSWD_LEN 20
#define DB_NAME_LEN 20

/*
 * 每个连接都应该有断线重连机制
 *
 * */

typedef struct db_connection_node {
    MYSQL *mysql;             //存放mysql连接的指针
    int used;                 //连接节点是否被使用
    enum {              //连接节点的状态
        DB_CONN,
        DB_DIS_CONN
    } state;
    int index;                //下标
    pthread_mutex_t lock;     //互斥锁
} DB_CONN_NODE;

typedef struct db_connection_pool {
    //连接数据库数据
    char ip[IP_LEN];                 //ip
    int port;                 //端口号
    char username[USERNAME_LEN];           //用户名
    char passwd[PASSWD_LEN];             //密码
    char db_name[DB_NAME_LEN];
    //连接节点数据
    DB_CONN_NODE *nodes[MAX_CONN_NUM];      //一堆连接节点
    int conn_node_num;        //连接节点数量
    int busy_number;   //被获取的连接数量
} DB_CONN_POOL;

//创建连接池, conn_node_num表示连接池的最小连接数
DB_CONN_POOL *create_conn_pool(int conn_node_num, char *ip, int port, char *username, char *passwd, char *db_name);
int create_db_conn(DB_CONN_POOL *dp, DB_CONN_NODE *db);
//获取连接池中一个链接
DB_CONN_NODE *get_one_conn(DB_CONN_POOL *db_conn_pool);
//归还连接
void release_conn(DB_CONN_POOL *db_conn_pool, DB_CONN_NODE *db_conn_node);
//销毁连接池
void destroy_conn_pool(DB_CONN_POOL *db_conn_pool);

//增加连接池中的节点数量
int add_conn_node_num(DB_CONN_POOL *db_conn_pool, int num);
