#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_global.h"

#define DM_FILENAME "main.db"

Frame* dm_write_pages(Frame *frames);
int dm_read_page_into(uint8_t *dest, uint32_t page_id);
int dm_write_page(Frame *frame);

int dm_init(void);
int dm_close(void);
int dm_clear(void);

#endif /* DISK_MANAGER_H */
