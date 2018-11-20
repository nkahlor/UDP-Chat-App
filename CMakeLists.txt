cmake_minimum_required(VERSION 3.12)
project(Client_Server_Chat)

set(CMAKE_CXX_STANDARD 14)

add_executable(Server_Chat server/udp_server_portal.cc server/server_main.cc server/user.cc portal.cc portal.h)
add_executable(Client_Chat client/udp_client_portal.cc client/client_main.cc portal.cc portal.h)
