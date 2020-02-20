#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sem.h"
#include "thread.h"
#include "queue.h"
#include "tps.h"

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

static char msg1[TPS_SIZE] = "Hello world!\n";
static sem_t sem1, sem2;

void* my_thread(){
      /* Create TPS */
      tps_create();

      /* Get TPS page address as allocated via mmap() */
      char *tps_addr = latest_mmap_addr;

      /* Cause an intentional TPS protection error */
      tps_addr[0] = 0;
      return NULL;
}

void* thread1(){
      pthread_t tid;
      // char *buffer = malloc(TPS_SIZE);

      /* Create TPS and initialize with *msg1 */
      tps_create();
      tps_write(0, TPS_SIZE, msg1);

      pthread_create(&tid, NULL, (void*)my_thread, NULL);
      pthread_join(tid, NULL);

      tps_destroy();
      // free(buffer);
      return NULL;      
}

int main(){
      sem1 = sem_create(0);
      sem2 = sem_create(0);

      pthread_t tid;
      tps_init(0);

      pthread_create(&tid, NULL, (void*)thread1, NULL);
      pthread_join(tid, NULL);


      sem_destroy(sem1);
      sem_destroy(sem2);

      return 0;
}
