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

int sem_destroy(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_down(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_up(sem_t sem)
{
	/* TODO Phase 1 */
}

int sem_getvalue(sem_t sem, int *sval)
{
	/* TODO Phase 1 */
}

