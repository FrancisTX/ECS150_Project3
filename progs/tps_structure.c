#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
#include <sem.h>

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
                         
void *test_write_and_read_struct(){
    struct test_struct {
    int t1;
    long t2;
    char* t3;
    char t4;
    };
    struct test_struct* to_write = malloc(sizeof(struct test_struct));
    struct test_struct* to_read = malloc(sizeof(struct test_struct));
    to_write->t1 = 1;
    to_write->t2 = 2;
    to_write->t3 = malloc(sizeof(5));
    to_write->t4 = 'h';
    tps_create();
    tps_write(0, sizeof(struct test_struct),to_write);
    tps_read(0,sizeof(struct test_struct),to_read);

    TEST_ASSERT(to_read->t1 == to_write->t1);
    TEST_ASSERT(to_read->t2 == to_write->t2);
    TEST_ASSERT(to_read->t3 == to_write->t3);
    TEST_ASSERT(to_read->t4 == to_write->t4);
    free(to_write->t3);
    free(to_write);
    free(to_read);

    return NULL;
}


int main() {
    pthread_t tid;
    tps_init(0);

    /*go the test_write_and_read_struct*/
    pthread_create(&tid, NULL, (void*)test_write_and_read_struct, NULL);
    pthread_join(tid, NULL);

  return 0;
}
