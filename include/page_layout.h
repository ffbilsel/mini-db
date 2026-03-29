#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include <stdlib.h>
#include <string.h>
#include "db_global.h"

#define PAGE_ID_OFFSET            0
#define LSN_OFFSET                4
#define SLOT_COUNT_OFFSET         12
#define FREE_SPACE_POINTER_OFFSET 14
#define FRAGMENTED_SPACE_OFFSET   16
#define FLAGS_OFFSET              18
#define PL_HEADER_SIZE            22

int pl_insert_tuple(uint8_t page[], uint8_t *tuple, uint16_t size);
int pl_update_tuple(uint8_t page[], uint16_t slot_id, uint8_t *tuple, uint16_t new_size);
int pl_delete_tuple(uint8_t page[], uint16_t slot_id);

int pl_defragment_page(uint8_t page[]);

void pl_set_page_layout(uint8_t *page, uint32_t page_id);

#endif /* PAGE_LAYOUT_H */
