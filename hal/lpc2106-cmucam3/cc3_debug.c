/*
 *  Copyright (C) 2010 Joel Andres Granados jogr@itu.dk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
 */

#include <stdio.h>
#include <stdarg.h>
#include "cc3.h"
#include "cc3_debug.h"

bool _cc3_debug_initialized = false;

bool
cc3_debug_initialize(void)
{
#if defined CC3_DEBUGGING
  _cc3_debug_initialized =
    cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
        CC3_UART_BINMODE_TEXT);
  return _cc3_debug_initialized;
#else
  return true;
#endif
}

void
cc3_debug_debug ( const int level, const char* file, const int line,
    const char* message, ...)
{
#if defined CC3_DEBUGGING
  va_list arg_list;

  if ( !_cc3_debug_initialized )
    cc3_debug_initialize();

  //If we don't initialize we just don't write to the serial.
  if ( _cc3_debug_initialized )
  {

    fprintf(stderr, "%d (%s:%d) - ", level, file, line);

    //We output the variable list.
    va_start(arg_list, message);
    vfprintf(stderr, message, arg_list);
    va_end(arg_list);

    fprintf(stderr, "\n");
    fflush(stderr);
  }
#endif
}

/*
 * Here we should think about centralizing the error handling...
 */
void cc3_debug_error ( const int level, const char* file, const int line,
    const char* message, ...)
{
#if defined CC3_DEBUGGING
  va_list arg_list;

  if ( !_cc3_debug_initialized )
    cc3_debug_initialize();

  //If we don't initialize we just don't write to the serial.
  if ( _cc3_debug_initialized )
  {
    fprintf(stderr, "%d (%s:%d) - ", level, file, line);

    //We output the variable list.
    va_start(arg_list, message);
    vfprintf(stderr, message, arg_list);
    va_end(arg_list);

    fprintf(stderr, "\n");

    // FIXME: We should be more informative here.  Not sure how to manage that.
    perror("");
    fflush(stderr);
  }
#endif
}
