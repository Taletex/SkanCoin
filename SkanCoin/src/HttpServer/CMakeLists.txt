cmake_minimum_required(VERSION 3.0)
project (skancoin_http_server)

add_executable(helloworld HttpServer.cpp)
target_link_libraries(helloworld ${CMAKE_THREAD_LIBS_INIT})
