//
// Created by Zhecheng Huang on 2/17/20.
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sem.h>
#include "thread.h"
#include "queue.h"
#include "tps.h"

#define TEST_ASSERT(assert)            \
do {                        \
    printf("ASSERT: " #assert " ... ");    \
    if (assert) {                \
        printf("PASS\n");        \
    } else    {                \
        printf("FAIL\n");        \
        exit(1);            \
    }                    \
} while(0)

void *latest_mmap_addr;

void *__real_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
  latest_mmap_addr = __real_mmap(
      addr,
      len,
      prot,
      flags,
      fildes,
      off);
  return latest_mmap_addr;
}

void *test_write_and_read() {                             //tid[1]
  tps_create();
  char sample[] = "hello";
  char sample_from_read[strlen(sample)];
  tps_write(0, strlen(sample), sample);
  tps_read(0, strlen(sample), sample_from_read);
  TEST_ASSERT(strcmp(sample, sample_from_read));

  return NULL;
}

void *test_write_and_read_offset() {                     //tid[2]
  tps_create();
  char sample[] = "hello ECS_150";
  char sample_from_read[strlen(sample)];
  tps_read(7, strlen(sample), sample_from_read);
  TEST_ASSERT(strcmp("ECS_150", sample_from_read));

  return NULL;
}

void *test_clone(pthread_t clone_tid){                   //tid[3]
  char buffer[10];             //clone the thread whose tid is 1 
  char write_to_memory[] = " Winter";
  tps_clone(clone_tid);
  tps_read(0, strlen(buffer), buffer);
  TEST_ASSERT(strcmp("hello", buffer));

  tps_write(6, strlen(write_to_memory), write_to_memory);
  tps_read(0, strlen(buffer), buffer);
  TEST_ASSERT(strcmp("hello Winter", buffer));

  return 0;
}
int main() {
  pthread_t tid[100];
  tps_init(0);

  pthread_create(&tid[1], NULL, (void*)test_write_and_read, NULL);
  pthread_join(tid[1], NULL);

  pthread_create(&tid[2], NULL, (void*)test_write_and_read_offset, NULL);
  pthread_join(tid[2], NULL);

  pthread_create(&tid[3], NULL, (void *)test_clone, &tid[1]);
  pthread_join(tid[3], NULL);

  //pthread_create(&tid[4], NULL, test_write_and_read_offset, NULL);
  //pthread_join(tid[4], NULL);



  

  return 0;
}
