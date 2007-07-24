ENTRY(start)
SECTIONS {
        .text 0x00100000 : { 
		"textStart" = . ;
                *(.text) 
                *(.rodata )                
                "textEnd" = . ;
        }
        .data 0x200000 : AT ( LOADADDR(.text) + SIZEOF ( .text) ){ 
		"dataStart" = . ;
                *(.data) 
                "dataEnd" = . ;
        }
        .bss  : AT ( LOADADDR(.data) + SIZEOF ( .data) ){ 
		"bssStart" = . ;
                *(.bss) *(COMMON) 
                "bssEnd" = . ;
        }
}
