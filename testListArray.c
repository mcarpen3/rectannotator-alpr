// cl test_main.c parson.c linked_list.c ^
#include <stdio.h>
#include "linked_list.h"
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand(time(NULL));
    printf("Linked List Tests. Generating 5 Lists with 5 ListElmts, each containing a random integer.!\n");
    List *buckets = (List *)malloc(sizeof(List) * 5);
    for (int i = 0; i < 5; ++i) {
        list_init(&buckets[i], free);
        for (int j = 0; j < 5; ++j) {
            int *ldata = (int *)malloc(sizeof(int));
            *ldata = rand() % 100;
            list_ins_next(&buckets[i], NULL, ldata); 
        }
    }
    for (int i = 0; i < 5; ++i) {
        ListElmt *cur = list_head(&buckets[i]);
        int idx = 0;
        while (cur != NULL) {
            printf("list[%d] item[%d] = %d\n", i, idx, *(int *)list_data(cur));
            cur = cur->next;
            idx++;
        }
        list_destroy(&buckets[i]);
    }
}