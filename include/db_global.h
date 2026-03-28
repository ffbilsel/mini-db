#ifndef DB_GLOBAL_H
#define DB_GLOBAL_H

#include <stdint.h>

#define DB_PAGE_SIZE 8192
#define DB_MAP_SIZE 1024

typedef struct PageNode {
    uint8_t page[DB_PAGE_SIZE];
    uint32_t page_id;
    int is_new;
    struct PageNode* next;
} PageNode;

extern uint32_t db_page_count;

#endif  /* DB_GLOBAL_H */
