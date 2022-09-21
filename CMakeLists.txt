cmake_minimum_required(VERSION 3.23)
project(CIS457Project1 C)

set(CMAKE_C_STANDARD 11)

add_executable(CIS457Project1
        udpclient.c
        udpserver.c)
