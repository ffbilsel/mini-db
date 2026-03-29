#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H

#include <stdint.h>
#include "db_global.h"

#define POOL_SIZE 1024

/* Initializes the buffer pool and page table. Returns 0 on success, 1 on failure. */
int pm_init(void);

/* * Fetches a page from the buffer pool or reads it from disk.
 * IMPORTANT: The returned frame is PINNED. You must call pm_unpin_frame
 * when you are done with it, otherwise the Clock algorithm will deadlock.
 */
Frame* pm_fetch_frame(uint32_t page_id);

/* * Creates a brand new page, assigns it a new ID, and places it in the pool.
 * Returns a PINNED frame.
 */
Frame* pm_create_frame(void);

/* * Decrements the pin count of a frame, allowing it to be evicted if needed.
 * If is_dirty is 1, the frame's state is updated to FRAME_STATE_DIRTY.
 */
void pm_unpin_frame(uint32_t page_id, int is_dirty);

/* Flushes all dirty frames to disk and frees manager memory. Returns 0 on success. */
int pm_close(void);

#endif /* BUFFER_POOL_MANAGER_H */
