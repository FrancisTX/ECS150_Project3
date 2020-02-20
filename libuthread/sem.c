#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
	/* TODO Phase 1 */
    size_t count;
    queue_t block_list;
};

sem_t sem_create(size_t count)
{
	/* TODO Phase 1 */

    struct semaphore* sem;
    sem = (struct semaphore*) malloc(sizeof(struct semaphore));

    if (sem == NULL){
        return NULL;
    }

    sem->count = count;
    sem->block_list = queue_create();

    return sem;
}

int sem_destroy(sem_t sem) {
    /* TODO Phase 1 */
    if (sem == NULL || queue_length(sem->block_list) > 0) {
        return -1;
    } else {
        queue_destroy(sem->block_list);
        free(sem);
    }

	return 0;
}

int sem_down(sem_t sem)
{
    enter_critical_section();
    if (!sem) {
      return -1;
    }
  if (sem->count == 0) {
    queue_enqueue(sem->block_list, (void*)pthread_self());
    thread_block();
  } else {
    sem->count--;
  }
  exit_critical_section();
    return 0;
}

int sem_up(sem_t sem) {
  /* TODO Phase 1 */
  enter_critical_section();
  if (!sem) {
    exit_critical_section();
    return -1;
  }
  if (queue_length(sem->block_list) != 0) {
    pthread_t tmp;
    queue_dequeue(sem->block_list, (void **) &tmp);
    thread_unblock(tmp);
  } else {
    sem->count++;
  }

  exit_critical_section();
  return 0;
}

int sem_getvalue(sem_t sem, int *sval)
{
	/* TODO Phase 1 */
  enter_critical_section();
    if(sem == NULL || sval == NULL){
        return -1;
    }
    if (sem->count > 0){
        *sval = sem->count;
    }else
        *sval = queue_length(sem->block_list);
  exit_critical_section();
    return 0;

}

