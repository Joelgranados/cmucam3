#ifndef CSMSG_H_
#define CSMSG_H_

#include "stdint.h"
#include "cc3.h"

#define DEBUG_MSG_ID	10
#define PPM_MSG_ID		20
#define LUA_BYTECODE_MSG_ID		30
#define INSTALL_BYTECODE_MSG_ID	40
#define TRACKER_MSG_ID  50
#define RECT_MSG_ID 60
#define LINE_MSG_ID 61
#define OVAL_MSG_ID 62
#define CLEAR_GRAPHICS_MSG_ID 69
#define PIXBUF_INFO_MSG_ID 70

typedef uint8_t param_id_t;

// Identify which part of the union are we using
// within the MsgParam
enum param_type {
	PT_DATA_BLOB,
	PT_BYTE_STREAM
};

typedef struct {
    int r;
    int g;
    int b;
    long avg_r;
    long avg_g;
    long avg_b;
    int x;
    int y;
} image_info_pkt_t;

// DATA_BLOB body type
typedef struct {
	uint32_t size;
	uint_least8_t* data;
} ParamBodyDataBlob;

typedef struct {
	int idx;
	void* blob;
} StreamState;

// BYTE_STREAM body type
typedef struct ParamBodyByteStream_t {
	char* (*fp_get_char)(struct ParamBodyByteStream_t* param);
	int (*fp_get_size)(struct ParamBodyByteStream_t* param);
	void (*fp_dispose) (struct ParamBodyByteStream_t* param);
	StreamState state;
} ParamBodyByteStream;

// protype of the functions used to get the size
// of a BYTE_STREAM MsgParam
typedef int (*size_getter)(ParamBodyByteStream*);

// protype of the functions used to get the next
// char in a BYTE_STREAM MsgParam
typedef char* (*char_getter)(struct ParamBodyByteStream_t*);


typedef struct MsgParam_t {
	enum param_type type;
	union {
		ParamBodyDataBlob data_blob;
		ParamBodyByteStream data_stream;
	} data;
	struct MsgParam_t *next_param;
} MsgParam;

typedef struct {
	param_id_t id;
	uint8_t param_count;
	MsgParam* parameters;
} ScripterMsg;

// Testing begins
//
char* test_get_char(ParamBodyByteStream* bstream);
int test_get_size(ParamBodyByteStream* bstream);
ScripterMsg* build_debug_msg_str( char* txt );
void stream_debug_msg( char* msg, ... );
// Testing ends
//
MsgParam* create_stream_msg_param(void);
void stream_msg(ScripterMsg* msg);
uint_least8_t get_msg_id_most_repr(param_id_t id);
uint_least8_t get_msg_id_least_repr(param_id_t id);
uint_least8_t get_size_most_repr(uint32_t size);         
uint_least8_t get_size_middle_repr(uint32_t size);
uint_least8_t get_size_least_repr(uint32_t size);
uint_least8_t* s_to_u( char* txt );
ScripterMsg* create_msg(param_id_t id);
MsgParam* create_msg_param( uint_least8_t* data, uint32_t size );
int message_to_bytes( ScripterMsg msg, uint_least8_t** msg_bytes );
ScripterMsg* bytes_to_message( uint_least8_t* msg_bytes, int bytes_count );
void destroy_msg( ScripterMsg* msg );
ScripterMsg* build_debug_msg( char * txt );
void send_msg(ScripterMsg* msg);
void send_image_info_msg(image_info_pkt_t* info);
void send_debug_msg( char* msg, ... );

#endif /*CSMSG_H_*/
