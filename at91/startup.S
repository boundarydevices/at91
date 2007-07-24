.extern lowlevel_Init
#include "hardware.h"

.global __div0
.global StartUp
__div0:
StartUp:
	b	V_Reset			//0
	b	V_UndefinedInstr	//4
	b	V_SWI			//8
	b	V_PrefetchAbort		//0x0c
	b	V_DataAbort		//0x10
	b	V_Unused		//0x14, not used
	b	V_IRQ			//0x18
	b	V_FIQ			//0x1c

.space [0x100-.-Startup];

.global imageLength
imageLength:
.space   4

.global start
V_Reset:
start:
        msr	cpsr_c, #PSR_F_BIT | PSR_I_BIT | SVC_MODE // ensure svc mode and irqs disabled

#ifdef RAMSTART
        ldr     r0,=StartUp             // absolute    (0x200000)
        adr     r1,StartUp              // PC-relative (0x202000)
        subs    r2,r1,r0
        beq     doneCopy

        ldr     r2,=bssStart            // absolute (0x20000+X)
        sub     r2,r2,r0                // subtract StartUp to determine length of text+data

        /*
         * r0 - destination
         * r1 - src
         * r2 - length
         */
        bl      shiftRAM
doneCopy:
#endif
        mov     r0,#0x0020
        mov     r0,r0,lsl #16
        add     r0,r0,#SRAM_MAX
        mov     sp,r0
        mov     fp,r0

        bl      lowlevel_Init
        mov	r0,#0
        mov     pc,r0

V_UndefinedInstr:
V_SWI:
V_PrefetchAbort:
V_DataAbort:
V_Unused:
V_IRQ:
V_FIQ:
        b V_Reset
