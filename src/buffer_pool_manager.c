#include "buffer_pool_manager.h"
#include "disk_manager.h"
#include "page_layout.h"
#include "page_table.h"

static PageTable *page_table = NULL;
static Frame *buffer_pool = NULL;
static int clock_hand = 0;

int pm_init(void) {
    int i;
    
    page_table = (PageTable*)malloc(sizeof(PageTable));
    buffer_pool = (Frame*)malloc(POOL_SIZE * sizeof(Frame));
    
    if (page_table == NULL || buffer_pool == NULL) return 1;
    
    pt_init(page_table);
    clock_hand = 0;
    
    for (i = 0; i < POOL_SIZE; i++) {
        buffer_pool[i].pin_count = 0;
        buffer_pool[i].ref_bit = 0;
        buffer_pool[i].state = FRAME_STATE_EMPTY;
        buffer_pool[i].next = NULL;
    }
    return 0;
}

static int find_victim_frame(void) {
    int iterations = 0;
    while (iterations < POOL_SIZE * 2) { /* Sweep twice at most */
        Frame *f = &buffer_pool[clock_hand];
        
        if (f->state == FRAME_STATE_EMPTY) {
            int victim = clock_hand;
            clock_hand = (clock_hand + 1) % POOL_SIZE;
            return victim;
        }

        if (f->pin_count == 0) {
            if (f->ref_bit == 1) {
                f->ref_bit = 0; /* Second chance */
            } else {
                int victim = clock_hand;
                clock_hand = (clock_hand + 1) % POOL_SIZE;
                return victim;
            }
        }
        clock_hand = (clock_hand + 1) % POOL_SIZE;
        iterations++;
    }
    
    /* If we get here, every single page in the pool is pinned! */
    return -1; 
}

Frame* pm_fetch_frame(uint32_t page_id) {
    int frame_id = pt_get(page_table, page_id);
    Frame *frame;

    if (frame_id != -1) {
        buffer_pool[frame_id].pin_count++;
        buffer_pool[frame_id].ref_bit = 1;
        return &buffer_pool[frame_id];
    }

    frame_id = find_victim_frame();
    frame = &buffer_pool[frame_id];

    if (frame->state == FRAME_STATE_DIRTY || frame->state == FRAME_STATE_NEW) {
        dm_write_page(frame);
    }

    if (frame->state != FRAME_STATE_EMPTY) {
        pt_remove(page_table, frame->page_id);
    }

    if (dm_read_page_into(frame->page, page_id) != 0) {
        return NULL; 
    }

    frame->page_id = page_id;
    frame->pin_count = 1; 
    frame->ref_bit = 1;
    frame->state = FRAME_STATE_CLEAN;

    pt_put(page_table, page_id, frame_id);

    return frame;
}

Frame* pm_create_frame(void) {
    int frame_id = find_victim_frame();
    Frame *frame = &buffer_pool[frame_id];

    if (frame->state == FRAME_STATE_DIRTY || frame->state == FRAME_STATE_NEW) {
        dm_write_page(frame);
    }

    if (frame->state != FRAME_STATE_EMPTY) {
        pt_remove(page_table, frame->page_id);
    }

    /* Assign next global ID and initialize the physical page layout */
    frame->page_id = db_page_count; 
    db_page_count++;
    
    pl_set_page_layout(frame->page, frame->page_id);

    frame->pin_count = 1;
    frame->ref_bit = 1;
    frame->state = FRAME_STATE_NEW;

    pt_put(page_table, frame->page_id, frame_id);

    return frame;
}

void pm_unpin_frame(uint32_t page_id, int is_dirty) {
    int frame_id = pt_get(page_table, page_id);
    if (frame_id != -1) {
        if (buffer_pool[frame_id].pin_count > 0) {
            buffer_pool[frame_id].pin_count--;
        }
        if (is_dirty) {
            buffer_pool[frame_id].state = FRAME_STATE_DIRTY;
        }
    }
}

int pm_close(void) {
    int i;
    Frame *dirty_list = NULL;

    if (buffer_pool == NULL) return 0;

    for (i = 0; i < POOL_SIZE; i++) {
        Frame *f = &buffer_pool[i];
        if (f->state == FRAME_STATE_DIRTY || f->state == FRAME_STATE_NEW) {
            /* Create a copy of the frame for the bulk write list */
            Frame *copy = (Frame*)malloc(sizeof(Frame));
            memcpy(copy, f, sizeof(Frame));
            copy->next = dirty_list;
            dirty_list = copy;
        }
    }

    if (dirty_list != NULL) {
        dm_write_pages(dirty_list); 
    }

    pt_clear(page_table);
    free(page_table);
    free(buffer_pool);

    page_table = NULL;
    buffer_pool = NULL;
    return 0;
}
