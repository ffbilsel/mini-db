#include "page_layout.h"

static uint64_t frame_read_bytes(uint8_t frame[], uint16_t offset, uint16_t byte_size) {
    uint64_t res = 0;
    uint16_t i; 
    for (i = 0; i < byte_size; i++) {
        res = (res << 8) | frame[offset + i];
    }
    return res;
}

static void frame_write_bytes_to_frame(uint8_t frame[], uint16_t offset, uint64_t data, uint8_t byte_size) {
    uint16_t i; 
    for (i = 0; i < byte_size; i++) {
        uint8_t shift_amount = 8 * (byte_size - 1 - i);
        frame[offset + i] = (uint8_t)((data >> shift_amount) & 0xFF);
    }
}

static void util_buffer_write_section(uint8_t* dest, uint8_t* src, uint16_t dest_offset, uint16_t src_offset, uint16_t size) {
    memcpy(dest + dest_offset, src + src_offset, size);
}

uint32_t pl_frame_get_page_id(uint8_t frame[]) {
    return frame_read_bytes(frame, PAGE_ID_OFFSET, 4);
}

uint64_t pl_frame_get_lsn(uint8_t frame[]) {
    return frame_read_bytes(frame, LSN_OFFSET, 8);
}

uint16_t pl_frame_get_slot_count(uint8_t frame[]) {
    return frame_read_bytes(frame, SLOT_COUNT_OFFSET, 2);
}

uint16_t pl_frame_get_free_space_pointer(uint8_t frame[]) {
    return frame_read_bytes(frame, FREE_SPACE_POINTER_OFFSET, 2);
}

uint16_t pl_frame_get_free_space(uint8_t frame[]) {
    uint16_t slot_array_end = PL_HEADER_SIZE + (4 * pl_frame_get_slot_count(frame));
    return pl_frame_get_free_space_pointer(frame) - slot_array_end;
}

uint16_t pl_frame_get_fragmented_space(uint8_t frame[]) {
    return frame_read_bytes(frame, FRAGMENTED_SPACE_OFFSET, 2);
}

uint32_t pl_frame_get_flags(uint8_t frame[]) {
    return frame_read_bytes(frame, FLAGS_OFFSET, 4);
}

void pl_frame_update_slot_array(uint8_t frame[], uint16_t slot_id, uint16_t new_start, uint16_t new_size) {
    uint16_t slot_array_offset = PL_HEADER_SIZE + 4 * slot_id;
    frame_write_bytes_to_frame(frame, slot_array_offset, new_start, 2);
    frame_write_bytes_to_frame(frame, slot_array_offset + 2, new_size, 2);
}

int pl_frame_insert_slot(uint8_t frame[], uint8_t *slot, uint16_t size) {
    /* C89: All variables declared at the top */
    uint16_t required_space;
    uint16_t offset;
    uint16_t new_slot_count;

    required_space = size + 4; /* Data size + 4 bytes for the slot array entry */
    if (required_space > pl_frame_get_free_space(frame)) {
        return 1; /* Not enough room */
    }

    offset = pl_frame_get_free_space_pointer(frame) - size;
    util_buffer_write_section(frame, slot, offset, 0, size);

    frame_write_bytes_to_frame(frame, FREE_SPACE_POINTER_OFFSET, offset, 2);

    new_slot_count = pl_frame_get_slot_count(frame) + 1; 
    frame_write_bytes_to_frame(frame, SLOT_COUNT_OFFSET, new_slot_count, 2);

    pl_frame_update_slot_array(frame, new_slot_count - 1, offset, size);

    return 0;
}

