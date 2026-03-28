#include "dmap.h"

static uint32_t get_idx(uint32_t key) {
    return key % MAX_ENTRIES;
}

static int is_key_occupied(dmap *dm, uint32_t key) {
    uint32_t idx = get_idx(key);
    return (dm->table[idx].valid && dm->table[idx].key == key) ? 1 : 0;
}

int dmap_set_is_dirty(dmap *dm, uint32_t key, int is_dirty) {
    uint32_t idx = get_idx(key);
    
    if (is_key_occupied(dm, key) == 1) {
        dm->table[idx].is_dirty = is_dirty;
        return 0;
    }
    return 1;
}

int dmap_get(dmap *dm, uint32_t key, uint8_t *out) {
    uint32_t idx = get_idx(key);
    
    if (is_key_occupied(dm, key) == 1) {
        memcpy(out, dm->table[idx].value, VALUE_SIZE);
        return 0;
    }
    return 1; 
}

int dmap_put(dmap *dm, uint32_t key, uint8_t *val, int (*callback)(uint32_t, uint8_t *), uint8_t* callback_buffer) {
    uint32_t idx = get_idx(key);
    
    /* Evict existing dirty page if there's a collision */
    if (dm->table[idx].valid == 1 && dm->table[idx].is_dirty == 1) {
        /* Pass the old key to the callback so it writes to the correct disk location */
        uint32_t old_key = dm->table[idx].key;
        memcpy(callback_buffer, dm->table[idx].value, VALUE_SIZE);
        if (callback(old_key, callback_buffer) != 0) {
            return 1;
        }
    }
    
    dm->table[idx].key = key;
    dm->table[idx].valid = 1;
    dm->table[idx].is_dirty = 0;
    
    if (val != NULL) {
        memcpy(dm->table[idx].value, val, VALUE_SIZE);
    }
    return 0;
}
