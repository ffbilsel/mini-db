#ifndef DIRECT_MAP_H
#define DIRECT_MAP_H

#include "db_global.h"
#include <string.h>

#define MAX_ENTRIES DB_MAP_SIZE
#define VALUE_SIZE DB_PAGE_SIZE

typedef struct {
    uint32_t key;
    uint8_t value[VALUE_SIZE];
    int valid; 
    int is_dirty;
} dmap_slot;

typedef struct {
    dmap_slot table[MAX_ENTRIES];
} dmap;

int dmap_set_is_dirty(dmap *dm, uint32_t key, int is_dirty);
int dmap_get(dmap *dm, uint32_t key, uint8_t *out);

/* Callback now takes the page_id (key) so it can be written to disk */
int dmap_put(dmap *dm, uint32_t key, uint8_t *val, int (*callback)(uint32_t, uint8_t *), uint8_t* callback_buffer);

#endif /* DIRECT_MAP_H */
