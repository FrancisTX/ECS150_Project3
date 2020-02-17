#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"

/* TODO: Phase 2 */

queue_t tps_queue;

typedef struct count_storage_class {
  void *storage;
  size_t count;
} count_storage_class;

typedef struct tps_page {
  pthread_t tid;
  count_storage_class *count_storage;
} tps_page;

static int find_page(void *data, void *arg) {
  tps_page *current_page = (tps_page *) data;
  pthread_t match = (pthread_t) arg;

  if (current_page->tid == match)
    return 1;

  return 0;
}

int tps_init(int segv) {
  /* TODO: Phase 2 */
}

int tps_create(void) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (target) {
    //find the replicate
    return -1;
  }
  tps_page *page = malloc(sizeof(tps_page));
  page->tid = pthread_self();
  page->count_storage = malloc(sizeof(count_storage_class));
  page->count_storage->storage = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, 1, 0);
  page->count_storage->count = 1;
  queue_enqueue(tps_queue, (void *) page);
  exit_critical_section();
  return 0;
}

int tps_destroy(void) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (!target) {
    return -1;
  }
  if (target->count_storage->count == 1) {
    munmap(target->count_storage->storage, TPS_SIZE);
    free(target->count_storage);
  }
  queue_delete(tps_queue, pthread_self());
  exit_critical_section();
  return 0;
}

int tps_read(size_t offset, size_t length, void *buffer) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
    return -1;
  }
  mprotect(target->count_storage->storage, TPS_SIZE, PROT_READ);
  memcpy(buffer, target->count_storage->storage + offset, length);
  mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
  exit_critical_section();
  return 0;
}

int tps_write(size_t offset, size_t length, void *buffer) {
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
    return -1;
  }

  if (target->count_storage->count > 1) {
    count_storage_class *new_count_storage = malloc(sizeof(count_storage_class));
    new_count_storage->storage = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, 1, 0);
    new_count_storage->count = 1;
    mprotect(new_count_storage->storage, TPS_SIZE, PROT_WRITE);
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_READ);
    memcpy(new_count_storage->storage, target->count_storage->storage, TPS_SIZE);
    mprotect(new_count_storage->storage, TPS_SIZE, PROT_NONE);
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
    target->count_storage->count--;
    target->count_storage = new_count_storage;
  }
  mprotect(target->count_storage->storage, TPS_SIZE, PROT_WRITE);
  memcpy(target->count_storage->storage + offset, buffer, length);
  mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
  exit_critical_section();
  return 0;
}

int tps_clone(pthread_t tid) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *self = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &self);

  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, tid, (void **) &target);

  if (self) {
    return -1;
  }
  if (!target) {
    return -1;
  }
  tps_page *page = malloc(sizeof(tps_page));
  page->tid = pthread_self();
  page->count_storage = malloc(sizeof(count_storage_class));
  page->count_storage = target->count_storage;
  page->count_storage->count++;
  queue_enqueue(tps_queue, (void *) page);
  exit_critical_section();
  return 0;
}

