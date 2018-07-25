SECTIONS {
    . = DEFINED(_load_addr) ? _load_addr : 0x8000;
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
    _ram_start = .;
    . = . + 0x4000000;
    _heap_end = .;
    
    . = 0x8000000;
    _estack = .;
    _ram_end = .;
}
