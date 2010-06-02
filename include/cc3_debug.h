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

#define CC3_DEBUG_LEVEL0 0

#if defined CC3_DEBUGGING

#define CC3_DEBUG(message) \
            cc3_debug_debug(CC3_DEBUG_LEVEL0, __FILE__, __LINE__, message)

#else

#define CC3_DEBUG(message) do{}while(0)

#endif

bool cc3_debug_initialize(void);

/**
 * Handle debug messages.
 *
 * @param[in] level The level of importance of the message. Does not reall do
 *                  anything fancy, just prints a number.
 * @param[in] file The file where the debug message was originated.
 * @param[in] line The line where the debug message was originated.
 * @param[in] message The debug message.
 */
void cc3_debug_debug(const int level, const char* file,
        const int line, const char* message);
