/** Copyright © 2021 The Things Industries B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM1) + LENGTH(RAM1); /* end of "SRAM1" Ram type memory */

_Min_Heap_Size  = 0x400; /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack */

/* Memories definition */
MEMORY
{
  ROM    (rx)    : ORIGIN = 0x08000000, LENGTH = 220K   /* Flash memory dedicated to CM4, code (220KB) */
  STORAGE(r)     : ORIGIN = 0x08037800, LENGTH = 32K    /* Flash memory dedicated to CM4, storage (32KB) */
  COMMISSIONING (r) : ORIGIN = 0x0803F800, LENGTH = 2K   /* Flash memory dedicated to CM4, commissioning */
  RAM1   (xrw)   : ORIGIN = 0x20000000, LENGTH = 32K    /* Non-backup SRAM1 dedicated to CM4 */
  RAM2   (xrw)   : ORIGIN = 0x20008000, LENGTH = 32K    /* Backup SRAM2 dedicated to CM4 */
}

/* Sections */
SECTIONS
{
  .rom_start : {
    PROVIDE (_rom_start = .);
  } >ROM
  
  /* The startup code into "ROM" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(8);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(8);
  } >ROM

  /* The program code and other data into "ROM" Rom type memory */
  .text :
  {
    . = ALIGN(8);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(8);
    _etext = .;        /* define a global symbols at end of code */
  } >ROM

  /* Constant data into "ROM" Rom type memory */
  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(8);
  } >ROM

  .ARM.extab   : {
    . = ALIGN(8);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(8);
  } >ROM

  .ARM : {
    . = ALIGN(8);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(8);
  } >ROM

  .preinit_array     :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(8);
  } >ROM

  .init_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(8);
  } >ROM

  .fini_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(8);
  } >ROM

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections into "SRAM1" Ram type memory */
  .data :
  {
    . = ALIGN(8);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(8);
    _edata = .;        /* define a global symbol at data end */

  } >RAM1 AT> ROM

  /* Uninitialized data section into "SRAM1" Ram type memory */
  . = ALIGN(8);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(8);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM1

  /* Data section into "SRAM1" Ram type memory: Non-backup SRAM1 dedicated to CM4 */
  . = ALIGN(8);
  RAM1_region :
  {
    _sRAM1_region = .;         /* define a global symbol at section start */
    *(.RAM1_region)

    . = ALIGN(8);
    _eRAM1_region = .;         /* define a global symbol at section end */
  } >RAM1

  /* Data section into "SRAM2" Ram type memory: Backup SRAM2 dedicated to CM4 */
  . = ALIGN(8);
  RAM2_region :
  {
    _sRAM2_region = .;         /* define a global symbol at section start */
    *(.RAM2_region)

    . = ALIGN(8);
    _eRAM2_region = .;         /* define a global symbol at section end */
  } >RAM2

  /* User_heap_stack section, used to check that there is enough "SRAM1" Ram  type memory left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
    __RAM_segment_used_end__ = .;
  } >RAM1

  ._storage (NOLOAD):
  {
      . = ALIGN(8);
      *(.storage)
  } >STORAGE

  ._commissioning (NOLOAD):
  {
      . = ALIGN(8);
      *(.commissioning)
  } >COMMISSIONING

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
