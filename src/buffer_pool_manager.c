#include "buffer_pool_manager.h"

dmap* pm_dmap = NULL;

int pm_init(void) {
    if(pm_dmap == NULL) {
        pm_dmap = (dmap *)malloc(sizeof(dmap));
        if (pm_dmap) {
            return 0;
        }
    }
    return 1;
}

uint8_t* pm_fetch_frame(uint32_t page_id, uint8_t *out_buffer) {
    int status = dmap_get(pm_dmap, page_id, out_buffer);
    if(status == 0) {
        status = dm_page_read((char *)out_buffer, page_id);
        if(status == 1) {
            dmap_put(pm_dmap, page_id, out_buffer);
        }
    }
    return out_buffer;
}

// uint8_t* pm_unpin_frame(uint32_t page_id, uint8_t is_dirty) {
//     return;
// }

// int pm_flush_frame(uint32_t page_id) {
//     return;
// }

// int pm_delete_frame(uint32_t page_id) {
//     return;
// }

// uint8_t* pm_cache_miss(uint32_t frame) {
//     return;
// }

void pm_evict(void) {
    return;
}
