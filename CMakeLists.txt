cmake_minimum_required(VERSION 3.20)
project(monitor_system C)

set(CMAKE_C_STANDARD 99)
#头文件目录
include_directories(common)

set(SERVER_DEP
        server/server.c
        server/server_func.c
        common/common.c
        common/cJSON.c)

set(CLIENT_DEP client/client.c
        common/common.c
        common/cJSON.c
        client/client_func.c)

#生成可执行文件
add_executable(server ${SERVER_DEP})
#添加链接库
target_link_libraries(server -lpthread)

add_executable(client ${CLIENT_DEP})
target_link_libraries(client -lpthread)