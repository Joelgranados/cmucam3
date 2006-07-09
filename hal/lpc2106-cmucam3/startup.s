/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 */

/*
 crt0.S for LPC2xxx
 - based on examples from R O Software
 - based on examples from newlib-lpc
 - based on an example from Anglia Designs

 collected and modified by Martin Thomas
*/

/*
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


        .global _etext                  @ -> .data initial values in ROM
        .global _data                   @ -> .data area in RAM
        .global _edata                  @ end of .data area
        .global __bss_start             @ -> .bss area in RAM
        .global __bss_end__             @ end of .bss area
        .global _stack                  @ top of stack

@ Stack Sizes
        .set  UND_STACK_SIZE, 0x00000004
        .set  ABT_STACK_SIZE, 0x00000004
        .set  FIQ_STACK_SIZE, 0x00000004
        .set  IRQ_STACK_SIZE, 0X00000080
        .set  SVC_STACK_SIZE, 0x00000004

@ Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
        .set  MODE_USR, 0x10            @ User Mode
        .set  MODE_FIQ, 0x11            @ FIQ Mode
        .set  MODE_IRQ, 0x12            @ IRQ Mode
        .set  MODE_SVC, 0x13            @ Supervisor Mode
        .set  MODE_ABT, 0x17            @ Abort Mode
        .set  MODE_UND, 0x1B            @ Undefined Mode
        .set  MODE_SYS, 0x1F            @ System Mode

        .equ  I_BIT, 0x80               @ when I bit is set, IRQ is disabled
        .equ  F_BIT, 0x40               @ when F bit is set, FIQ is disabled

        .text
	.arm
	.section .init, "ax"

        .code 32
        .align 2

        .global _boot
        .func   _boot
_boot:

@ Runtime Interrupt Vectors
@ -------------------------
Vectors:
        b     _start                    @ reset - _start
        ldr   pc,_undf                  @ undefined - _undf
        ldr   pc,_swi                   @ SWI - _swi
	ldr   pc,_pabt                  @ program abort - _pabt
	ldr   pc,_dabt                  @ data abort - _dabt
        nop                             @ reserved
        ldr   pc,_irq                   @ IRQ
        ldr   pc,_fiq                   @ FIQ - _fiq

@ Use this group for development
_undf:  .word __undf                    @ undefined
_swi:   .word __swi                     @ SWI
_pabt:  .word __pabt                    @ prefetch abort
_dabt:  .word __dabt                    @ data abort
_irq:   .word __irq                     @ IRQ
_fiq:   .word __fiq                     @ FIQ

__undf: b     undefined                 @ undefined
__swi:  b     swi                       @ SWI
__pabt: b     .				@ prefetch abort
__dabt: b     .			 	@ data abort
__fiq:  nop                              @ FIQ
__irq:  stmfd   sp!, { lr }               /* save return address on stack */
	mrs     lr, spsr                  /* use lr to save spsr_irq */
	stmfd   sp!, { r0-r3, r12, lr }   /* save work regs & spsr on stack */
	bl      interrupt                 /* go handle the interrupt */
	ldmfd   sp!, { r0-r3, r12, lr }   /* restore regs from stack */
	msr     spsr_cxsf, lr             /* put back spsr_irq */
	ldmfd   sp!, { lr }               /* put back lr_irq */
	subs    pc, lr, #0x4              /* return, restoring CPSR from SPSR */
        .size _boot, . - _boot
        .endfunc


@ Setup the operating mode & stack.
@ ---------------------------------
        .global _start, start, _mainCRTStartup
        .func   _start

_start:
start:
_mainCRTStartup:

@ Initialize Interrupt System
@ - Set stack location for each mode
@ - Leave in System Mode with Interrupts Disabled
@ -----------------------------------------------
        ldr   r0,=_stack
        msr   CPSR_c,#MODE_UND|I_BIT|F_BIT @ Undefined Instruction Mode
        mov   sp,r0
        sub   r0,r0,#UND_STACK_SIZE
        msr   CPSR_c,#MODE_ABT|I_BIT|F_BIT @ Abort Mode
        mov   sp,r0
        sub   r0,r0,#ABT_STACK_SIZE
        msr   CPSR_c,#MODE_FIQ|I_BIT|F_BIT @ FIQ Mode
        mov   sp,r0
        sub   r0,r0,#FIQ_STACK_SIZE
        msr   CPSR_c,#MODE_IRQ|I_BIT|F_BIT @ IRQ Mode
        mov   sp,r0
        sub   r0,r0,#IRQ_STACK_SIZE
        msr   CPSR_c,#MODE_SVC|I_BIT|F_BIT @ Supervisor Mode
        mov   sp,r0
        sub   r0,r0,#SVC_STACK_SIZE
        msr   CPSR_c,#MODE_SYS|I_BIT|F_BIT @ System Mode
        mov   sp,r0

@ Clear .bss
@ ----------
        mov   r0,#0                     @ get a zero
        ldr   r1,=__bss_start           @ -> bss start
        ldr   r2,=__bss_end__           @ -> bss end
2:      cmp   r1,r2                     @ check if data to clear
        strlo r0,[r1],#4                @ clear 4 bytes
        blo   2b                        @ loop until done

@ Copy initialized data to its execution address in RAM
@ -----------------------------------------------------
        ldr   r1,=_etext                @ -> ROM data start
        ldr   r2,=_data                 @ -> data start
        ldr   r3,=_edata                @ -> end of data
1:      cmp   r2,r3                     @ check if data to move
        ldrlo r0,[r1],#4                @ copy it
        strlo r0,[r2],#4
        blo   1b                        @ loop until done


@ Call main program: main(0)
@ --------------------------
        mov   r0,#0                     @ no arguments (argc = 0)
        mov   r1,r0
        mov   r2,r0
        mov   fp,r0                     @ null frame pointer
        mov   r7,r0                     @ null frame pointer for thumb
        ldr   r10,=main
        mov   lr,pc

/* Enter the C code, use BX instruction so as to never return */
/* use BLX (?) main if you want to use c++ destructors below */
        @msr     cpsr_c, #0x13       /* I=0 F=0 T=0 MODE=supervisor */
        msr   CPSR_c,#MODE_SYS|F_BIT @ System Mode
        bx    r10                       @ enter main()

_reset:
reset:
exit:
abort:
	b .

        .size _reset, . - _reset
        .endfunc

        .end
