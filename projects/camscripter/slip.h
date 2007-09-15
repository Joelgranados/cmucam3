/* SLIP special character codes */
#ifndef SLIP_H
#define SLIP_H

#include "csmsg.h"

#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

void init_packet( void );
void finish_packet( void );
void put_slip_char( char c );
void send_packet(uint_least8_t *p, int len);
void stream_packet(char* (*char_getter)(char*), char* msg );
int recv_packet(uint_least8_t* p, int len);
void put_slip_string( char* s );
void put_slip_size(int size);
void put_slip_integer(int number);
#endif /* SLIP_H */
