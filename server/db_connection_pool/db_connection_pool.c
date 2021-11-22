#include "head.h"
#include "db_connection_pool.h"

DB_CONN_POOL *create_conn_pool(int conn_node_num, char *ip, int port, char *username, char *passwd, char *db_name) {
    if (conn_node_num > MAX_CONN_NUM)
        conn_node_num = MAX_CONN_NUM;
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library\n");
        exit(1);
    }
    DB_CONN_POOL *db_conn_pool = (DB_CONN_POOL *)malloc(sizeof(DB_CONN_POOL));
    strcpy(db_conn_pool->ip, ip);
    db_conn_pool->port = port;
    db_conn_pool->busy_number = 0;
    strcpy(db_conn_pool->username, username);
    strcpy(db_conn_pool->passwd, passwd);
    strcpy(db_conn_pool->db_name, db_name);
    db_conn_pool->conn_node_num = conn_node_num;
    for (int i = 0; i < conn_node_num; i++) {
        db_conn_pool->nodes[i] = (DB_CONN_NODE *)malloc(sizeof(DB_CONN_NODE));
        //创建失败
        if (0 == create_db_conn(db_conn_pool, db_conn_pool->nodes[i])) {
            perror("error in create_conn_pool");
            //销毁连接池
            destroy_conn_pool(db_conn_pool);
            exit(1);
        }
        //创建成功
        db_conn_pool->nodes[i]->index = i;
    }
    DBG(YELLOW"启动数据库连接池成功！！\n"GREEN);
    return db_conn_pool;
}


int create_db_conn(DB_CONN_POOL *dp, DB_CONN_NODE *db) {
    //res 1 success , 0 fail
    int res = 0;
    if (dp == NULL) {
        DBG(RED"conn_pool is NUll\n");
        return res;
    }
    pthread_mutex_init(&db->lock, NULL);
    if ((db->mysql = mysql_init(NULL)) == NULL) {
        perror("error in mysql_init");
        return res;
    }
    if (!mysql_real_connect(db->mysql, dp->ip, dp->username, dp->passwd, dp->db_name, dp->port, 0, 0)) {
        perror("error in mysql_real_connect");
        return res;
    }
    res = 1;
    db->used = 0;
    db->state = DB_CONN;
    return res;
}

DB_CONN_NODE *get_one_conn(DB_CONN_POOL *db_conn_pool) {
    if (db_conn_pool == NULL) return NULL;
    //获取连接池 ,随机获取。
    srand(time(0));
    int start_index, index, i;
    start_index = rand() % db_conn_pool->conn_node_num;
    for (i = 0; i < db_conn_pool->conn_node_num; i++) {
        index = (start_index + i) % db_conn_pool->conn_node_num;
        if (!pthread_mutex_trylock(&(db_conn_pool->nodes[index]->lock))) {
            break;
        }
    }
    if (i == db_conn_pool->conn_node_num) {
        return NULL;
    } else {
        db_conn_pool->nodes[index]->used = 1;
        db_conn_pool->busy_number++;
        return db_conn_pool->nodes[index];
    }
}

void release_conn(DB_CONN_POOL *db_conn_pool, DB_CONN_NODE *db_conn_node) {
    if (db_conn_pool == NULL) return ;
    if (db_conn_node == NULL) return ;
    db_conn_node->used = 0;
    db_conn_pool->busy_number--;
    pthread_mutex_unlock(&(db_conn_node->lock));
    return ;
}

void destroy_conn_pool(DB_CONN_POOL *db_conn_pool) {
    if (db_conn_pool == NULL) return ;
    for (int i = 0; i < db_conn_pool->conn_node_num; i++) {
        mysql_close(db_conn_pool->nodes[i]->mysql);
        free(db_conn_pool->nodes[i]);
    }
    free(db_conn_pool);
    return ;
}


int add_conn_node_num(DB_CONN_POOL *db_conn_pool, int num) {
    if (db_conn_pool == NULL) return 0;
    if (db_conn_pool->conn_node_num == MAX_CONN_NUM) {
        fprintf(stdout, "connection_pool is full\n");
        return 0;
    } else if (db_conn_pool->conn_node_num + num < MAX_CONN_NUM) {
        for (int i = db_conn_pool->conn_node_num; i < db_conn_pool->conn_node_num + num; i++) {
            db_conn_pool->nodes[i] = (DB_CONN_NODE *)malloc(sizeof(DB_CONN_NODE));
            if (0 == create_db_conn(db_conn_pool, db_conn_pool->nodes[i])) {
                perror("create_db_conn fail");
                return 0;
            }
        }
    } else {
        for (int i = db_conn_pool->conn_node_num; i < MAX_CONN_NUM; i++) {
            if (0 == create_db_conn(db_conn_pool, db_conn_pool->nodes[i])) {
                perror("create_db_conn fail");
                return 0;
            }
        }
    }
    return 1;
}

