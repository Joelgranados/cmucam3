#ifndef LUALIB_H_
#define LUALIB_H_

#include <lua.h>
#include <lauxlib.h>
#include <lgc.h>
#include <llex.h>
#include <lzio.h>
#include <lundump.h>
#include <stdint.h>
#include <cc3.h>
#include <stdlib.h>
#include "csmsg.h"

// Temporary file used to execute LUA chunks
#define BYTECODE_TMP_FILE	"c:/tmp.byt"
// Filename where the standalone LUA code is to be stored
#define BYTECODE_PERM_FILE	"c:/permlua.byt"

typedef struct {
  	uint32_t size_x;
  	uint32_t size_y;
  	uint8_t *row;
	char* header;
	int size;
	uint32_t x;
	uint32_t y;
} PPMData;

extern int debug_on;

// lua control methods
void init_lua(void);
void close_lua(void);
void interrupt_lua_exec(lua_State *_l, lua_Debug *ar);
void execute_lua( uint_least8_t* bytecode, size_t size );
void install_lua( uint_least8_t* bytecode, size_t size );
int lualib_dofile( const char* file );
void show_error (lua_State *_l, int status);

// picture-related methods
int take_picture(lua_State *L);
int save_picture(lua_State *_l);
void capture_ppm(FILE *f);
char* get_PPM_char(ParamBodyByteStream* bstream);
void dispose_PPM(ParamBodyByteStream* bstream);

// debugging methods
int set_debug(lua_State *_l);
int print(lua_State *L);
int print_rectangle(lua_State *_l);
int print_line(lua_State *_l);
int print_oval(lua_State *_l);
int print_color_tracker(lua_State *_l);
int clear_graphics(lua_State *_l);
int get_PPM_size(ParamBodyByteStream* bstream);
int send_picture_msg(lua_State *_l);
void send_image_info(int x, int y);

// camera and pixbuf settings methods
int camera_set_resolution(lua_State *_l);
int camera_set_colorspace(lua_State *_l);
int pixbuf_read_rows(lua_State *_l);
int pixbuf_set_coi(lua_State *_l);
int pixbuf_get_width(lua_State *_l);
int pixbuf_get_height(lua_State *_l);
int pixbuf_get_y_mid(lua_State *_l);
int pixbuf_get_x_mid(lua_State *_l);

// other methods
int is_format_8_3(const char* fname);
int file_exists(lua_State *_l);
int wait(lua_State *_l);
int turn_on_led(lua_State *_l);
int turn_off_led(lua_State *_l);
void blink_led( uint8_t led, uint32_t ms );
void turn_on_led_for( uint8_t led, uint32_t ms );

// image related methods
int image_new(lua_State *_l);
int image_dispose(lua_State *_l);
int image_get_width(lua_State *_l);
int image_get_height(lua_State *_l);
//int image_set_width(lua_State *_l);
//int image_set_height(lua_State *_l);
int image_get_channels(lua_State *_l);
int image_set_channels(lua_State *_l);

// framediff related methods
int framediff_new(lua_State *_l);
int framediff_dispose(lua_State *_l);
int framediff_get_template_width(lua_State *_l);
int framediff_get_template_height(lua_State *_l);
int framediff_get_num_pixels(lua_State *_l);
int framediff_get_threshold(lua_State *_l);
int framediff_set_threshold(lua_State *_l);
int framediff_get_total_x(lua_State *_l);
int framediff_set_total_x(lua_State *_l);
int framediff_get_total_y(lua_State *_l);
int framediff_set_total_y(lua_State *_l);
int framediff_get_coi(lua_State *_l);
int framediff_set_coi(lua_State *_l);
int framediff_get_load_frame(lua_State *_l);
int framediff_set_load_frame(lua_State *_l);
int framediff_swap_templates(lua_State *_l);

// framediff scanline methods
int framediff_scanline_start(lua_State *_l);
int framediff_scanline(lua_State *_l);
int framediff_scanline_finish(lua_State *_l);

// tracker methods
int tracker_new(lua_State *_l);
int tracker_get_centroid_x(lua_State *_l);
int tracker_get_centroid_y(lua_State *_l);
int tracker_num_pixels(lua_State *_l);
int tracker_get_density(lua_State *_l);
int tracker_get_binary_scanline(lua_State *_l);
int tracker_set_upper_bound(lua_State *_l);
int tracker_set_lower_bound(lua_State *_l);
int tracker_set_noise_filter(lua_State *_l);
int tracker_get_x0(lua_State *_l);
int tracker_get_y0(lua_State *_l);
int tracker_get_x1(lua_State *_l);
int tracker_get_y1(lua_State *_l);
int tracker_track_color(lua_State *_l);

// tracker scanline methods
int tracker_scanline_start(lua_State *_l);
int tracker_scanline(lua_State *_l);
int tracker_scanline_finish(lua_State *_l);

// pixel methods
int pixel_new(lua_State *_l);
int pixel_get_value(lua_State *_l);
int get_pixel(lua_State *_l);


// util functions to register lua functions, objects, etc
void register_tracker_struct(void);
void register_framediff_struct(void);
void register_image_struct(void);
void register_lua_constants(void);
void register_lua_functions(void);
void register_pixel_struct(void);


#endif /*LUALIB_H_*/
