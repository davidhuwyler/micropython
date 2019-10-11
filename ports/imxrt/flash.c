#include "flash.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"

void flash_setSysPathToFlashFS(void)
{
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd_slash_lib));
}
