#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
#include <sem.h>

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

void *test_clone(pthread_t clone_tid){                  
  char buffer[20];             
  char write_to_memory[] = " Winter";

  /*clone "hello ECS_150" and read it*/
  tps_clone(clone_tid);
  tps_read(0, 12, buffer);
  TEST_ASSERT(!memcmp("hello ECS_150", buffer, 12));

  /*write " Winter" behind and read it*/
  tps_write(13, strlen(write_to_memory), write_to_memory);
  tps_read(0, 19, buffer);
  TEST_ASSERT(!memcmp("hello ECS_150 Winter", buffer,19));

  return NULL;
}

void *test_write_and_read_offset() {                   
  pthread_t tid;
  tps_create();
  char sample[TPS_SIZE] = "hello ECS_150\n";
  char sample_from_read[7];
  
  /*write "hello ECS_150" into the mem page and read it fore offset 6*/ 
  tps_write(0, TPS_SIZE, sample);
  tps_read(6, 7, sample_from_read);
  TEST_ASSERT(!memcmp("ECS_150", sample_from_read, 7));

  /*go to test_clone*/
  pthread_create(&tid, NULL, (void*)test_clone, (void *)pthread_self());
  pthread_join(tid, NULL);

  return NULL;
}

void *test_write_and_read() {                          
  pthread_t tid;
  tps_create();
  char sample[TPS_SIZE] = "hello\n";
  char sample_from_read[TPS_SIZE];

  /*write "hello" and read it from mem page*/
  tps_write(0, TPS_SIZE, sample);
  tps_read(0,TPS_SIZE, sample_from_read);
  TEST_ASSERT(!memcmp(sample, sample_from_read,TPS_SIZE));
 
  /*go to test_write_and_read_offset*/
  pthread_create(&tid, NULL, (void*)test_write_and_read_offset, NULL);
  pthread_join(tid, NULL);

  
  return NULL;
}

int main() {
  pthread_t tid;
  tps_init(0);

  /*go the test_write_and_read*/
  pthread_create(&tid, NULL, (void*)test_write_and_read, NULL);
  pthread_join(tid, NULL);

  return 0;
}
