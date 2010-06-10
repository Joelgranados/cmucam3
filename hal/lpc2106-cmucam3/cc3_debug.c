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


static const char*
_debugnum_to_str ( const int debugnum )
{
  if ( debugnum == CC3_DEBUG_ERR )
    return "CC3_ERROR";
  else if ( debugnum == CC3_DEBUG_PRO )
    return "CC3_PRO_DEBUG";
  else if ( debugnum == CC3_DEBUG_HW )
    return "CC3_HW_DEBUG";
  else
    return "CC3_UNKNOWN";
}

bool _cc3_debug_initialized = false;

bool
cc3_debug_init ( void )
{
#if defined CC3_DEBUGGING
  if ( !_cc3_debug_initialized )
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
    const char* message, ... )
{
#if defined CC3_DEBUGGING
  va_list arg_list;

  //If we don't initialize we just don't write to the serial.
  if ( cc3_debug_init() )
  {

    fprintf(stderr, "%s (%s:%d) - ", _debugnum_to_str(level), file, line);

    //We output the variable list.
    va_start(arg_list, message);
    vfprintf(stderr, message, arg_list);
    va_end(arg_list);

    fprintf(stderr, "\n");
    fflush(stderr);
  }
#endif
}