uint16_t pl_frame_slot_get_start(uint8_t frame[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return frame_read_bytes(frame, slot_offset, 2);
}

uint16_t pl_frame_slot_get_size(uint8_t frame[], uint16_t slot_id) {
    uint16_t slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    return frame_read_bytes(frame, slot_offset + 2, 2);
}

int pl_frame_update_slot(uint8_t frame[], uint16_t slot_id, uint8_t *new_slot, uint16_t new_size) {
    /* C89: All variables declared at the top */
    uint16_t slot_count;
    uint16_t slot_start;
    uint16_t slot_size;

    slot_count = pl_frame_get_slot_count(frame);
    if (slot_count <= slot_id) {
        return 1;
    }

    slot_start = pl_frame_slot_get_start(frame, slot_id);
    slot_size = pl_frame_slot_get_size(frame, slot_id);
    
    if (new_size != slot_size) {
        return 1;
    }

    util_buffer_write_section(frame, new_slot, slot_start, 0, slot_size);

    return 0;
}

int pl_frame_delete_slot(uint8_t frame[], uint16_t slot_id) {
    /* C89: All variables declared at the top */
    uint16_t slot_start;
    uint16_t slot_size;
    uint16_t slot_offset;

    slot_start = pl_frame_slot_get_start(frame, slot_id);
    if (slot_start == 0) {
        return 1;
    }
    
    slot_size = pl_frame_slot_get_size(frame, slot_id);
    slot_offset = PL_HEADER_SIZE + 4 * slot_id;
    
    frame_write_bytes_to_frame(frame, slot_offset, 0, 2);
    frame_write_bytes_to_frame(frame, FRAGMENTED_SPACE_OFFSET, pl_frame_get_fragmented_space(frame) + slot_size, 2);
    
    return 1;
}

int pl_frame_compact(uint8_t frame[]) {
    /* C89: Declarations at the top */
    uint16_t free_space_pointer;
    uint16_t used_data_size;
    uint16_t slot_count;
    uint8_t *data_snapshot;
    uint16_t i;
    uint16_t slot_start;
    uint16_t slot_size;
    uint16_t target_offset;
    uint16_t new_slot_count;

    free_space_pointer = pl_frame_get_free_space_pointer(frame);
    
    /* The actual size of the tuples sitting at the end of the page */
    used_data_size = DB_PAGE_SIZE - free_space_pointer;
    slot_count = pl_frame_get_slot_count(frame);

    /* If there is no data to compact, just clear fragmentation and return */
    if (used_data_size == 0) {
        frame_write_bytes_to_frame(frame, FRAGMENTED_SPACE_OFFSET, 0, 2);
        return 0;
    }

    /* Allocate exactly enough memory to back up the active tuples */
    data_snapshot = (uint8_t *)malloc(used_data_size);
    if (!data_snapshot) return 1;

    /* BACKUP: Copy FROM frame TO data_snapshot */
    /* dest=data_snapshot, src=frame, dest_offset=0, src_offset=free_space_pointer */
    util_buffer_write_section(data_snapshot, frame, 0, free_space_pointer, used_data_size);
    
    target_offset = DB_PAGE_SIZE;
    new_slot_count = 0;
    
    for (i = 0; i < slot_count; i++) {
        slot_start = pl_frame_slot_get_start(frame, i);
        slot_size = pl_frame_slot_get_size(frame, i);
        
        if (slot_start != 0) { /* If the slot hasn't been deleted */
            target_offset -= slot_size;
            
            /* RESTORE: Copy FROM data_snapshot TO frame */
            /* The tuple's position in the snapshot is (slot_start - free_space_pointer) */
            util_buffer_write_section(frame, data_snapshot, target_offset, slot_start - free_space_pointer, slot_size);
            
            pl_frame_update_slot_array(frame, new_slot_count, target_offset, slot_size);            
            new_slot_count++;
        }
    }

    /* Update the header to reflect the newly compacted state */
    frame_write_bytes_to_frame(frame, SLOT_COUNT_OFFSET, new_slot_count, 2);
    frame_write_bytes_to_frame(frame, FREE_SPACE_POINTER_OFFSET, target_offset, 2);
    frame_write_bytes_to_frame(frame, FRAGMENTED_SPACE_OFFSET, 0, 2);
    
    free(data_snapshot);
    
    return 0;
}

uint8_t* pl_frame_create(uint8_t *new_frame) {
    uint32_t page_id = dm_page_count_get(); 
    frame_write_bytes_to_frame(new_frame, PAGE_ID_OFFSET, page_id, 4);
    frame_write_bytes_to_frame(new_frame, LSN_OFFSET, 0, 8);
    frame_write_bytes_to_frame(new_frame, SLOT_COUNT_OFFSET, 0, 2);
    frame_write_bytes_to_frame(new_frame, FREE_SPACE_POINTER_OFFSET, DB_PAGE_SIZE, 2); 
    frame_write_bytes_to_frame(new_frame, FRAGMENTED_SPACE_OFFSET, 0, 2);
    frame_write_bytes_to_frame(new_frame, FLAGS_OFFSET, 0, 4);
    return new_frame;
}
