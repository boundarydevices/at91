ENTRY(start)
SECTIONS {
        .text 0x202000 : { 
                *(.text) 
                "textEnd" = . ;
        }
        .data : { 
                *(.data) 
                *(.rodata )                
                "dataEnd" = . ;
        }
        .bss  : { 
                *(.bss) *(COMMON) 
                "bssEnd" = . ;
        }
}
