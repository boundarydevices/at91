/*
 * r0 - destination
 * r1 - src
 * r2 - length
 */
.section .xdata

.global shiftRAM
shiftRAM:
        ldr     r3,[r1],#4
        str     r3,[r0],#4
        subs    r2,r2,#4
        bpl     shiftRAM

        /*
         * Fix-up return value
         */
        sub     r0,r1,r0
        sub     pc,lr,r0

