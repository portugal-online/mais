.data
.globl _rom_start
.globl _rom_end
.globl _flash_end
.globl config_storage
.globl log_storage

       .align 8
_rom_start:
       .space 32768,0
config_storage: /* Two 2K pages */
       .space 4096,0
log_storage: /* Fourteen 2K pages */
       .space 28672,0
_rom_end:
       .space 8192,0
_flash_end:
