# Report of the Semaphore and TPS API


The project aims to simulate the behaviors of Semaphore to synchronize behaviors between threads and TPS-thread private storage which gives threads ownership of memory.

# Semaphore

##  Framework and Data Structure
 We define a `struct` called the semaphore which contains the 2 critical components as follows: 
`size_t count`
`queue_t block_list`
Count indicates how many units are available, and the block_list contains all tid which  are blocked due to the shortage of the resource. The block list works as a queue.


## `sem_create(size_t count)`
In this function, We need to initialize the semaphore structure here. It is very straightforward. We just use `queue_create()` to create the block_list and set the `count` as the initial value of the `count` which indicates how many resources are available initially. 

## `sem_destory()`
Here, we first determine if the `sem` is `NULL` and the `block_list` still contains elements. If so, we need to return -1. Otherwise, we need to destroy the semaphore structure. We use the `queue_destroy()` to destroy the `block_list` and then free `sem`. We free the `sem` because the `sem` is also created by `malloc()`. 


##  `sem_down(sem_t sem)`
When the function is called, the thread decreases the available resource. If there are no resources available (when count != 0), we block the current thread from running through the interface provided thread library. The thread `tid` is added to the blocked list, and until `sem_up()` is called and the thread is selected to unblock, the thread should remain blocked. Also, else_if structure ensures the count will never be below zero.

## `sem_up(sem_t sem)`
The `sem_up()` is releasing the available source to the next thread. What we are doing here is checking if the `block_list` still contains elements. If there is, we use the `queue_dequeue()` to return the oldest blocked thread from the `block_list` and unblock this thread with `thread_unblock()`. If there is not, we add `count` by one, because there will be one more available source in semaphore.

## `sem_getvalue()`
The function returns how many resources are available. If the internal count is 0, which means there is a non-negative count waiting block queue. The function will return a non positive number which shows how many threads are waiting for resources. If the internal count is positive the function assign this number to the retval. 


# TPS API
## Frame and Data Structure

Inspired by `std::shared_ptr` we define two structure as follow:

```
typedef struct count_storage_class {
  void *storage;
  size_t count;
} count_storage_class;

typedef struct tps_page {
  pthread_t tid;
  count_storage_class *count_storage;
} tps_page;
```
The ``count_storage_class`` contains an actual page and a counter which number shows how many threads are pointed to the count_storage unit. This kind of design gives the programm page sharing feature among threads because multiple tps pages can point to the same storage unit.  

### e.g
 tps_page 1--> CSC1
tps_page 2 --> CSC1
tps_page 3 --> CSC1

And count is three now.




## `tps_init(int segv)`
In this function, the point is we need to set the page fault handler here to recognize the `SIGBUS` and `SIGSEGV`. We have already been given the skeleton setup function and we need to finish the signal-catching function `segv_handler()`. In the `segv_handler()`, we have known the `p_fault` address of the fault page, so we have to find out if this address is in my memory pages or not. If it is, that means the fault is caused by the protection. Otherwise, it is because of a real programming error. Therefore, we need `queue_iterate` to go through all the pages and check, so we need another function to compare the `p_fault` and the current page’s address. If the `p_fault` matches with one page’s address, we output a `stderr` to tell the user this is a protection error. If there is no match, there will be a real segmentation fault.

## `tps_create()`
Before we create a TPS for a thread, we need to check if this thread has already had its own TPS, so we use `queue_iterate` to go through all the pages stored in `tps_queue` and find if the current `tid` exists in queue. If this `tid` is found in the queue, we do not create a new page. Only when it does not have, do we create a TPS for it. First, we get the `tid` of the current running thread and store it in the page structure. Then in the `count_storage`, we use `mmap()` to set a new memory page for the running thread and store it in `count_storage->storage`. Also, we store a count in the `count_storage->count` for counting the cloned pages. 

## `tps_destroy()`
Before destroying a TPS, we also need to go through all the pages in `tps_queue` to find out if the current `tid` exists in queue. If there is no `tid` that we want to destroy, that means this thread actually does not have a TPS right now, so we return -1 to tell the user destroy fails. If we find out this `tid` in queue, we can destroy it in the next step. However, we cannot immediately delete the TPS from the queue, because if there are some other thread also points to this TPS (e.g `tps_clone`), those thread will also lose their TPS. Therefore, we have to check if the TPS is shared by more than one thread, using the `count_storage->count`. If the count is more than 1, we can destroy the TPS by `munmap()`. If it is more than one, we need to just delete this one memory page by `queue_delete()`.

## `tps_read()`
Before we read from the memory page, we need to find the page with a specific `tid` like we did in the previous function. If the page exists, we can take the next step. We need to determine if the reading size is out of the page’s bound (`TPS_SIZE`) and if the buffer is null. Then, we can do the reading using the `memcpy()`, because this function can copy from a source file (the beginning of the memory page) a specific length (`length`) from a specific position (`offset`) to a destination file (`buffer`). That is just what we need here.

## `tps_write()`
In the `tps_write()`, we almost do the same thing, check if `tid` exists, if the page size after writing will over the `TPS_SIZE` and if the buffer is NULL. Then, we use the `memcpy()` to write the data in `buffer` to the memory page from the position `offset`. The most different point from the `tps_read()` is that we also need to check if the current thread shares this memory page with other threads. If there are more than one thread using this one page, we need to copy-on-write. 
