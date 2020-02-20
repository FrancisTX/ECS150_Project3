#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
    size_t count;          //for counting the available resources
    queue_t block_list;    //the queue to store the waiting thread
};

sem_t sem_create(size_t count)
{

    struct semaphore* sem;

    /*create a space for sem in the heap */
    sem = (struct semaphore*) malloc(sizeof(struct semaphore));

    if (sem == NULL){
        return NULL;
    }

    /*initialize the sem*/
    sem->count = count;
    sem->block_list = queue_create();

    return sem;
}

int sem_destroy(sem_t sem) {
    
    /*destroy the sem and free the space on the heap*/
    if (sem == NULL || queue_length(sem->block_list) > 0) {
        exit_critical_section();
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
        exit_critical_section();
        return -1;
    }

    /*
     *if there is no resource, block the current thread
     *if there is, count minus one
     */
    if (sem->count == 0) {
          enter_critical_section();
          queue_enqueue(sem->block_list, (void*)pthread_self());
          thread_block();
          exit_critical_section();
    }else     
          sem->count--;
  
    exit_critical_section();
    return 0;
}

int sem_up(sem_t sem) {
  enter_critical_section();
  if (!sem) {
      exit_critical_section();
      return -1;
  } 

  /*
   *if there is waiting thread in the queue, unblock it
   *if there is not, count plus one
   */
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
  enter_critical_section();

  if(sem == NULL || sval == NULL){
     exit_critical_section();
     return -1;
  }
  
   /*
   *if there are available resources, return the count
   *if there is not, return negative length of the queue
   */

  if (sem->count > 0){
     *sval = sem->count;
  }else
     *sval = -queue_length(sem->block_list);
  
  exit_critical_section();
  return 0;

}

