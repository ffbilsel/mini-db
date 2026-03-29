#ifndef DB_GLOBAL_H
#define DB_GLOBAL_H

#include <stdint.h>

#define DB_PAGE_SIZE 8192

typedef enum {
    FRAME_STATE_EMPTY,
    FRAME_STATE_CLEAN,
    FRAME_STATE_NEW,
    FRAME_STATE_DIRTY
} FrameState;

typedef struct Frame {
    uint32_t page_id;
    uint8_t page[DB_PAGE_SIZE];
    FrameState state;
    int pin_count;      /* Number of active users preventing eviction */
    int ref_bit;        /* Clock algorithm second-chance bit */
    struct Frame *next; /* For bulk writing lists */
} Frame;

extern uint32_t db_page_count;

#endif  /* DB_GLOBAL_H */
