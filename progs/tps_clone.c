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
static char sample[TPS_SIZE] = "hello";
static char sample_2[TPS_SIZE] = "hello Winter";
static pthread_t tid;

void *test_clone(){
  char buffer[TPS_SIZE];            
  char write_to_memory[] = " Winter";

  /*clone previous thread tps and test the memoery page*/
  tps_clone(tid);
  tps_read(0, TPS_SIZE, buffer);
  TEST_ASSERT(!memcmp(sample, buffer,TPS_SIZE));

  /*write on the current tps's memory page*/
  tps_write(5, 7, write_to_memory);
  tps_read(0, TPS_SIZE, buffer);
  TEST_ASSERT(!memcmp(sample_2, buffer,TPS_SIZE));
  
  tps_destroy();
  sem_up(sem1);
  
  return NULL;
}
void *test_write_and_read() {  	
  pthread_t tid_clone;
  tps_create();
  char sample_from_read[TPS_SIZE];

  /*write "hello" in the memory page and read*/
  tps_write(0, TPS_SIZE, sample);                 
  tps_read(0, TPS_SIZE, sample_from_read);
  TEST_ASSERT(!memcmp(sample, sample_from_read, TPS_SIZE));
  
  pthread_create(&tid_clone, NULL, (void*) test_clone, NULL);
 
  /*block this thread and go to test_clone()*/
  sem_down(sem1);                                        
  
  /*Wait for test_clone die, and check if the tps of current thread are modified*/
  pthread_join(tid_clone, NULL);
   
  
  tps_read(0, TPS_SIZE, sample_from_read);
  TEST_ASSERT(!memcmp(sample, sample_from_read, TPS_SIZE));

  tps_destroy();
  return NULL;
}

int main() {
  sem1 = sem_create(0);
  sem2 = sem_create(0);

  tps_init(0);
  
  /*go to the teat_write_read()*/
  pthread_create(&tid, NULL, (void*)test_write_and_read, NULL);
  pthread_join(tid, NULL);


  sem_destroy(sem1);
  sem_destroy(sem2);

  return 0;
}
