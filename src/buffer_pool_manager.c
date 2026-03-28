#include "buffer_pool_manager.h"
#include "disk_manager.h"
#include "page_layout.h"
#include "dmap.h"
#include <string.h>

static dmap* pm_dmap = NULL;
static uint8_t* pm_callback_buffer = NULL;
static PageNode* pm_dirty_pages = NULL;

/* Eviction callback: Adds the evicted dirty page to the bulk write list */
static int pm_dmap_callback(uint32_t page_id, uint8_t* buffer) {
    PageNode* new_node = (PageNode*)malloc(sizeof(PageNode));
    if (new_node == NULL) {
        return 1; /* Memory allocation failure */
    }

    /* Copy the evicted buffer data into the page node */
    memcpy(new_node->page, buffer, DB_PAGE_SIZE);
    new_node->page_id = page_id;
    new_node->is_new = 0; /* Evicted pages from cache already exist on disk */
    
    /* Prepend to the linked list */
    new_node->next = pm_dirty_pages;
    pm_dirty_pages = new_node;

    return 0;
}

int pm_init(void) {
    if (pm_dmap == NULL) pm_dmap = (dmap *)malloc(sizeof(dmap));
    if (pm_callback_buffer == NULL) pm_callback_buffer = (uint8_t *)malloc(DB_PAGE_SIZE * sizeof(uint8_t));
    
    pm_dirty_pages = NULL; 
    
    if (pm_dmap != NULL) {
        memset(pm_dmap, 0, sizeof(dmap));
    }

    return (pm_dmap == NULL || pm_callback_buffer == NULL) ? 1 : 0;
}

uint8_t* pm_fetch_page(uint32_t page_id) {
    uint8_t* out_buffer = (uint8_t*)malloc(DB_PAGE_SIZE);
    int status;
    if (!out_buffer) return NULL;

    status = dmap_get(pm_dmap, page_id, out_buffer);
    
    if (status != 0) {
        free(out_buffer);
        out_buffer = dm_page_read(page_id);
        
        if (out_buffer == NULL) {
            return NULL;
        }
        
        status = dmap_put(pm_dmap, page_id, out_buffer, pm_dmap_callback, pm_callback_buffer);
        if (status != 0) {
            free(out_buffer);
            return NULL;
        }
    }
    return out_buffer;
}

uint32_t pm_create_page(uint8_t *buffer) {
    uint32_t page_id = db_page_count; // PM gets the ID from the global
    pl_page_create(buffer, page_id);  // Passes it to the page layout
    
    dmap_put(pm_dmap, page_id, buffer, pm_dmap_callback, pm_callback_buffer);
    dmap_set_is_dirty(pm_dmap, page_id, 1);
    
    db_page_count++; // Increment it here or let the disk manager do it on write
    return page_id;
}

int pm_close(void) {
    int i;

    /* 1. Sweep the cache for any dirty pages that haven't been evicted yet */
    if (pm_dmap != NULL) {
        for (i = 0; i < MAX_ENTRIES; i++) {
            if (pm_dmap->table[i].valid == 1 && pm_dmap->table[i].is_dirty == 1) {
                pm_dmap_callback(pm_dmap->table[i].key, pm_dmap->table[i].value);
            }
        }
    }

    /* 2. Execute the bulk write for all accumulated dirty pages */
    if (pm_dirty_pages != NULL) {
        /* dm_page_write_bulk handles the memory freeing of the nodes */
        pm_dirty_pages = dm_page_write_bulk(pm_dirty_pages); 
    }

    /* 3. Clean up manager memory */
    free(pm_dmap);
    free(pm_callback_buffer);
    
    pm_dmap = NULL;
    pm_callback_buffer = NULL;
    
    return 0;
}
