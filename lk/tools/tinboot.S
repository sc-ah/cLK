@
@ This script wraps LK bootloader to the CE XIP format
@ (C) 2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
@ based on Tinboot which is (C) Martin Johnson - M.J.Johnson@massey.ac.nz
@
.equ    LOAD_START,     0x10000000
.equ	TARGET_ADDR, 	0x00000000
.equ	XIP_END,	0x3f000

		.org 0
		b boot

		.org 0x40
		.word 0x43454345
		.word romhdr+0x80000000				@ location of wince romhdr
		.word romhdr
		.org 0x00001000

boot:
		ldr r0,smi_mpu_addr @disable MPU to allow us to copy the bootloader
		mov r1, #0
		str r1, [r0]
		ldr r0,blob_ptr
		ldr r1,target_addr_ptr
		ldr r3,blobend_ptr

copycode:
		ldr r2,[r0],#4
		str r2,[r1],#4
		cmp r0,r3
		blo copycode

		mov pc, #0

smi_mpu_addr:
		.word 0xa8250800
blob_ptr:
		.word LOAD_START + kernel
target_addr_ptr:
		.word TARGET_ADDR
blobend_ptr:
		.word LOAD_START + blobend

romhdr:
		.word 0x0		@ dllfirst
		.word 0x0		@ dlllast
		.word 0x80000000	@ physfirst
		.word 0x80000000+blobend	@ physlast
		.word 0			@ num mods
		.word 0x80000000+blobend	@ ramstart
		.word 0x80000000	@ ramfree
		.word 0x88000000	@ ramend
		.word 0			@ copyentries
		.word 0x80000000	@ copyoffset
		.word 0			@ profilelen
		.word 0			@ profileoffset
		.word 0			@ numfiles
		.word 0			@ flags
		.word 0x20		@ fsrampercent
		.word 0			@ drvglobstart
		.word 0			@ drvgloblen
		.word 0x201c2		@ cputype/flags
		.word 0x80000000	@ pextensions
		.word 0			@ trackingstart
		.word 0			@ trackinglen

		.align 12
kernel:
		.incbin "lk.bin"
		.align
blobend:
		.end
