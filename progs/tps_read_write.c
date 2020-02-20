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
  char buffer[20];             //clone the thread whose tid is 1 
  char write_to_memory[] = " Winter";

  tps_clone(clone_tid);
  tps_read(0, 12, buffer);
  TEST_ASSERT(!memcmp("hello ECS_150", buffer, 12));

  tps_write(13, strlen(write_to_memory), write_to_memory);
  tps_read(0, 19, buffer);
  TEST_ASSERT(!memcmp("hello ECS_150 Winter", buffer,19));

  return NULL;
}

void *test_write_and_read_offset() {                     //tid[2]
  pthread_t tid;
  tps_create();
  char sample[TPS_SIZE] = "hello ECS_150\n";
  char sample_from_read[7];
  tps_write(0, TPS_SIZE, sample);
  tps_read(6, 7, sample_from_read);
  TEST_ASSERT(!memcmp("ECS_150", sample_from_read, 7));

  pthread_create(&tid, NULL, (void*)test_clone, (void *)pthread_self());
  pthread_join(tid, NULL);

  return NULL;
}

void *test_write_and_read() {                             //tid[1]
  pthread_t tid;
  tps_create();
  char sample[TPS_SIZE] = "hello\n";
  char sample_from_read[TPS_SIZE];
  tps_write(0, TPS_SIZE, sample);
  tps_read(0,TPS_SIZE, sample_from_read);
  TEST_ASSERT(!memcmp(sample, sample_from_read,TPS_SIZE));

  pthread_create(&tid, NULL, (void*)test_write_and_read_offset, NULL);
  pthread_join(tid, NULL);

  
  return NULL;
}

int main() {
  pthread_t tid;
  tps_init(0);

  pthread_create(&tid, NULL, (void*)test_write_and_read, NULL);
  pthread_join(tid, NULL);

  return 0;
}
