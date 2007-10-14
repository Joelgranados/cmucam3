#include <stdlib.h>
#include <stdarg.h>
#include "csmsg.h"
#include "stdint.h"
#include "string.h"
#include "scripter.h"
#include "slip.h"

/**
 * MSG format is:
 * byte
 *  12	message ID
 *  3	param count
 *  456 param 1 size
 *  7->N param 1 data
 *  N+1,+2,+3 Param 2 size
 *  N+4->M Pram 2 data
 *  etc.
*/
uint_least8_t get_msg_id_most_repr(param_id_t id) {
	return id >> 8;
}

uint_least8_t get_msg_id_least_repr(param_id_t id) {
	return id & 0x00FFF;
}

uint_least8_t get_size_most_repr(uint32_t size) { 
	return (uint_least8_t)((size&0x00FFFFFF)>>16);
}

uint_least8_t get_size_middle_repr(uint32_t size) {
	return (uint_least8_t)((size&0x00FFFFFF)>>8);
}

uint_least8_t get_size_least_repr(uint32_t size) {
	return (uint_least8_t)((size&0x00FFFFFF)&0x000000FF);
}

int message_to_bytes( ScripterMsg msg, uint_least8_t** msg_bytes ) {
	// First the header
	uint_least8_t *header = malloc(3);
	// first the ID we have to put it in two bytes
	// big endian
	header[0] = get_msg_id_most_repr(msg.id);
	header[1] = get_msg_id_least_repr(msg.id);
	
	// now the param count. 1 bytes
	header[2] = msg.param_count;
	
	// now parameters
	int param_bytes_count = 0;
	MsgParam* param = msg.parameters;
	while(param != NULL) {
		param_bytes_count += 3+param->data.data_blob.size;
		param = param->next_param;
	}
	
	// Now, we put everything together in on place:
	// header + each of the params + each of the param sizes
	uint_least8_t* final_bytes = malloc( 3 + param_bytes_count);

	// Let's get the 3 bytes for teh header in now
	final_bytes[0] = header[0];
	final_bytes[1] = header[1];
	final_bytes[2] = header[2];
	free(header);
	int p_idx = 3; // parameters start at position 4
	

	param = msg.parameters;
	while(param != NULL) {
		// size is 3 bytes, so we first get
		// rid of most representative byte
		final_bytes[p_idx++] = get_size_most_repr(param->data.data_blob.size);
		final_bytes[p_idx++] = get_size_middle_repr(param->data.data_blob.size);
		final_bytes[p_idx++] = get_size_least_repr(param->data.data_blob.size);
		int i;
		for(i=0; i<(int)param->data.data_blob.size; i++) {
			final_bytes[p_idx++] = *(param->data.data_blob.data+i);
		}
		param = param->next_param;
	}
	*msg_bytes = final_bytes;
	return 3+param_bytes_count;
}

/**
 * Takes a stream of bytes and builds a ScripterMsg from it
 */
ScripterMsg* bytes_to_message( uint_least8_t* msg_bytes, int total_size ) {
	ScripterMsg* msg = malloc(sizeof(ScripterMsg));
	// ID first
	if ( total_size >= 3 ) {
		msg->id = msg_bytes[0]<<8 | msg_bytes[1];
		// Third byte gives me param count
		msg->param_count = msg_bytes[2];
	} else {
		send_debug_msg("Received less than 3 bytes. Can't create message.");
		free(msg);
		return NULL;
	}
	
	// now we parse parameters
	int i;
	int idx; // used to traverse msg_bytes. Starts after the header, 4th byte
	MsgParam* previous = NULL;
	for( i=0, idx=3; i<msg->param_count; i++ ) {
		uint32_t size;
		if ( total_size >= idx+3 ) {
			// Three bytes for the size
			size = msg_bytes[idx]<<16 | msg_bytes[idx+1]<<8 | msg_bytes[idx+2];
			idx += 3;
		} else {
			send_debug_msg("Received less than %d bytes. Can't create message",idx+3);
			free(msg);
			return NULL;
		}
		if ( idx+(int)size > total_size ) {
			send_debug_msg("Received %d bytes, but need at least %d for this message.",
				total_size, idx+size);
			free(msg);
			return NULL;
		}
		
		uint_least8_t* data = malloc(size);
		// now the content
		int j;
		for( j=0; j<(int)size; j++ ) {
			*(data+j) = msg_bytes[idx++];
		}
		MsgParam* param = create_msg_param(data,size);
		// link the parameters
		if ( previous == NULL ) {
			// this is the first parameter we parsed
			msg->parameters = param;
		} else {
			previous->next_param = param;
		}
		previous = param;
	}
	return msg;
}

/**
 * Creates a new ScripterMsg pointer.
 * @param id Message ID used to initialize the message
 */
ScripterMsg* create_msg(param_id_t id) {
	ScripterMsg* msg = malloc(sizeof(ScripterMsg));
	msg->parameters = NULL;
	msg->param_count = 0;
	msg->id = id;	
	return msg;
}

