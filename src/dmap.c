#include "dmap.h"

/* Simple Index Calculation */
static uint32_t get_idx(uint32_t key) {
    return key % MAX_ENTRIES;
}

void dmap_put(dmap *dm, uint32_t key, uint8_t *val) {
    uint32_t idx = get_idx(key);
    
    /* Just overwrite whatever was there */
    dm->table[idx].key = key;
    dm->table[idx].valid = 1;
    memcpy(dm->table[idx].value, val, VALUE_SIZE);
}

int dmap_get(dmap *dm, uint32_t key, uint8_t *out) {
    uint32_t idx = get_idx(key);
    
    /* Check if the key matches AND the slot is valid */
    if (dm->table[idx].valid && dm->table[idx].key == key) {
        memcpy(out, dm->table[idx].value, VALUE_SIZE);
        return 1;
    }
    return 0; /* Miss! */
}
