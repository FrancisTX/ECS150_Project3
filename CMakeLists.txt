cmake_minimum_required(VERSION 3.10)
project(Semaphore C)
set(CMAKE_C_STANDARD 99)
include_directories(.)
add_executable(Semaphore sem_count.c sem.c thread.o queue.o)
add_executable(prime sem.c sem_prime.c thread.o queue.o)
add_executable(ddd tps.c)
target_link_libraries(Semaphore pthread)
target_link_libraries(prime pthread)





