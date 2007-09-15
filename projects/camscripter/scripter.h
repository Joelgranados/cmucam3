#ifndef SCRIPTER_H_
#define SCRIPTER_H_

#include <stdbool.h>
#include "cc3.h"
#include "csmsg.h"

#define SERIAL_BAUD_RATE  CC3_UART_RATE_115200 //CC3_UART_RATE_14400 //CC3_UART_RATE_115200
#define MAX_MSG_LEN 5*1024

#define DEFAULT_DEBUG	1
#define MODE_STAND_ALONE	0
#define MODE_INTERACTIVE	1

void debug( char* msg, ... );
void handle_message( ScripterMsg* msg );
uint8_t get_exec_mode(void);
void go_stand_alone(void);
void go_interactive(void);
int get_int(uint8_t *bytes); // turn four-byte message into an int

/** non-blocking getchar **/
bool recv_char(uint_least8_t* c);

#endif /*SCRIPTER_H_*/
