#ifndef INTERRUPT_H
#define INTERRUPT_H

void enable_ext_interrupt (void);
void disable_ext_interrupt (void);
void enable_servo_interrupt (void);
void disable_servo_interrupt (void);
void interrupt (void);

void undefined (void);
void swi (void);
void prefetch_abort (void);
void data_abort (void);
void segfault (void);

#endif
