#include "dmap.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

dmap test_map;
uint8_t test_callback_buffer[DB_PAGE_SIZE];
int callback_trigger_count = 0;
uint32_t last_evicted_key = 0;

/* Mock eviction callback */
int mock_eviction_callback(uint32_t key, uint8_t *buffer) {
    (void)buffer;
    callback_trigger_count++;
    last_evicted_key = key;
    return 0; /* Success */
}

void test_dmap_put_and_get(void) {
    uint8_t in_buf[DB_PAGE_SIZE];
    uint8_t out_buf[DB_PAGE_SIZE];
    memset(in_buf, 'A', DB_PAGE_SIZE);
    
    assert(dmap_put(&test_map, 5, in_buf, mock_eviction_callback, test_callback_buffer) == 0);
    assert(dmap_get(&test_map, 5, out_buf) == 0);
    assert(memcmp(in_buf, out_buf, DB_PAGE_SIZE) == 0);
}

void test_dmap_get_miss(void) {
    uint8_t out_buf[DB_PAGE_SIZE];
    /* Key 10 has not been inserted */
    assert(dmap_get(&test_map, 10, out_buf) == 1); 
}

void test_dmap_collision_eviction(void) {
    uint32_t colliding_key;
    
    uint8_t new_buf[DB_PAGE_SIZE];
    memset(new_buf, 'B', DB_PAGE_SIZE);
    
    /* Mark key 5 as dirty so it triggers eviction on collision */
    assert(dmap_set_is_dirty(&test_map, 5, 1) == 0);
    
    /* Calculate a key that collides with 5 (assuming MAX_ENTRIES is e.g. 1024) */
    colliding_key = 5 + MAX_ENTRIES;
    
    callback_trigger_count = 0;
    
    /* This put should collide with key 5, triggering the callback to evict key 5 */
    assert(dmap_put(&test_map, colliding_key, new_buf, mock_eviction_callback, test_callback_buffer) == 0);
    assert(callback_trigger_count == 1);
    assert(last_evicted_key == 5);
}

int main(void) {
    memset(&test_map, 0, sizeof(dmap));
    
    test_dmap_put_and_get();
    test_dmap_get_miss();
    test_dmap_collision_eviction();
    
    printf("DMAP - All assertions passed!\n");
    return 0;
}
