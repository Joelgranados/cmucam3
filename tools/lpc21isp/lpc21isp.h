/*****************************************************************************
 ** This data and information is proprietary to, and a valuable trade
 ** secret off, Q-Free Products A/S.  It is given in confidence by
 ** Q-Free Products A/S and may only be used as permitted under the
 ** license agreement under which it has been distributed, and in no
 ** other way.
 **
 ** Copyright (C) 2004 Q-Free Products A/S. All rights reserved.
 ****************************************************************************/

/** \file
 *
 * \brief define a function to Upload Binary according to LPC2114 bootloader
 *        protocol.
 *
 * $Author: cyrilh $
 * $Date: 2006/02/20 14:02:29 $
 * $Revision: 1.18 $
 */
#ifndef lpc21isp_h
#define lpc21isp_h



#include "Types.h"
#include "Uart.h"
#include "QFBCNP.h"
#include "lpc210x.h"
#include "buff.h"
#include "app.h"


/*
debug levels
0 - very quiet          - Anything at this level gets printed
1 - quiet               - Only error messages should be printed
2 - indicate progress   - Add progress messages
3 - first level debug   - Major level tracing
4 - second level debug  - Add detailed deugging
5 - log comm's          - log serial I/O
*/



#ifdef COMPILE_FOR_LPC21

#   define my_level( level ) (          \
        (level == 0)?   0:              \
        (level == 1)?   DFLT_LEVEL/2:   \
        (level == 2)?   DFLT_LEVEL:     \
        (level == 3)?   0xA0:           \
        (level == 4)?   0xB0:           \
                        0xC0            \
    )


#   define DebugPrintf( level, args... )  { appDebugPrintf( my_level( level ), ##args ); }
#   ifdef fflush
#       undef fflush
#   endif
#   define fflush(a) {}
#   ifdef time
#       undef time
#   endif
#   define time(a) 0

#   include "timer.h"
#   define Sleep sleep

    static inline void write(int UNUSED fd, const unsigned char *s, int n)
    {
        buffSaveData( app_map.nw2serial, s, n );
    }
    
    static inline int read(int UNUSED fd, char* r, int count)
    {   
        resetWatchDogIfCritical();
        return buffDelete( app_map.serial2nw, r, count );
    }    

#else

    typedef unsigned char BINARY;   /**< data type used for microcontroller
                                      *  memory image.                        */
                                
    typedef struct
    {
        unsigned char TerminalOnly;
        unsigned char DetectOnly;
        int           DetectedDevice;       /** index in LPCtypes[] array */
        char *baud_rate;                    /**< Baud rate to use on the serial
                                             * port communicating with the
                                             * microcontroller. Read from the
                                             * command line.                        */
                                         
        char *StringOscillator;             /**< Holds representation of oscillator
                                              * speed from the command line.         */
        BINARY *BinaryContent;              /**< Binary image of the
                                              * microcontroller's memory.             */
        unsigned long BinaryLength;
        unsigned long BinaryOffset;
        unsigned long StartAddress;
    
        int fdCom;
    
        unsigned serial_timeout_count;   /**< Local used to track
                                           * timeouts on serial port read. */
    
    } ISP_ENVIRONMENT;
   
    
    extern int PhilipsDownload(ISP_ENVIRONMENT *IspEnvironment);

    static inline int UploadBinary( unsigned char *BinaryContent, unsigned long BinaryLength,
                        unsigned char* StringOscillator )
    {
        ISP_ENVIRONMENT environment = {
            .StringOscillator = StringOscillator,
            .BinaryContent = BinaryContent,
            .BinaryLength = BinaryLength
        };
        return PhilipsDownload( &environment );
    }
#endif



#endif /* !defined(lpc21isp_h) */

/* EOF lpc21isp.h */
