ENTRY(start)
SECTIONS {
        .text 0x200000 : { 
		"textStart" = . ;
		"StartUp" = . ;
                *(.text) 
                *(.rodata )                
                "textEnd" = . ;
        }
        .data : { 
		"dataStart" = . ;
                *(.data) 
                "dataEnd" = . ;
		"bssStart" = . ;
                *(.xdata)
        }
	.bss  : { 
                *(.bss) *(COMMON) 
                "bssEnd" = . ;
        }
        
}
