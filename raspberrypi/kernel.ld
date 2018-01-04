SECTIONS {
    . = 0x8000;
    .init . : { *(.init) }
    . = ALIGN(4);
    
    .text : { *(.text) }
    . = ALIGN(4);
    
    .data : { *(.data) }
    . = ALIGN(4);
    
    _bss_start = .;
    .bss : { *(.bss COMMON) }
    . = ALIGN(4);
    _bss_end = .;
    
    . = . + 0x1000;
    _heap_start = .;
    . = . + 0x800000;
    _heap_end = .;
    
    . = . + 0x800000;
    _estack = .;
}
