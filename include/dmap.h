#ifndef DIRECT_MAP_H
#define DIRECT_MAP_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "db_global.h"

#define MAX_ENTRIES DB_FRAME_SIZE
#define VALUE_SIZE DB_PAGE_SIZE

typedef struct {
    uint32_t key;
    uint8_t value[VALUE_SIZE];
    int valid; /* 0 if empty, 1 if contains data */
} dmap_slot;

typedef struct {
    dmap_slot table[MAX_ENTRIES];
} dmap;

void dmap_put(dmap *dm, uint32_t key, uint8_t *val);
int dmap_get(dmap *dm, uint32_t key, uint8_t *out);

#endif /* DIRECT_MAP_H */
