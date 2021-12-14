#include "mysql/mysql.h"
#ifndef _DB_CONNECTION_POOL_H
#define _DB_CONNECTION_POOL_H

#ifndef __SQLPOOL_H_
#define __SQLPOOL_H_

#define IP_LEN      15
#define DBNAME_LEN  64
#define DBUSER_LEN  64
#define PASSWD_LEN  64
#define POOL_MAX_NUMBER 20


typedef struct _SQL_NODE SQL_NODE;                /*连接节点*/
typedef struct _SQL_CONN_POOL SQL_CONN_POOL;      /*连接池*/

enum SQL_STATE{
    DB_DISCONN, DB_CONN
};

/*连接节点*/
typedef struct _SQL_NODE{
    MYSQL            fd;                  /* mysql对象文件描述符 */
    MYSQL            *mysql_sock;         /* 指向已经连接的MYSQL的指针 */
    pthread_mutex_t  lock;                /* 互斥锁*/
    int              used;                /* 使用标志 */
    int              index;               /* 下标 */
    SQL_STATE sql_state;
}SQL_NODE;

/*连接池*/
typedef struct _SQL_CONN_POOL{
    int        shutdown;                   /*是否关闭*/
    SQL_NODE   sql_pool[POOL_MAX_NUMBER];  /* 一堆连接 */
    int        pool_number;                /* 连接数量 */
    int        busy_number;                /*被获取了的连接数量*/
    char       ip[IP_LEN+1];               /* 数据库的ip */
    int        port;                       /* 数据库的port,一般是3306 */
    char       db_name[DBNAME_LEN+1];      /* 数据库的名字 */
    char       user[DBUSER_LEN+1];         /* 用户名 */
    char       passwd[PASSWD_LEN+1];       /* 密码 */
}SQL_CONN_POOL;

/*创建连接池*/
SQL_CONN_POOL *sql_pool_create(int connect_pool_number, const char* ip, int port,
                               const char* db_name, const char* user, const char* passwd);
/*节点创建连接*/
int create_db_connect(SQL_CONN_POOL *sp, SQL_NODE *node);
/*销毁连接池*/
void sql_pool_destroy(SQL_CONN_POOL *sp);
/*取出一个未使用的连接*/
SQL_NODE *get_db_connect(SQL_CONN_POOL *sp);
/*归回连接*/
void release_node(SQL_CONN_POOL *sp, SQL_NODE *node);
/*增加或删除连接*/
SQL_CONN_POOL *changeNodeNum(SQL_CONN_POOL *sp, int op);

#endif
#endif