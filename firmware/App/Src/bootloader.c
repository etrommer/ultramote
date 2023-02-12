#include "bootloader.h"
#include "main.h"

#include <stdint.h>

static uint32_t bootloader_token __attribute__((section(".bootloader_token")));

void bootloader_enter()
{
    // set magic number to detect bootloader request during startup
    bootloader_token = BOOTLOADER_MAGIC;
    // Reset MCU
    __NVIC_SystemReset();
}

void bootloader_check()
{
    // check if magic number is set during startup
    if (bootloader_token == BOOTLOADER_MAGIC)
    {
        // erase so that we don't enter next time around
        bootloader_token = 0;

        //const uint32_t sysmem_addr = SYSTEM_FLASH_BASE;
        void (*sysmem_boot_jmp)(void);
        sysmem_boot_jmp = (void (*)(void))(*((uint32_t *)(SYSTEM_FLASH_BASE + 4)));

        // set bootloader stack pointer
        __set_MSP(*(uint32_t *) SYSTEM_FLASH_BASE);

        // jump to system memory
        sysmem_boot_jmp();
    }
}
