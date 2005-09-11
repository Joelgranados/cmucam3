/******************************************************************************
       Module: startup.s
  Description: Device controller - Startup Code for Flash example
Version         Date                Initials    Description
PFARM v1.0.0    19-FEB-2003         ASH         Initial
PFARM v1.0.8     9-JUL-2003         ASH         Initialise data and interrupts
                                                correctly
******************************************************************************/
.arm
.text
.global     _int_vectors
.func       _int_vectors

/******************************************************************************
*          Exception vector table - common to all ARM-based systems           *
******************************************************************************* 
* Common to all ARM-based systems. See ARM Architecture Reference Manual,     *
* Programmer's Model section for details. Table entries just jump to handlers *
* (using full 32-bit addressing).                                             *
******************************************************************************/

_int_vectors:   
    
    ldr     pc, do_reset_addr
    ldr     pc, do_undefined_instruction_addr
    ldr     pc, do_software_interrupt_addr
    ldr     pc, do_prefetch_abort_addr
    ldr     pc, do_data_abort_addr
    .long   0xB8A06F60                        /* ARM-reserved vector */
    ldr     pc, do_irq_addr
    ldr     pc, do_fiq_addr

do_reset_addr:                  .long   do_reset
do_undefined_instruction_addr:  .long   do_undefined_instruction
do_software_interrupt_addr:     .long   do_software_interrupt
do_prefetch_abort_addr:         .long   do_prefetch_abort
do_data_abort_addr:             .long   do_data_abort
do_fiq_addr:                    .long   do_fiq
do_irq_addr:                    .long   do_irq

/******************************************************************************
*                         Interrupt exceptions                                *
******************************************************************************/
do_irq:
    stmfd   sp!, { lr }               /* save return address on stack */
    mrs     lr, spsr                  /* use lr to save spsr_irq */
    stmfd   sp!, { r0-r3, r12, lr }   /* save work regs & spsr on stack */
    bl      interrupt		      /* go handle the interrupt */
    ldmfd   sp!, { r0-r3, r12, lr }   /* restore regs from stack */
    msr     spsr_cxsf, lr             /* put back spsr_irq */
    ldmfd   sp!, { lr }               /* put back lr_irq */
    subs    pc, lr, #0x4              /* return, restoring CPSR from SPSR */    
    
    
/******************************************************************************
*                         Yet to be implemented exceptions                    *
*******************************************************************************
* Just fall through to reset exception handler for now                        *                                     *
******************************************************************************/                                                                            
do_undefined_instruction:
do_software_interrupt:
do_prefetch_abort:
do_data_abort:
do_fiq:

/******************************************************************************
*                            System reset handler                             *
******************************************************************************/
do_reset:

    /*Set stack pointers for all modes used (supervisor, IRQ and FIQ)*/
    msr     cpsr_c, #0xd3       /* ensure we're in supervisor mode */
    ldr     sp, =__stack_svc    /* set supervisor mode stack pointer */

    msr     cpsr_c, #0xd2       /* enter IRQ mode, with interrupts disabled */
    ldr     sp, =__stack_irq    /* set IRQ mode stack pointer */

    msr     cpsr_c, #0xd1       /* enter FIQ mode, with interrupts disabled */
    ldr     sp, =__stack_fiq    /* set FIQ mode stack pointer */

    /* Clear uninitialized data section (bss) */
    ldr     r4, =__start_bss      /* First address     */ 
    ldr     r5, =__end_bss       /* Last  address      */
    mov     r6, #0x0                                
                                               
loop_zero:  
    str     r6, [r4]                                
    add     r4, r4, #0x4                            
    cmp     r4, r5                                  
    blt     loop_zero

/* Copy initialized data sections from ROM into RAM 
    ldr     r4, =_fdata      /* destination address       
    ldr     r5, =_edata      /* Last  address      
    ldr     r6, =_etext      /* source address      
    cmp     r4, r5                                  
    beq     skip_initialize

loop_initialise:
    ldr     r3, [r6]
    str     r3, [r4]                                
    add     r4, r4, #0x4
    add     r6, r6, #0x4
    cmp     r4, r5                                  
    blt     loop_initialise
    
skip_initialize:*/

    /* Enable interrupts, enter supervisor mode and branch to start of 'C' code */
    msr     cpsr_c, #0x13       /* I=0 F=0 T=0 MODE=supervisor */
    bl      main

/******************************************************************************
*                        End of startup and interrupt code                    *
******************************************************************************/
.size   _int_vectors,.-_int_vectors;
.endfunc