/**
 * Creates a new MsgParam pointer.
 * @param data a pointer to the data this parameter is holding
 * @param size size of the data this paramter holds
 */
MsgParam* create_msg_param( uint_least8_t* data, uint32_t size ) {
	MsgParam* param = malloc(sizeof(MsgParam));
	param->type = PT_DATA_BLOB;
	param->data.data_blob.size = size;
	param->data.data_blob.data = data;
	param->next_param = NULL;
	return param;
}

MsgParam* create_stream_msg_param() {
	MsgParam* param = malloc(sizeof(MsgParam));
	param->type = PT_BYTE_STREAM;
	param->next_param = NULL;
	// Initialize the iterator index
	param->data.data_stream.state.idx = 0;
	return param;
}


/**
 * Helper method to build a debug msg.
 * txt is the string used to build the debug msg
 */
ScripterMsg* build_debug_msg( char* txt ) {
	ScripterMsg* msg = create_msg(DEBUG_MSG_ID);
	msg->param_count = 1;
	MsgParam* param = create_msg_param(s_to_u(txt), strlen(txt));
	msg->parameters = param;
	return msg;
}

/**
 * Frees up memory used by the ScripterMsg
 * msg ScripterMsg to be destroyed
 */
void destroy_msg( ScripterMsg* msg ) {
	if ( msg == NULL ) return;
	MsgParam* param = msg->parameters;
	while( param != NULL ) {
		MsgParam* aux = param->next_param;
		if ( param->type == PT_BYTE_STREAM ) {
			if ( param->data.data_stream.fp_dispose != NULL ) {
				//call dispose function
				(*(param->data.data_stream.fp_dispose))(&param->data.data_stream);
			} else {
				free(param->data.data_stream.state.blob);
			}
		} else {
			free(param->data.data_blob.data);
		}
		free(param);
		param = aux;
	}
	free(msg);
}

/**
 * Converts a string into a byte array
 */
uint_least8_t* s_to_u( char* txt ) {
	uint_least8_t* bytes = malloc(strlen(txt));
	size_t i=0;
	for(i=0; i<strlen(txt); i++) {
		bytes[i] = txt[i];
	}
	return bytes;
}

/**
 * Sends a ScripterMsg through the serial link
 * using SLIP framing.
 * @param msg ScripterMsg pointer to be sent across
 */
void send_msg(ScripterMsg* msg) {
	uint_least8_t* msg_bytes;
	int size = message_to_bytes(*msg,&msg_bytes);
	send_packet(msg_bytes,size);
	free(msg_bytes);
}

/**
 * Streams a message instead of getting the entire msg as a byte array
 * and then sending the bytes accross.
 */
void stream_msg(ScripterMsg* msg) {
	init_packet();
	// send header
	put_slip_char(get_msg_id_most_repr(msg->id));
	put_slip_char(get_msg_id_least_repr(msg->id));
	put_slip_char(msg->param_count);
		
	// send parameters
	MsgParam* param = msg->parameters;
	while(param != NULL) {
		// size is 3 bytes, so we first get
		// rid of most representative byte
		if (param->type == PT_BYTE_STREAM) {
			int size = (*(param->data.data_stream.fp_get_size))(&param->data.data_stream);
			put_slip_char(get_size_most_repr(size));
			put_slip_char(get_size_middle_repr(size));
			put_slip_char(get_size_least_repr(size));
			char *c = NULL;
			while( (c =(*(param->data.data_stream.fp_get_char))(&param->data.data_stream)) != NULL ) {
				put_slip_char(*c);
			}
		} else {
			put_slip_char(get_size_most_repr(param->data.data_blob.size));
			put_slip_char(get_size_middle_repr(param->data.data_blob.size));
			put_slip_char(get_size_least_repr(param->data.data_blob.size));
			int i;
			for(i=0; i<(int)param->data.data_blob.size; i++) {
				put_slip_char(*(param->data.data_blob.data+i));
			}
		}
		param = param->next_param;
	}
	finish_packet();	
}

void send_debug_msg( char* msg, ... ) {
	va_list arg;
	va_start(arg, msg);
	char* txt = malloc(1024); //1K worth of debug is a good start
	vsprintf(txt, msg, arg); //Pay Attention: vprintf - not printf
	va_end(arg);
	ScripterMsg* s_msg = build_debug_msg(txt);
	send_msg(s_msg);
	destroy_msg(s_msg);
	free(txt);
}

/**
 * Send detailed information about a particular pixel -- it's rgb values.
 */
void send_image_info_msg(image_info_pkt_t* info) {
    char txt[64]; // should be plenty
    sprintf(txt, "R,G,B:(%d,%d,%d) Average in area:(%d,%d,%d)", 
            info->r, info->g, info->b, info->avg_r, info->avg_g, info->avg_b, info->x, info->y);

    ScripterMsg* msg = build_debug_msg(txt);
    msg->id = PIXBUF_INFO_MSG_ID;
    send_msg(msg);
    destroy_msg(msg);
    free(info);
}
