cmake_minimum_required(VERSION 3.12)
project(Client_Server_Chat)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

add_executable(Server_Chat server/udp_server_portal.cc server/server_main.cc server/user.cc portal.cc portal.h)
add_executable(Client_Chat client/udp_client_portal.cc client/client_main.cc portal.cc portal.h client/gui.cc client/gui.h)

target_link_libraries(Client_Chat Qt5::Widgets)
