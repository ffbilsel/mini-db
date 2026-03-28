#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H

#include <stdlib.h>
#include "db_global.h"

int pm_init(void);
uint8_t* pm_fetch_page(uint32_t page_id);
uint32_t pm_create_page(uint8_t *buffer);
int pm_close(void);

#endif /* BUFFER_POOL_MANAGER_H */
