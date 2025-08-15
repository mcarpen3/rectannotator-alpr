#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
#include "chtbl.h"

// Define a structure for chanined hash tables

// typedef struct CHTbl_ {
//     int buckets;

//     int (*h)(const void *key);
//     int (*match)(const void *key1, const void *key2);
//     void (*destroy)(void *data);

//     int size;
//     int *table;
// } CHTbl;

// initialize the chained hash table with buckets number of buckets. Runs in O(buckets) time since each bucket is a linked list with init O(1) time
int chtble_init(CHTbl *htbl, int buckets, int (*h)(const void *key), int (*match)(const void *key1, const void *key2), void (*destroy)(void *data)) {
    int i;
    if ((htbl->table = (List *)malloc(buckets * sizeof(List))) == NULL) return -1;

    // initialize the buckets
    htbl->buckets = buckets;
    for (i = 0; i < htbl->buckets; i++) {
        list_init(&htbl->table[i], destroy);
    }
    // encapsulate the functions
    htbl->h = h;
    htbl->match = match;
    htbl->destroy = destroy;
    
    // initialize the number of elements in the list
    htbl->size = 0;
    return 0;
}

// destroy the chained hash table. The destroy function passed to chtbl_init is called for each element in each bucket.
// Each bucket could have a number of elements equal to the "load factor" which is considered a small constant
// Runs in O(buckets) time.
void chtbl_destroy(CHTbl *htbl) {
    int i;
    for (i = 0; i < htbl->buckets; i++) {
        list_destroy(&htbl->table[i]);
    }

    // free the storage allocated for hash_table
    free(htbl->table);
    // clear the structure as a precaution
    memset(htbl, 0, sizeof(CHTbl));
    return;
}

// O(1) runtime to lookup, hash, and insert into linked-list
int chtbl_insert(CHTbl *htbl, const void *data) {
    void *temp;
    int bucket, retval;

    temp = (void *)data;

    // if data is already in the table do nothing
    if (chtbl_lookup(htbl, &temp) == 0) return 1;

    // hash the key
    bucket = htbl->h(data) % htbl->buckets;
    // insert the data into the bucket
    if ((retval = list_ins_next(&htbl->table[bucket], NULL, data)) == 0) htbl->size++;
    return retval;
}

// O(1) runtime to lookup, 
int chtbl_remove(CHTbl *htbl, void **data) {
    ListElmt *element, *prev;
    int bucket;

    // hash the key
    bucket = htbl->h(*data) % htbl->buckets;
    // search for the data in the bucket
    prev = NULL;
    for (element = list_head(&htbl->table[bucket]); element != NULL; element = list_next(element)) {
        if (htbl->match(*data, list_data(element))) {
            // remove the data from the bucket
            if (list_rem_next(&htbl->table[bucket], prev, data) == 0) {
                htbl->size--;
                return 0;
            } else {
                return -1;
            }
        }
        prev = element;
    }
    return -1;
}

int chtbl_lookup(const CHTbl *htbl, void **data) {
    ListElmt *element;
    int bucket;

    // hash the key
    bucket = htbl->h(*data) % htbl->buckets;

    // search for the data in the bucket
    for (element = list_head(&htbl->table[bucket]); element != NULL; element = list_next(element)) {
        if (htbl->match(*data, list_data(element))) {
            // pass back the data from the table
            *data = list_data(element);
            return 0;
        }
    }
    return -1;
}