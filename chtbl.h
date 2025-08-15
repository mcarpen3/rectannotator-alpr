#ifndef CHTBL_H
#define CHTBL_H

#include <stdlib.h>

#include "linked_list.h"

// Define a structure for chanined hash tables

typedef struct CHTbl_ {
    int buckets;

    int (*h)(const void *key);
    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);

    int size;
    List *table;
} CHTbl;

// initialize the chained hash table with buckets number of buckets. Runs in O(buckets) time since each bucket is a linked list with init O(1) time
int chtble_init(CHTbl *htbl, int buckets, int (*h)(const void *key), int (*match)(const void *key1, const void *key2), void (*destroy)(void *data));

// destroy the chained hash table. The destroy function passed to chtbl_init is called for each element in each bucket.
// Each bucket could have a number of elements equal to the "load factor" which is considered a small constant
// Runs in O(buckets) time.
void chtbl_destroy(CHTbl *htbl);

// O(1) runtime to lookup, hash, and insert into linked-list
int chtbl_insert(CHTbl *htbl, const void *data);

// O(1) runtime to lookup, 
int chtbl_remove(CHTbl *htbl, void **data);

int chtbl_lookup(const CHTbl *htbl, void **data);

#define chtbl_size(htbl) ((htbl)->size)

#endif