//
// Created by Zhecheng Huang on 2/17/20.
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sem.h>
#include "thread.h"
#include "sem.h"
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

static sem_t sem1, sem2;

void *test_clone(pthread_t clone_tid){
  char buffer[TPS_SIZE];             //clone the thread whose tid is 1 
  char write_to_memory[] = " Winter";

  /*clone previous thread tps and test the memoery page*/
  tps_clone(clone_tid);
  tps_read(0, TPS_SIZE, buffer);
  TEST_ASSERT(strcmp("hello", buffer));

  /*write on the current tps's memory page*/
  tps_write(6, strlen(write_to_memory), write_to_memory);
  tps_read(0, TPS_SIZE, buffer);
  TEST_ASSERT(strcmp("hello Winter", buffer));

  sem_up(sem1);

  return 0;
}
void *test_write_and_read() {  
  pthread_t tid;	
  tps_create();
  char sample[] = "hello";
  char sample_from_read[strlen(sample)];
  tps_write(0, strlen(sample), sample);                  //write "hello" in the memory page
  tps_read(0, strlen(sample), sample_from_read);
  TEST_ASSERT(strcmp(sample, sample_from_read));
  
  pthread_create(&tid, NULL, (void*) test_clone, (void*)&tid);
  sem_down(sem1);                                        //block this thread and go to test_clone()
  
  /*Wait for test_clone die, and check if the tps of current thread are modified*/
  pthread_join(tid, NULL);

  tps_read(0, strlen(sample), sample_from_read);
  TEST_ASSERT(strcmp(sample, sample_from_read));

  tps_destroy();
  return NULL;
}

int main() {
  sem1 = sem_create(0);
  sem2 = sem_create(0);

  pthread_t tid;
  tps_init(0);

  pthread_create(&tid, NULL, (void*)test_write_and_read, NULL);
  pthread_join(tid, NULL);


  sem_destroy(sem1);
  sem_destroy(sem2);

  return 0;
}
