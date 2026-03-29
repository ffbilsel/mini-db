#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdint.h>
#include "db_global.h"

#define HASH_TABLE_SIZE 2048 /* Larger than pool size to reduce collisions */

typedef struct HashNode {
    uint32_t page_id;
    int frame_id;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *table[HASH_TABLE_SIZE];
} PageTable;

void pt_init(PageTable *pt);
int pt_get(PageTable *pt, uint32_t page_id);
int pt_put(PageTable *pt, uint32_t page_id, int frame_id);
void pt_remove(PageTable *pt, uint32_t page_id);
void pt_clear(PageTable *pt);

#endif /* PAGE_TABLE_H */
