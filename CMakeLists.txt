cmake_minimum_required(VERSION 3.1)
project(Semaphore C)

set(CMAKE_C_STANDARD 99)
include_directories(.)
add_executable(Semaphore sem_count.c sem.c thread.o queue.o)
target_link_libraries(Semaphore pthread)

