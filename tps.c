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

static int find_fault_address(void* fault_address, void* page){
    tps_page *current_page = (tps_page *) page;

    if (fault_address == current_page->count_storage->storage){
        return 1;
    }
    return 0;
}

static void segv_handler(int sig, siginfo_t *si,  __attribute__((unused)) void *context)
{
    tps_page *current_page= NULL;
    /*
     * Get the address corresponding to the beginning of the page where the
     * fault occurred
     */
    void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));

    /*
     * Iterate through all the TPS areas and find if p_fault matches one of them
     */

    queue_iterate(tps_queue,find_fault_address, p_fault, (void**)&current_page);

    if (current_page)
        /* Printf the following error message */
        fprintf(stderr, "TPS protection error!\n");

    /* In any case, restore the default signal handlers */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    /* And transmit the signal again in order to cause the program to crash */
    raise(sig);
}

static int find_page(void *data, void *arg) {
  tps_page *current_page = (tps_page *) data;
  pthread_t match = (pthread_t) arg;

  if (current_page->tid == match)
    return 1;

  return 0;
}

int tps_init(int segv) {
  /* TODO: Phase 2 */
    if (tps_queue)
        return -1;

    tps_queue = queue_create();
    if (!tps_queue)
        return -1;

    if (segv) {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = segv_handler;
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGSEGV, &sa, NULL);
    }

    return 0;
}

int tps_create(void) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, (void*)pthread_self(), (void **) &target);
  if (target) {
    //find the replicate
    return -1;
  }
  tps_page *page = malloc(sizeof(tps_page));
  page->tid = pthread_self();
  page->count_storage = malloc(sizeof(count_storage_class));
  page->count_storage->storage = mmap(NULL, TPS_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 1, 0);
  page->count_storage->count = 1;
  queue_enqueue(tps_queue, (void *) page);
  exit_critical_section();
  return 0;
}

int tps_destroy(void) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, (void*)pthread_self(), (void **) &target);
  if (!target) {
    return -1;
  }
  if (target->count_storage->count == 1) {
    munmap(target->count_storage->storage, TPS_SIZE);
    free(target->count_storage);
  }
  queue_delete(tps_queue, (void**)pthread_self());
  exit_critical_section();
  return 0;
}

int tps_read(size_t offset, size_t length, void *buffer) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, (void*)pthread_self(), (void **) &target);
  if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
    return -1;
  }
  memcpy(buffer, target->count_storage->storage + offset, length);
  exit_critical_section();
  return 0;
}

int tps_write(size_t offset, size_t length, void *buffer) {
  enter_critical_section();
  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, (void*)pthread_self(), (void **) &target);
  if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
    return -1;
  }

  if (target->count_storage->count > 1) {
    count_storage_class *new_count_storage = malloc(sizeof(count_storage_class));
    new_count_storage->storage = mmap(NULL, TPS_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 1, 0);
    new_count_storage->count = 1;

    memcpy(new_count_storage->storage, target->count_storage->storage, TPS_SIZE);

    target->count_storage->count--;
    target->count_storage = new_count_storage;
  }
  memcpy(target->count_storage->storage + offset, buffer, length);
  exit_critical_section();
  return 0;
}

int tps_clone(pthread_t tid) {
  /* TODO: Phase 2 */
  enter_critical_section();
  tps_page *self = NULL;
  queue_iterate(tps_queue, find_page, (void*)pthread_self(), (void **) &self);

  tps_page *target = NULL;
  queue_iterate(tps_queue, find_page, (void*)tid, (void **) &target);

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

