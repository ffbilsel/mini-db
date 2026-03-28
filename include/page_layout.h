#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include "db_global.h"

#include <stdlib.h>
#include <string.h>

/* 
 * Page Header Offsets (Total: 22 Bytes)
 * Based on the following structure:
 * uint32_t page_id;             (4 bytes)
 * uint64_t lsn;                 (8 bytes)
 * uint16_t slot_count;          (2 bytes)
 * uint16_t free_space_pointer;  (2 bytes)
 * uint16_t fragmented_space;    (2 bytes)
 * uint32_t flags;               (4 bytes)
 */
#define PAGE_ID_OFFSET            0
#define LSN_OFFSET                4
#define SLOT_COUNT_OFFSET         12
#define FREE_SPACE_POINTER_OFFSET 14
#define FRAGMENTED_SPACE_OFFSET   16
#define FLAGS_OFFSET              18
#define PL_HEADER_SIZE            22

/* --------------------------------------------------------------------------
 * Page Operations (Create, Insert, Update, Delete, Compact)
 * -------------------------------------------------------------------------- */
int pl_page_insert_tuple(uint8_t page[], uint8_t *tuple, uint16_t size);
int pl_page_update_tuple(uint8_t page[], uint16_t slot_id, uint8_t *tuple, uint16_t new_size);
int pl_page_delete_tuple(uint8_t page[], uint16_t slot_id);

int pl_page_compact(uint8_t page[]);

void pl_page_create(uint8_t *new_page, uint32_t page_id);

#endif /* PAGE_LAYOUT_H */
