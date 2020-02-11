cmake_minimum_required(VERSION 3.10)
project(untitled_folder C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled_folder sem_count.c sem.c sem.h queue.h thread.h)