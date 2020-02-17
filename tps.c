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
  bool is_clone;
  void *storage;
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

  /* TODO: find replicate*/
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (target) {
    return -1;
  }
  tps_page *page = malloc(sizeof(tps_page));
  page->tid = pthread_self();
  page->is_clone = false;
  page->storage = mmap(NULL, TPS_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 1, 0);
  queue_enqueue(tps_queue, (void *) page);
  return 0;
}

int tps_destroy(void) {
  /* TODO: Phase 2 */
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);
  if (!target) {
    return -1;
  }
  if (target->is_clone == false)
    munmap(target->count_storage.storage, TPS_SIZE);
  queue_delete(tps_queue, pthread_self());

  return 0;
}

int tps_read(size_t offset, size_t length, void *buffer) {
  /* TODO: Phase 2 */
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, pthread_self(), (void **) &target);

  if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
    return -1;
  }
  memcpy(buffer, target->storage + offset, length);
  return 0;
}

int tps_write(size_t offset, size_t length, void *buffer)
{
	/* TODO: Phase 2 */
}

int tps_clone(pthread_t tid)
{
	/* TODO: Phase 2 */
}

