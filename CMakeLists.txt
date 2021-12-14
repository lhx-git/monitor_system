cmake_minimum_required(VERSION 3.20)
project(monitor_system)

set(CMAKE_CXX_STANDARD 11)
#头文件目录
include_directories(common)
include_directories(server)
include_directories(client)
include_directories(server/db_connection_pool)
include_directories(server/thread_pool)

set(SERVER_DEP
        server/server.cpp
        server/server_func.cpp
        common/common.cpp
        server/db_connection_pool/db_connection_pool.cpp
        server/thread_pool/thread_poll.cpp
        common/cJSON/cJSON.cpp)

set(DB_CONN_POOL
        common/)

set(CLIENT_DEP client/client.cpp
        common/common.cpp
        common/cJSON/cJSON.cpp
        client/send_file.cpp
        client/collect_local_data.cpp
        client/login.cpp
        client/send_data_to_server.cpp
        client/function.h)

#生成可执行文件
add_executable(server ${SERVER_DEP} ${DB_CONN_POOL})
#添加链接库
target_link_libraries(server -lpthread -lmysqlclient)

add_executable(client ${CLIENT_DEP})
target_link_libraries(client -lpthread)


#测试
add_executable(server_test test/server_test.c common/common.cpp)

add_executable(client_test test/client_test.c common/common.cpp)


add_executable(threadPool_test test/threadPool.c)
target_link_libraries(threadPool_test -lpthread)