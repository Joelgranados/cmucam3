#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lualib.h"
#include <cc3.h>
#include "slip.h"
#include "scripter.h"
#include <string.h>
#include "csmsg.h"

// Get rid of lua code generator and parser.
LUAI_FUNC int luaU_dump (lua_State* L, const Proto* f, lua_Writer w, void* data, int strip) {
  UNUSED(f);
  UNUSED(w);
  UNUSED(data);
  UNUSED(strip);
  UNUSED(L);
  return 0;
}
LUAI_FUNC void luaX_init (lua_State *L) {
  UNUSED(L);
}

// initialize the debug flag to it's default value
int debug_on = DEFAULT_DEBUG;

// Statically allocate memory for receiving messages.
static uint_least8_t message[MAX_MSG_LEN+1];

int main(void) {
    cc3_uart_init (0, SERIAL_BAUD_RATE, CC3_UART_MODE_8N1, CC3_UART_BINMODE_BINARY);
    setvbuf (stdout, NULL, _IONBF, 0);
    if (!cc3_camera_init ()) {
      cc3_led_set_state (0, true);
      exit (1);
    }
    // init filesystem driver
	cc3_filesystem_init ();
	cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);

    init_lua();
  	uint8_t mode = get_exec_mode();

    if (mode != MODE_STAND_ALONE) {
        send_debug_msg("Welcome to CamScripter Interactive mode. Click the Execute button to run a script.");
    } else {
        send_debug_msg("Starting CamScripter in Standalone mode.  About to execute installed script.");
    }
	
	if ( MODE_STAND_ALONE == mode ) {
		go_stand_alone();
	} else {
		go_interactive();
	}
    close_lua();
    return 0;
}

/**
 * puts the camera in stand alone mode. It will try to execute
 * the installed LUA code in a loop without listening on the
 * serial interface.
 */
void go_stand_alone( void ) {
	// Check if we have the file ready
	FILE *f = fopen(BYTECODE_PERM_FILE, "r");
	if ( f == NULL ) {
  		// Just in case there's somebody listening...
  		send_debug_msg("Unable to execute bytecode. Could not open file on MMC card. \nTroubleshooting:\n1. Have you installed the file?\n2. Is the MMC card installed?");
		// Indicate this special case with red + orange leds on
		cc3_led_set_state(0, true);
		cc3_led_set_state(1, true);
		exit(1);
	} else {
		fclose(f);
	}
	while(true) {
		// execute the lua code in a loop for ever
	    if ( lualib_dofile(BYTECODE_PERM_FILE) ) {
	    	// An error ocurred within LUA. Flag and halt
	    	cc3_led_set_state(0, false);
	    	cc3_led_set_state(1, true);
	    	exit(1);
	    }
	}
}

/**
 * puts the camera in interactive mode. In this mode,
 * the camera waits for commands (messages) on the serial
 * port.
 */
void go_interactive( void ) {
    while(true) {
        int len = 0;
        if ( (len =recv_packet(message,MAX_MSG_LEN)) > 0 ) {
		  	turn_on_led_for(2,300);
			ScripterMsg* s_msg = bytes_to_message(message,len);
			if ( s_msg != NULL ) {
				handle_message(s_msg);
				destroy_msg(s_msg);
	 			cc3_timer_wait_ms(20);
			}
        } else {
		  	cc3_led_set_state(1, true);
			cc3_timer_wait_ms (300);
		  	cc3_led_set_state(1, false);
            debug(".");
        }
    }
}

/**
 * Function to turn a 4-byte message into an integer.
 */
int get_int(uint8_t *bytes) {
    int j = 0;
    for (int i = 0; i < 4; i++) {
        j |= ((int) bytes[i] & 0xff) << 8*(3-i);
    }
    return j;
}


void handle_message( ScripterMsg* msg ) {
	switch(msg->id) {
		case LUA_BYTECODE_MSG_ID:
			if ( msg->parameters != NULL ) {
				cc3_button_get_and_reset_trigger(); // Just in case...
				execute_lua(msg->parameters->data.data_blob.data, 
					msg->parameters->data.data_blob.size);
			}
			break;
		case INSTALL_BYTECODE_MSG_ID:
			if ( msg->parameters != NULL ) {
				install_lua(msg->parameters->data.data_blob.data, 
					msg->parameters->data.data_blob.size);
			}
			break;
        case PIXBUF_INFO_MSG_ID:
            if ( msg->parameters != NULL ) {
                int x = get_int(msg->parameters->data.data_blob.data);
                int y = get_int(msg->parameters->next_param->data.data_blob.data);
                send_image_info(x, y);
            }
            break;
		default:
			send_debug_msg("Unknown message id [%d]",msg->id);
	}
}



/**
 * Get's the execution mode.
 * lights LED 2 and waits for the button to be pressed. If the button
 * is pressed within 3 seconds, then it will go into interactive mode.
 * Otherwise, it will go into stand alone mode.
 */
uint8_t get_exec_mode(void) {
  	uint8_t mode = MODE_STAND_ALONE;
  	uint32_t start_time = cc3_timer_get_current_ms ();
  	
  	cc3_led_set_state(2, true);
	do {
		if (cc3_button_get_and_reset_trigger() == 1) {
			// Interactive Mode flag
			mode = MODE_INTERACTIVE;
			blink_led(2,1000);
			break;
		}
	} while (cc3_timer_get_current_ms () < (start_time + 3000));
  	return mode;
}

/** non-blocking getchar **/
bool recv_char(uint_least8_t* c) {
	for( int i=0; i<20; i++ ) {
		if ( !cc3_uart_has_data (0) ) {
			*c = getchar();
			debug("`recvd <%X>",*c);
			return true;
		}
	}		
	return false;
}

void debug( char* msg, ... ) {
	if (debug_on) {
		va_list arg;
		va_start(arg, msg);
		send_debug_msg(msg, arg);
		va_end(arg);
	}
}
