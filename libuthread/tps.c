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

queue_t tps_queue;         //the queue for storing the tps

typedef struct count_storage_class {
    void *storage;         //for stroing the memory page
    size_t count;          //count the threads pointing to this page
}count_storage_class;

typedef struct tps_page { 
    pthread_t tid;         //the threads tid
    count_storage_class *count_storage;    //structure of mem page 
} tps_page;

static int find_page(void *data, void *arg) {
    /*find if the page exist in the queue*/
    tps_page *current_page = (tps_page *) data;
    pthread_t match = (pthread_t) arg;

    if (current_page->tid == match)
        return 1;

    return 0;
}

static int find_fault_address(void* page, void* fault_address){
    
    tps_page *current_page = (tps_page *) page;
    void* address = fault_address;
    /*find the page that get protection error*/
    if (address == current_page->count_storage->storage){
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
    queue_iterate(tps_queue,find_fault_address,  (void*)p_fault , (void**)&current_page);

    if (current_page != NULL){
        /* Printf the following error message */
        fprintf(stderr, "TPS protection error!\n");
    }

    /* In any case, restore the default signal handlers */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    /* And transmit the signal again in order to cause the program to crash */
    raise(sig);
}

int tps_init(int segv) {
    if (tps_queue)
        return -1;

    /*create the queue for tps*/
    tps_queue = queue_create();
    if (!tps_queue)
        return -1;

    /*set up a signal handler*/
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
    enter_critical_section();

    /*check if the tps have already in the queue*/
    tps_page *target = NULL;
    queue_iterate(tps_queue, find_page, (void *)pthread_self(), (void **) &target);
    if (target) {
        exit_critical_section();
        return -1;
    }
    
    /*create the space for the tps*/
    tps_page *page = malloc(sizeof(tps_page));

    /*initialize the tps*/
    page->tid = pthread_self();
    page->count_storage = malloc(sizeof(count_storage_class));
    page->count_storage->storage = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, 1, 0);
    page->count_storage->count = 1;

    /*put the tps into the tps_queue*/
    queue_enqueue(tps_queue, (void *) page);
    exit_critical_section();
    return 0;
}

int tps_destroy(void) {
    enter_critical_section();

    /*check if the tps exists in the queue*/
    tps_page *target = NULL;
    queue_iterate(tps_queue, find_page, (void *)pthread_self(), (void **) &target);
    if (!target) {
        exit_critical_section();
        return -1;
    }

    /*
     *if there are just one thread points to this tps, destroy it
     *it there are more than one, delete it in the queue
     */
    if (target->count_storage->count == 1) {
        munmap(target->count_storage->storage, TPS_SIZE);
        free(target->count_storage);
    }
    queue_delete(tps_queue, (void **)pthread_self());
    exit_critical_section();
    return 0;
}

int tps_read(size_t offset, size_t length, void *buffer) {
 
    enter_critical_section();

    /*find the tps want to be read in the queue*/
    tps_page *target = NULL;
    queue_iterate(tps_queue, find_page, (void *)pthread_self(), (void **) &target);

    /*
     *check if the size are enough for reading
     *check if the tps exists in the queue
     *check if the buffer is null
     */
    if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
        exit_critical_section();
        return -1;
    }

    /*change the mem protection and read*/
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_READ);
    memcpy(buffer, target->count_storage->storage + offset, length);
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
    exit_critical_section();
    return 0;
}

int tps_write(size_t offset, size_t length, void *buffer) {
    enter_critical_section();

    /*find the tps want to be read in the queue*/
    tps_page *target = NULL;
    queue_iterate(tps_queue, find_page, (void *)pthread_self(), (void **) &target);

    /*
     *check if the size are enough for writing
     *check if the tps exists in the queue
     *check if the buffer is null
     */
    if (offset + length > TPS_SIZE || target == NULL || buffer == NULL) {
        exit_critical_section();
        return -1;
    }
    
    /*
     *if there are more than one thread sharing the tps, 
     *create a new page for this thread
     */
    if (target->count_storage->count > 1) {
        /*create a new space for the tps*/
        count_storage_class *new_count_storage = malloc(sizeof(count_storage_class));

        /*initialize the tps*/
        new_count_storage->storage = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_ANON | MAP_PRIVATE, 1, 0);
        new_count_storage->count = 1;

        /*
         *change the mem proptection
         *write to the mem page
         */
        mprotect(new_count_storage->storage, TPS_SIZE, PROT_WRITE);
        mprotect(target->count_storage->storage, TPS_SIZE, PROT_READ);
        
        memcpy(new_count_storage->storage, target->count_storage->storage, TPS_SIZE);

        mprotect(new_count_storage->storage, TPS_SIZE, PROT_NONE);
        mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
        
        /*sharing thread decrease by one*/
        target->count_storage->count--;
        target->count_storage = new_count_storage;
    }

    /*
     *if there is no sharing thread
     *change protection and write on the mem page immediately
     */
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_WRITE);
    memcpy(target->count_storage->storage + offset, buffer, length);
    mprotect(target->count_storage->storage, TPS_SIZE, PROT_NONE);
    exit_critical_section();
    return 0;
}

int tps_clone(pthread_t tid) {
  
    enter_critical_section();
    /*find the self tps*/
    tps_page *self = NULL;
    queue_iterate(tps_queue, find_page, (void *)pthread_self(), (void **) &self);

    /*find the tps cloned by current thread*/
    tps_page *target = NULL;
    queue_iterate(tps_queue, find_page, (void *)tid, (void **) &target);

    /*check if both of seld tps and tps cloned are exist*/
    if (self || !target) {
        exit_critical_section();
        return -1;
    }
  
    tps_page *page = malloc(sizeof(tps_page));

    /*initialize the cloned mem page*/
    page->tid = pthread_self();
    page->count_storage = malloc(sizeof(count_storage_class));
    page->count_storage = target->count_storage;
    page->count_storage->count++;

    /*put it in tps queue*/
    queue_enqueue(tps_queue, (void *) page);
    exit_critical_section();
    return 0;
}

