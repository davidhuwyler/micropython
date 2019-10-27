#ifndef _FLASH_HYPER_H_
#define _FLASH_HYPER_H_

#include "mpconfigport.h"
#if MICROPY_HW_HAS_HYPER_FLASH

#include "fsl_flexspi.h"
int flexspi_nor_init(void);
int flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
int flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t address, const uint32_t *src);

#endif //#if MICROPY_HW_HAS_HYPER_FLASH
#endif 

