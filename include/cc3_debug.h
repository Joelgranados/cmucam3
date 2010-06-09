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

#define CC3_DEBUG_ERR 10
#define CC3_DEBUG_PRO 50
#define CC3_DEBUG_HW 100

#if CC3_DEBUGGING >= CC3_DEBUG_ERR
#define CC3_ERROR(message) \
    cc3_debug_error(CC3_DEBUG_ERR, __FILE__, __LINE__, message);
#else
#define CC3_ERROR(message) do{}while(0);
#endif

#if CC3_DEBUGGING >= CC3_DEBUG_PRO
#define CC3_PDEBUG(message) \
    cc3_debug_debug(CC3_DEBUG_PRO, __FILE__, __LINE__, message);
#else
#define CC3_PDEBUG(message) do{}while(0);
#endif

#if CC3_DEBUGGING >= CC3_DEBUG_HW
#define CC3_HWDEBUG(message) \
    cc3_debug_debug(CC3_DEBUG_HW, __FILE__, __LINE__, message);
#else
#define CC3_HWDEBUG(message) do{}while(0);
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

/**
 * Handle error messages.
 *
 * @param[in] level The level of importance of the message. Does not reall do
 *                  anything fancy, just prints a number.
 * @param[in] file The file where the debug message was originated.
 * @param[in] line The line where the debug message was originated.
 * @param[in] message The debug message.
 */
void cc3_debug_error(const int level, const char* file,
        const int line, const char* message);
