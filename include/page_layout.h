#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "disk_manager.h" /* Required for DM_PAGE_SIZE and dm_page_count_get() */

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
 * Page Header Getters
 * -------------------------------------------------------------------------- */
uint32_t pl_frame_get_page_id(uint8_t frame[]);
uint64_t pl_frame_get_lsn(uint8_t frame[]);
uint16_t pl_frame_get_slot_count(uint8_t frame[]);
uint16_t pl_frame_get_free_space_pointer(uint8_t frame[]);
uint16_t pl_frame_get_free_space(uint8_t frame[]);
uint16_t pl_frame_get_fragmented_space(uint8_t frame[]);
uint32_t pl_frame_get_flags(uint8_t frame[]);

/* --------------------------------------------------------------------------
 * Slot Array Management
 * -------------------------------------------------------------------------- */
uint16_t pl_frame_slot_get_start(uint8_t frame[], uint16_t slot_id);
uint16_t pl_frame_slot_get_size(uint8_t frame[], uint16_t slot_id);
void pl_frame_update_slot_array(uint8_t frame[], uint16_t slot_id, uint16_t new_start, uint16_t new_size);

/* --------------------------------------------------------------------------
 * Page Operations (Create, Insert, Update, Delete, Compact)
 * -------------------------------------------------------------------------- */
uint8_t* pl_frame_create(uint8_t *new_frame);
int pl_frame_insert_slot(uint8_t frame[], uint8_t *slot, uint16_t size);
int pl_frame_update_slot(uint8_t frame[], uint16_t slot_id, uint8_t *new_slot, uint16_t new_size);
int pl_frame_delete_slot(uint8_t frame[], uint16_t slot_id);
int pl_frame_compact(uint8_t frame[]);

#endif /* PAGE_LAYOUT_H */
