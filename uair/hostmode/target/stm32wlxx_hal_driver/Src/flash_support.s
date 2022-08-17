.data
.globl _rom_start
.globl _rom_end
.globl _flash_end
.globl config_storage
.globl audit_storage
.globl log_storage
.globl commissioning_data

       .align 8
_rom_start:
       .space 32768,0x55
config_storage: /* Two 2K pages */
       .space 4096,255
audit_storage: /* Fourteen 2K pages */
       .space 28672,255
_rom_end:
       .space 8192,0xaa
commissioning_data:
       .space 2048,0xff
_flash_end:
