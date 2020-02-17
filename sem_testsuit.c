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

void test_write_and_read() {
  tps_create();
  char sample[] = "hello";
  char sample_from_read[strlen(sample)];
  tps_write(0, strlen(sample), sample);
  tps_read(0, strlen(sample), sample_from_read);
  TEST_ASSERT(strcmp(sample, sample_from_read));
}

int main() {
  pthread_t tid[100];
  tps_init();
  pthread_create(&tid[1], NULL, test_write_and_read, NULL);

}




