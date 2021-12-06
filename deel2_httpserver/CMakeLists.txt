cmake_minimum_required(VERSION 3.10)
# set the project name
project(deel2_httpserver)
#add the libraries
link_libraries(libtlpi.a PJ_RPI)
# add the socket_client executable
add_executable(Client ud_ucase_cl.c)
# add the http_server executable
add_executable(http http_server.c)
