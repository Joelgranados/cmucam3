#include "lualib.h"
#include "string.h"
#include "scripter.h"
#include "slip.h"
#include <cc3_frame_diff.h>
#include <cc3_color_track.h>
#include <cc3.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


/* State information so that get_pixel() doesn't have to always grab 
 * This is cleared by take_picture()
 */
cc3_image_t g_img;
int g_img_prev_x, g_img_prev_y;

    
/* type-safety helper macros - checks that the object passed in is the correct type of pointer */
#define checkimage(_l) (cc3_image_t*) luaL_checkudata(_l, 1, "scripter.image")
#define checkframediff(_l, pos) (cc3_frame_diff_pkt_t*) luaL_checkudata(_l, pos, "scripter.framediff")
#define checktracker(_l, pos) (cc3_track_pkt_t*) luaL_checkudata(_l, pos, "scripter.tracker")
#define checkpixel(_l, pos) (cc3_pixel_t*) luaL_checkudata(_l, pos, "scripter.pixel")

/* macro to register C #defines with Lua */
#define lua_setconst(_l, name) { lua_pushnumber(_l, name); lua_setglobal(_l, #name); }

/* macro to replicate itoa function (non-standard c way of converting int to ascii) */
static lua_State *L;

/* Struct containing all our image methods.
   Example: i = image.new();
   i.get_width();  // get_width() is the method
*/
static const struct luaL_Reg imagelib_m[] = {
    {"dispose", image_dispose},
    {"get_width", image_get_width},
    {"get_height", image_get_height},
    {"get_channels", image_get_channels},
    {"set_channels", image_set_channels},
    {NULL, NULL}
};

/* Struct containing all framediff methods */
static const struct luaL_Reg framedifflib_m[] = {
    {"dispose", framediff_dispose},
    {"get_num_pixels", framediff_get_num_pixels},
    {"get_template_width", framediff_get_template_width},
    {"get_template_height", framediff_get_template_height},
    {"get_threshold", framediff_get_threshold},
    {"set_threshold", framediff_set_threshold},
    {"get_total_x", framediff_get_total_x},
    {"set_total_x", framediff_set_total_x},
    {"get_total_y", framediff_get_total_y},
    {"set_total_y", framediff_set_total_y},
    {"get_coi", framediff_get_coi},
    {"set_coi", framediff_set_coi},
    {"get_load_frame", framediff_get_load_frame},
    {"set_load_frame", framediff_set_load_frame},
    {"swap_templates", framediff_swap_templates},
    {NULL, NULL}
};


/* Struct containing all tracking methods */
static const struct luaL_Reg trackerlib_m[] = {
    {"get_centroid_x", tracker_get_centroid_x},
    {"get_centroid_y", tracker_get_centroid_y},
    {"get_x0", tracker_get_x0},
    {"get_y0", tracker_get_y0},
    {"get_x1", tracker_get_x1},
    {"get_y1", tracker_get_y1},
    {"get_num_pixels", tracker_num_pixels},
    {"get_density", tracker_get_density},
    {"get_binary_scanline", tracker_get_binary_scanline},
    {"set_lower_bound", tracker_set_lower_bound},
    {"set_upper_bound", tracker_set_upper_bound},
    {"set_noise_filter", tracker_set_noise_filter},
    {NULL, NULL}
};

/* struct containing all pixel methods */
static const struct luaL_Reg pixellib_m[] = {
    {"get_value", pixel_get_value},
    {NULL, NULL}
};


/**
 * Initialize the camscripter API we're exposing to Lua
 */
void init_lua() {
    /* open lua state */
	L=lua_open();
    
    // open lua libraries as needed/wanted (not all can fit on the scripter)
    //luaopen_base(L);
    //luaopen_table(L);
    //luaopen_math(L);
    //luaopen_string(L);
    //luaopen_package(L);
    //luaopen_os(L);
    //luaopen_io(L);
    //luaopen_debug(L);

    // register our functions, constants, and structs
    register_lua_functions();
    register_lua_constants();
    register_image_struct();
    register_framediff_struct();
    register_tracker_struct();
    register_pixel_struct();

    
	// register call back for interrupting LUA execution
	lua_sethook(L, interrupt_lua_exec, LUA_MASKLINE, 0);

    // lessen the pause length to make the lua garbage collector more aggressive
    lua_gc(L, LUA_GCSETPAUSE, 125);  // default is 200
}


/**
 *  Close Lua state 
 */
void close_lua() {
	if ( L != NULL ) {
		lua_close(L);
	}
}


/**
 * Register all lua functions for the CMUcam3
 */
void register_lua_functions(void) {
    /* Register assorted lua functions */
	lua_register(L, "take_picture",take_picture);
    lua_register(L, "save_picture", save_picture);
	lua_register(L, "set_debug",set_debug);
	lua_register(L, "print",print);
	lua_register(L, "print_picture",send_picture_msg);
	lua_register(L, "print_color_tracker",print_color_tracker);
    lua_register(L, "print_rectangle", print_rectangle);
    lua_register(L, "print_line", print_line);
    lua_register(L, "print_oval", print_oval);
    lua_register(L, "clear_graphics", clear_graphics);
    lua_register(L, "file_exists", file_exists);
    lua_register(L, "camera_set_resolution", camera_set_resolution);
    lua_register(L, "camera_set_colorspace", camera_set_colorspace);
    lua_register(L, "wait", wait);
    lua_register(L, "pixbuf_set_coi", pixbuf_set_coi);
    lua_register(L, "pixbuf_read_rows", pixbuf_read_rows);
    lua_register(L, "pixbuf_get_width", pixbuf_get_width);
    lua_register(L, "pixbuf_get_height", pixbuf_get_height);
    lua_register(L, "framediff_scanline_start", framediff_scanline_start);
    lua_register(L, "framediff_scanline", framediff_scanline);
    lua_register(L, "framediff_scanline_finish", framediff_scanline_finish);
    lua_register(L, "turn_on_led", turn_on_led);
    lua_register(L, "turn_off_led", turn_off_led);
    lua_register(L, "tracker_track_color", tracker_track_color);
    lua_register(L, "image_new", image_new);
    lua_register(L, "framediff_new", framediff_new);
    lua_register(L, "tracker_new", tracker_new);
    lua_register(L, "pixel_new", pixel_new);
    lua_register(L, "get_pixel", get_pixel);
}

/**
 * Register any C #defines and constants we want to make available to the 
 * CamScripter in Lua.
 */
void register_lua_constants(void) {
    // register #defines
    lua_setconst(L, CC3_CAMERA_RESOLUTION_LOW);
    lua_setconst(L, CC3_CAMERA_RESOLUTION_HIGH);
    lua_setconst(L, CC3_CHANNEL_SINGLE);
    lua_setconst(L, CC3_CHANNEL_RED);
    lua_setconst(L, CC3_CHANNEL_GREEN);
    lua_setconst(L, CC3_CHANNEL_BLUE);
    lua_setconst(L, CC3_CHANNEL_Y);
    lua_setconst(L, CC3_CHANNEL_CR);
    lua_setconst(L, CC3_CHANNEL_CB);
    lua_setconst(L, CC3_CHANNEL_HUE);
    lua_setconst(L, CC3_CHANNEL_SAT);
    lua_setconst(L, CC3_CHANNEL_VAL);
    lua_setconst(L, CC3_CHANNEL_ALL);
    lua_setconst(L, CC3_COLORSPACE_YCRCB);
    lua_setconst(L, CC3_COLORSPACE_RGB);
}

/**
 * Make the image struct available to the CamScripter lua side.
 */
void register_image_struct(void) {
    luaL_newmetatable(L, "scripter.image"); // register image metatable

    /* set scripter image gc */
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, image_dispose);
    lua_settable(L, -3);
    
    // register methods on image struct
    lua_pushvalue(L, -1); // duplicate the metatable
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, imagelib_m); // image methods
}

/**
 * Make the framediff struct available to the Camscripter.
 */
void register_framediff_struct(void) {
    // register the framediff image data type
    luaL_newmetatable(L, "scripter.framediff"); // register framediff metatable
    
    // set framediff garbage collector function
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, framediff_dispose);
    lua_settable(L, -3);

    // register the framediff methods
    lua_pushvalue(L, -1); // duplicate the table
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, framedifflib_m); // framediff methods
}

/**
 * Make the cc3_tracker_pkt_t struct available to CamScripter.
 */
void register_tracker_struct(void) {
    // register the tracker data type
    luaL_newmetatable(L, "scripter.tracker"); // register tracker metatable
    lua_pushvalue(L, -1); // duplicate the metatable
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, trackerlib_m); // tracker methods
}

/**
 * Register the cc3_pixel_t datatype for Camscripter 
 */
void register_pixel_struct(void) {
    luaL_newmetatable(L, "scripter.pixel"); // pixel metatable
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, pixellib_m); // pixel methods
}


void interrupt_lua_exec(lua_State *_l, lua_Debug *ar) {
  	// Only listen to "Hook Lines" events
  	if(ar->event == LUA_HOOKLINE) {
    	// Check the button state to know if we should abort
  		if (cc3_button_get_and_reset_trigger() == 1) {
	  		lua_pushstring(_l, "Button pressed: Abort requested!");
	  		lua_error(_l);
  		}
    }
}


/**
 * Sends Lua error message
 */
void show_error (lua_State *_l, int status) {
  const char *msg;
  msg = lua_tostring(_l, -1);
  if (msg == NULL) msg = "(error with no message)";
  send_debug_msg("Error executing Lua!\nError code=%d, %s\n", status, msg);

}

/**
 * Helper function that executes the given file
 * using our global LUA context.
 */
int lualib_dofile( const char* file ) {
	return luaL_dofile(L,file);
}

/**
 * Execute Lua byte code by first creating a file on the MMC/SD card and then
 * passing that file to Lua interpreter.
 */
void execute_lua( uint_least8_t* bytecode, size_t size ) {
	FILE *f = fopen(BYTECODE_TMP_FILE, "w");
	if ( f == NULL ) {
  		send_debug_msg("Unable to execute bytecode. Could not open tmp file on MMC card. Is the MMC card installed?");
		return;
	}
	int status;
    size_t i;

	for(i=0; i<size; i++) {
		fputc(bytecode[i],f);
	}
	fclose(f);
    
    // execute the lua file, and show any errors 
    send_debug_msg("Executing Lua code...");
    status = lualib_dofile(BYTECODE_TMP_FILE);
    if (status) {
        show_error(L, i);
    } else {
        send_debug_msg("Lua execution finished!");
    }
}

/**
 * Installs Lua byte code by creating a file on the MMC/SD card.
 * This code will be executed in a loop if the camera is started
 * the next time in standalone mode.
 */
void install_lua( uint_least8_t* bytecode, size_t size ) {
	FILE *f = fopen(BYTECODE_PERM_FILE, "w");
	if ( f == NULL ) {
  		send_debug_msg("Unable to install bytecode. Could not open file on MMC card. Is the MMC card installed?");
		return;
	}
    size_t i;
	for(i=0; i<size; i++) {
		fputc(bytecode[i],f);
	}
	fclose(f);
	send_debug_msg("LUA bytecode installed successfully!");
}

/**
 * Set debug in Lua
 */
int set_debug(lua_State *_l) {
    luaL_checkany(_l, 1); /* anything can be boolean in lua, just make sure 
                             we got something */
	debug_on = lua_toboolean(_l, 1);
	return 0;
}

/**
 * Implementation of Lua's print function
 */
int print(lua_State *_l) {
    if (debug_on) {
        int n=lua_gettop(_l);
        int i;
        for (i=1; i<=n; i++) {
            if (lua_isstring(_l,i)) {
                send_debug_msg("%s",lua_tostring(_l,i));
            } else if (lua_isnil(_l,i)==2) {
                send_debug_msg("%s","nil");
            } else if (lua_isboolean(_l,i)) {
                send_debug_msg("%s",lua_toboolean(_l,i) ? "true" : "false");
            } else {
   			send_debug_msg("%s:%p",luaL_typename(_l,i),lua_topointer(_l,i));
            }
        }
    }
	return 0;
}

/**
 * Set the camera's resolution.
 * Parameters: The desired resolution, must be either 
 * CC3_CAMERA_RESOLUTION_HIGH or CC3_CAMERA_RESOLUTION_LOW.
 */
int camera_set_resolution(lua_State *_l) {
    int res = luaL_checkint(_l, 1); /* get the parameter */
    
    // resolution must be either 0 (low) or 1 (high)
    luaL_argcheck(_l, (res == 0 || res == 1), 1, 
                  "Resolution must be either CC3_CAMERA_RESOLUTION_HIGH or CC3_CAMERA_RESOLUTION_LOW.");
    cc3_camera_set_resolution(res);
    return 0;
}

/**
 * Set the camera's colorspace. Must be one of CC3_COLORSPACE_YCRCB, or 
 * CC3_COLORSPACE_RGB.
 */
int camera_set_colorspace(lua_State *_l) {
    int cs = luaL_checkint(_l, 1);

    // parameter must be either YCRCB or RGB
    luaL_argcheck(_l, (cs == CC3_COLORSPACE_YCRCB || cs == CC3_COLORSPACE_RGB),
                  1, "Colorspace must be either CC3_COLORSPACE_YCRCB or CC3_COLORSPACE_RGB.");
    cc3_camera_set_colorspace(cs);
    return 0; // no return values
}

/**
 * Figure out if the passed file already exists, and return a boolean.
 * 
 * Parameters: Get one parameter which should be a string, the name of 
 * the file to check. That filename must be fewer than 28 characters
 */
int file_exists(lua_State *_l) {
    FILE *f;
    const char *path = luaL_checkstring(_l, 1); /* get the parameter */
    luaL_argcheck(_l, strlen(path) < 13, 1, 
                  "Filename must be less than 13 characters.");
 
    // put C:/ in front of our filename
    char fname[20];
	strcpy(fname,"c:/");
	strcat(fname,path);
    
    f = fopen(fname, "r");
    lua_pushboolean(_l, (f != NULL));
    fclose(f);

    return 1;
}

//////////////////////////////////////////////////////
// START FUNCTIONS ASSOCIATED WITH THE CC3_PIXEL_T  //
//////////////////////////////////////////////////////

/**
 * Get a new pixel object
 */
int pixel_new(lua_State *_l) {
    cc3_pixel_t *pixel;
    
    pixel = (cc3_pixel_t*) lua_newuserdata(_l, sizeof(cc3_pixel_t));
    pixel->channel[0] = pixel->channel[1] = pixel->channel[2] = 0;

    luaL_getmetatable(_l, "scripter.pixel");
    lua_setmetatable(L, -2);
    return 1; // pixel's on the stack 
}

/**
 * Get the value of one of the channels for a single pixel.
 */
int pixel_get_value(lua_State *_l) {
    cc3_pixel_t *pixel = checkpixel(_l, 1);
    uint8_t channel = luaL_checknumber(_l, 2);
    
    // don't need to check < 0 for uint8_t
    luaL_argcheck(_l, (channel < 3), 2, "Channel must be oe of 0, 1, 2");
    lua_pushinteger(_l, pixel->channel[channel]);

    return 1; 
}

//////////////////////////////////////////////////////
// END FUNCTIONS ASSOCIATED WITH THE CC3_PIXEL_T    //
//////////////////////////////////////////////////////


//////////////////////////////////////////////////////
// START FUNCTIONS ASSOCIATED WITH THE CC3_IMAGE_T  //
//////////////////////////////////////////////////////

/** 
 * Get a new image struct. Returns a pointer to a cc3_image_t
 */
int image_new(lua_State *_l) {
    cc3_image_t *i;
    int num_bytes;
    uint16_t width, height;
    uint32_t pixsize;

    width = luaL_checknumber(_l, 1);
    height = luaL_checknumber(_l, 2);
    pixsize = width*height;

    num_bytes = sizeof(cc3_image_t);

    i = (cc3_image_t*) lua_newuserdata(_l, num_bytes);
    i->width = width;
    i->height = height;
    i->pix = malloc(pixsize);
    if (i->pix == NULL) {
        send_debug_msg("Error allocating memory for the images's pix");
    }
    i->channels = CC3_CHANNEL_ALL;

    luaL_getmetatable(_l, "scripter.image");
    lua_setmetatable(L, -2);
    return 1; // new userdata is already on the lua stack 
}

/**
 * Free all memory associated with this image.
 */
int image_dispose(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);

    if (img != NULL && img->pix != NULL) {
        free(img->pix);
        img->pix = NULL;
    }

    return 0;
}

/**
 * Get the height of the cc3_image_t passed as first param.
 */
int image_get_height(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    lua_pushinteger(_l, img->height);
    return 1;
}

/**
 * Set the height of the cc3_image_t passed as first param to the value
 * passed in the second parameter.

NOTE: commenting these out.  For now, width and height are
immutable, since I'm allocating memory based on them.  If we change that,
these two functions need to do something about the memory allocated to 
img->pix, when they get changed. (It didn't seem like they ever got 
changed, in the security cam use case).

int image_set_height(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    img->height = luaL_checknumber(_l, 2);
    return 0;
}

 * Set the width of the cc3_image_t passed as the first param to the value
 * passed in the second parameter.
int image_set_width(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    img->width = luaL_checknumber(_l, 2);
    return 0;
}

*/

/**
 * Get the channels of interest of the cc3_image_t passed as first param.
 */
int image_get_channels(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    lua_pushinteger(_l, img->channels);
    return 1;
}

/**
 * Set the channels of the cc3_image_t passed as first param to the value
 * passed in the second parameter.
 */
int image_set_channels(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    img->channels = luaL_checknumber(_l, 2);
    return 0;
}


/**
 * Get the width of the cc3_image_t passed as first param
 */
int image_get_width(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    lua_pushinteger(_l, img->width);
    return 1;
}

/**
 * Get the pixel located at x, y in the image data structure.  Just stores
 * the values at that pixel into the passed-in pixel data struct.
 */
int get_pixel(lua_State *_l) {
    cc3_pixel_t *pixel = checkpixel(_l, 1);
    int x = luaL_checknumber(_l, 2);
    int y = luaL_checknumber(_l, 3);

    // verify the x, y values
    luaL_argcheck(_l, x < cc3_g_pixbuf_frame.width, 2, 
                   "x is outside the bounds of the image.");
    luaL_argcheck(_l, y < cc3_g_pixbuf_frame.height, 3, 
                  "y is outside the bounds of the image");

    // Only rewind the frame if we are accessing data behind current point in FIFO 
    if(g_img_prev_y<y || g_img_prev_y==-1) { cc3_pixbuf_rewind();   g_img_prev_y=-1; }
   
    // only load a new row if we need to 
    int idy = g_img_prev_y;
    while( idy<y) {
        cc3_pixbuf_read_rows(g_img.pix, 1);
        idy++;
    } 

    cc3_get_pixel(&g_img, x, 0, pixel); 
    // update position in global image
    g_img_prev_y=idy;
    g_img_prev_x=x;

    return 0;
}

//////////////////////////////////////////////////////////////
// START FUNCTIONS ASSOCIATED WITH THE CC3_FRAME_DIFF_PKT_T //
//////////////////////////////////////////////////////////////

/** 
 * Get a new frame diff packet as a full userdata.  Returns a frame diff 
 * packet with the previous and current templates set to template_width * 
 * template_height size. 
 */
int framediff_new(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt;
    uint16_t tw, th;
    int template_size;
    tw = luaL_checknumber(_l, 1);
    th = luaL_checknumber(_l, 2);

    template_size = tw * th * sizeof(uint32_t);
    pkt = (cc3_frame_diff_pkt_t*) lua_newuserdata(_l, sizeof(cc3_frame_diff_pkt_t));

    if (pkt == NULL) {
        send_debug_msg("Error allocating memory for the new framediff.");
    }

    pkt->template_width = tw;
    pkt->template_height = th;
    pkt->current_template = (uint32_t*) malloc(template_size);
    if (pkt->current_template == NULL) {
        send_debug_msg("Error allocating memory for the framediff's current template");
    }
    pkt->previous_template = (uint32_t*) malloc(template_size);
    if (pkt->previous_template == NULL) {
        send_debug_msg("Error allocating memory for the framediff's previous template");
    }

    // initialize all other fields
    pkt->x0 = pkt->x1 = pkt->y0 = pkt->y1 = 0;
    pkt->centroid_x = pkt->centroid_y = 0;
    pkt->int_density = pkt->total_x = pkt->total_y = 0;
    pkt->num_pixels = pkt->threshold = 0;
    pkt->load_frame = false;
    pkt->coi = CC3_CHANNEL_ALL;

    luaL_getmetatable(_l, "scripter.framediff");
    lua_setmetatable(L, -2);
    return 1; // pkt's already on the stack, thanks to the newuserdata call
}

/**
 * Dispose of the memory associated with a frame_diff_pkt. Can be called by 
 * users, or gets called by lua's garbage collector when a framediff object
 * goes out of scope.
 */
int framediff_dispose(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);

    if (pkt != NULL && pkt->current_template != NULL) {
        free(pkt->current_template);
        pkt->current_template = NULL;
    }
    if (pkt != NULL && pkt->previous_template != NULL) {
        free(pkt->previous_template);
        pkt->previous_template = NULL;
    }
    return 0;
}

/**
 * Get the template width of this framediff packet
 */
int framediff_get_template_width(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->template_width);
    return 1;
}

/**
 * Get the template height of this framediff packet
 */
int framediff_get_template_height(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->template_height);
    return 1;
}

/**
 * Get the diff (in number of pixels) from this frame_diff_pkt
 */
int framediff_get_num_pixels(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->num_pixels);
    return 1;
}

/**
 * Get the threshold for this frame_diff_pkt
 */
int framediff_get_threshold(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->threshold);
    return 1;
}

/**
 * Set the threshold for this frame_diff_pkt
 */
int framediff_set_threshold(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    pkt->threshold = luaL_checkint(_l, 2); /* get the parameter */
    return 0;
}

/**
 * Get the total_x value for this frame_diff_pkt
 */
int framediff_get_total_x(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->total_x);
    return 1;
}

/**
 * Set total_x for this frame_diff_pkt
 */
int framediff_set_total_x(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    pkt->total_x = luaL_checkint(_l, 2); /* get the parameter */
    return 0;
}

/**
 * Get the total_y value for this frame_diff_pkt
 */
int framediff_get_total_y(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->total_y);
    return 1;
}

/**
 * Set total_y for this frame_diff_pkt
 */
int framediff_set_total_y(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    pkt->total_y = luaL_checkint(_l, 2); /* get the parameter */
    return 0;
}

/**
 * Get the channels of interest value for this frame_diff_pkt
 */
int framediff_get_coi(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushinteger(_l, pkt->coi);
    return 1;
}

/**
 * Set channels_of_interest for this frame_diff_pkt
 */
int framediff_set_coi(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    int coi = luaL_checkint(_l, 2); /* get the parameter */
    luaL_argcheck(_l, (coi >= 0 && coi <= 3), 1, 
                  "COI must be between 0 (single) and 3 (all channels).");
    pkt->coi = coi;
    return 0;
}

/**
 * Do we want to load a frame for this packet?
 */
int framediff_get_load_frame(lua_State *_l) { 
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    lua_pushboolean(_l, pkt->load_frame);
    return 1;
}

/**
 * Set load_frame for this packet
 */
int framediff_set_load_frame(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    luaL_checkany(_l, 2); /* anything can be boolean in lua, just make sure 
                             we got something */
    pkt->load_frame = lua_toboolean(_l, 2);
    return 0;
}

/**
 * Swap the current and previous templates in the framediff object.
 */
int framediff_swap_templates(lua_State *_l) {
    void *tmp;
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    
    tmp = pkt->previous_template;
    pkt->previous_template = pkt->current_template;
    pkt->current_template = tmp;

    return 0;
}

/**
 * Wrapper for the cc3 library's cc3_frame_diff_scanline_start function.
 * Just gets the framediff object passed in from lua, calls the c function,
 * and then sticks the integer returned onto the lua stack.
 */
int framediff_scanline_start(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    int rv = cc3_frame_diff_scanline_start(pkt);
    lua_pushinteger(_l, rv);
    
    return 1; // one return value
}

/**
 * Wrapper for the cc3 library's cc3_frame_diff_scanline_finish function.
 * Just gets the framediff object passed in from lua, calls the c function,
 * and then sticks the integer returned onto the lua stack.
 */
int framediff_scanline_finish(lua_State *_l) {
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 1);
    int rv = cc3_frame_diff_scanline_finish(pkt);
    lua_pushinteger(_l, rv);

    return 1; // one return value
}

/**
 * Wrapper for the cc3 library's cc3_frame_diff_scanline function.
 * Just gets the framediff object and the image object passed in from lua,
 * calls the c function and then sticks the integer returned onto the stack.
 */
int framediff_scanline(lua_State *_l) {
    cc3_image_t *img = checkimage(_l);
    cc3_frame_diff_pkt_t *pkt = checkframediff(_l, 2);

    int rv = cc3_frame_diff_scanline(img, pkt);
    lua_pushinteger(_l, rv);
    
    return 1;
}


/////////////////////////////////////////////////////////
// START FUNCTIONS ASSOCIATED WITH THE CC3_TRACK_PKT_T //
/////////////////////////////////////////////////////////

/** 
 * Get a new track packet as a full userdata.  Returns a track 
 * packet.
 */
int tracker_new(lua_State *_l) {
	cc3_track_pkt_t *pkt;
	uint8_t minred, mingreen, minblue, maxred, maxgreen, maxblue;
    
    pkt = (cc3_track_pkt_t*) lua_newuserdata(_l, sizeof(cc3_track_pkt_t));
    if (pkt == NULL) {
        send_debug_msg("Error allocating memory for the new tracker.");
    }
    
    minred = luaL_checknumber(_l, 1);
    mingreen = luaL_checknumber(_l, 2);
    minblue = luaL_checknumber(_l, 3);
    maxred = luaL_checknumber(_l, 4);
    maxgreen = luaL_checknumber(_l, 5);
    maxblue = luaL_checknumber(_l, 6);
    
    pkt->lower_bound.channel[0] = minred;
    pkt->lower_bound.channel[1] = mingreen;
    pkt->lower_bound.channel[2] = minblue;
    pkt->upper_bound.channel[0] = maxred;
    pkt->upper_bound.channel[1] = maxgreen;
    pkt->upper_bound.channel[2] = maxblue;
    
    pkt->x0 = pkt->x1 = pkt->y0 = pkt->y1 = pkt->scratch_x = pkt->scratch_y = 0;
    pkt->centroid_x = pkt->centroid_y = 0;
    pkt->num_pixels = pkt->int_density = 0;
    pkt->noise_filter = 2; // default in cmucam2, duplicating
    pkt->track_invert = false;
    pkt->binary_scanline[0] = 0;
  	
  	luaL_getmetatable(_l, "scripter.tracker");
    lua_setmetatable(L, -2);
    return 1;
}

/**
 * Get the centroid x of the object being tracked
 */
int tracker_get_centroid_x(lua_State *_l) { 
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->centroid_x);
    return 1;
}

/**
 * Get the centroid y of the object being tracked
 */
int tracker_get_centroid_y(lua_State *_l) { 
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->centroid_y);
    return 1;
}

/**
 * Get x0 from the tracker (upper left hand corner of bounding box)
 */
int tracker_get_x0(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->x0);
    return 1;
}

/**
 * Get y0 from the tracker (upper left hand corner of bounding box)
 */
int tracker_get_y0(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->y0);
    return 1;
}

/**
 * Get x1 from the tracker (lower right hand corner of bounding box)
 */
int tracker_get_x1(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->x1);
    return 1;
}

/**
 * Get y1 from the tracker (lower right hand corner of bounding box)
 */
int tracker_get_y1(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->y1);
    return 1;
}

/**
 * Get the number of pixel that is part of the tracked object
 */
int tracker_num_pixels(lua_State *_l) { 
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->num_pixels);
    return 1;
}

/**
 * Get the density of the tracked object
 */
int tracker_get_density(lua_State *_l) { 
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_pushinteger(_l, pkt->int_density);
    return 1;
}

/**
 * Get binary scanline of the tracked object
 */
int tracker_get_binary_scanline(lua_State *_l) { 
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    lua_newtable(_l);
  	for(int i = 0;i < MAX_BINARY_WIDTH;i++){
    	lua_pushinteger(_l, pkt->binary_scanline[i]);
    	lua_rawseti(_l,-2,i + 1);
  	}
    return 1;
}


/**
 * Wrapper for the cc3_track_color function.
 */
int tracker_track_color(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    int rv = cc3_track_color(pkt);
    lua_pushboolean(_l, rv);
    
    return 1; 
}


/**
 * Function called from within Lua to set the upper bound on a tracker 
 * object.  Passes the tracker object, the upper bound to set, and the 
 * channel it's setting the upper bound for.
 */
int tracker_set_upper_bound(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    uint8_t channel = luaL_checkinteger(_l, 2);
    int bound = luaL_checkinteger(_l, 3);

    // make sure we got legal arguments
    luaL_argcheck(_l, channel < 3, 2, "Channel can only be 0, 1 or 2");

    pkt->upper_bound.channel[channel] = bound;
    return 0;
}

/**
 * Set the noise filter property for the tracker. Argument must be 
 * between 0 and 255.
 */
int tracker_set_noise_filter(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    uint16_t filter = luaL_checkinteger(_l, 2);
    luaL_argcheck(_l, filter <= 255, 2, "Filter must be between 0 and 255.");

    pkt->noise_filter = filter;
    return 0;
}

/**
 * Function called from within Lua to set the lower bound on a tracker 
 * object.  Passes the tracker object, the bound to set, and the 
 * channel it's setting the lower bound for.
 * Color is an optional argument.
 */
int tracker_set_lower_bound(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);
    uint8_t channel = luaL_checkinteger(_l, 2);
    int bound = luaL_checkinteger(_l, 3);

    // make sure we got legal arguments
    luaL_argcheck(_l, channel < 3, 2, "Channel can only be 0, 1 or 2");

    pkt->lower_bound.channel[channel] = bound;
    return 0;
}


/**
 * Function called from inside lua to print a rectangle on the CamScripter.
 * Send over the rectangle information (width, height, x, y).
 * Color is an optional argument.
 */
int print_rectangle(lua_State *_l) {
    int x, y, width, height;
    int red, green, blue;

    if (!debug_on) return 0;

    // get all the arguments
    x = luaL_checkint(_l, 1);
    y = luaL_checkint(_l, 2);
    width = luaL_checkint(_l, 3);
    height = luaL_checkint(_l, 4);
    red = luaL_optnumber(_l, 5, 0); // colors are optional, 0 is the default
    green = luaL_optnumber(_l, 6, 0);
    blue = luaL_optnumber(_l, 7, 0);

    // now push them across to the CamScripter
    init_packet();
    
    put_slip_char(get_msg_id_most_repr(RECT_MSG_ID));
    put_slip_char(get_msg_id_least_repr(RECT_MSG_ID));

    put_slip_char(7); // number of params

    // now the parameters
    put_slip_integer(x);
    put_slip_integer(y);
    put_slip_integer(width);
    put_slip_integer(height);
    put_slip_integer(red);
    put_slip_integer(green);
    put_slip_integer(blue);

    finish_packet();
    return 0;
}

/**
 * Function called from inside lua to print a line on the CamScripter.
 * Send over the relevant information (startx, starty, endx, endy).
 */
int print_line(lua_State *_l) {
    int start_x, start_y, end_x, end_y;
    int red, green, blue;

    if (!debug_on) return 0;

    start_x = luaL_checkint(_l, 1);
    start_y = luaL_checkint(_l, 2);
    end_x = luaL_checkint(_l, 3);
    end_y = luaL_checkint(_l, 4);
    red = luaL_optnumber(_l, 5, 0); // colors are optional, 0 is the default
    green = luaL_optnumber(_l, 6, 0);
    blue = luaL_optnumber(_l, 7, 0);

    // now push them across to the CamScripter
    init_packet();
    
    put_slip_char(get_msg_id_most_repr(LINE_MSG_ID));
    put_slip_char(get_msg_id_least_repr(LINE_MSG_ID));

    put_slip_char(7); // number of params

    // now the parameters
    put_slip_integer(start_x);
    put_slip_integer(start_y);
    put_slip_integer(end_x);
    put_slip_integer(end_y);
    put_slip_integer(red);
    put_slip_integer(green);
    put_slip_integer(blue);

    finish_packet();
    return 0;
}

/**
 * Function called from inside lua to print an oval on the CamScripter. 
 * Send over the oval information (upper_left_x, upper_left_y, width, height).
 * Color is an optional argument.
 */
int print_oval(lua_State *_l) {
    int x, y, width, height;
    int red, green, blue;

    if (!debug_on) return 0;

    x = luaL_checkint(_l, 1);
    y = luaL_checkint(_l, 2);
    width = luaL_checkint(_l, 3);
    height = luaL_checkint(_l, 4);
    red = luaL_optnumber(_l, 5, 0); // colors are optional, 0 is the default
    green = luaL_optnumber(_l, 6, 0);
    blue = luaL_optnumber(_l, 7, 0);

    // now push them across to the CamScripter
    init_packet();
    
    put_slip_char(get_msg_id_most_repr(OVAL_MSG_ID));
    put_slip_char(get_msg_id_least_repr(OVAL_MSG_ID));

    put_slip_char(7); // number of params

    // now the parameters
    put_slip_integer(x);
    put_slip_integer(y);
    put_slip_integer(width);
    put_slip_integer(height);
    put_slip_integer(red);
    put_slip_integer(green);
    put_slip_integer(blue);

    finish_packet();

    return 0;
}

/**
 * Function called from within Lua to wipe the camscripter graphics area clean.
 * Just forward on across the serial port.
 */
int clear_graphics(lua_State *_l) {
    init_packet();
    put_slip_char(get_msg_id_most_repr(CLEAR_GRAPHICS_MSG_ID));
    put_slip_char(get_msg_id_least_repr(CLEAR_GRAPHICS_MSG_ID));
    put_slip_char(0); // number of params
    finish_packet();
    return 0;
}


/**
 * Function called from inside lua to print a graphical representation of 
 * a color tracker packet. Send the relevant pieces of the color tracker 
 * data structure over as a ScripterMsg.
 */
int print_color_tracker(lua_State *_l) {
    cc3_track_pkt_t *pkt = checktracker(_l, 1);

    // only send messages if we're in debug mode
    if (debug_on) {
        init_packet();

        // first the ID we have to put it in two bytes
        // big endian
        put_slip_char(get_msg_id_most_repr(TRACKER_MSG_ID));
        put_slip_char(get_msg_id_least_repr(TRACKER_MSG_ID));
        
        // now the param count. 1 bytes
        put_slip_char(8);

        // now the parameters
        put_slip_integer(cc3_g_pixbuf_frame.width);  // image width
        put_slip_integer(cc3_g_pixbuf_frame.height); // image height
        put_slip_integer(pkt->centroid_x); // centroid x
        put_slip_integer(pkt->centroid_y); // centroid y
        put_slip_integer(pkt->x0); // x0
        put_slip_integer(pkt->y0); // y0
        put_slip_integer(pkt->x1); // x1
        put_slip_integer(pkt->y1); // y1

        finish_packet();
    }

    return 0;
}


////////////////////////////////////////////
// START FUNCTIONS RELATED TO cc3_pixbuf //
///////////////////////////////////////////

/**
 * Function to take a picure using the CMUcam3. Just a wrapper of 
 * cc3_pixbuf_load. Note that this function 
 * only takes a picture and loads it into the pixbuf -- you need to call 
 * something else if you want to write the picture to file or anything else.
 */
int take_picture(lua_State *_l) {
    cc3_pixbuf_load();
    // reset line buffer in case image properties changed
    g_img_prev_y=-1; 
    g_img_prev_x=-1; 
    if(g_img.pix!=NULL) free(g_img.pix);
    g_img.pix = cc3_malloc_rows(1);
    g_img.channels = cc3_g_pixbuf_frame.channels;
    return 0;
}


/**
 * Wrapper function for cc3_pixbuf_read_rows.  After double-checking the 
 * lua arguments, just call the function and stick the return value on 
 * the lua stack.
 */
int pixbuf_read_rows(lua_State *_l) {
    int rv;
    cc3_image_t *img = checkimage(_l);
    int num_rows = luaL_checkinteger(_l, 2);

    luaL_argcheck(_l, num_rows > 0 && num_rows <= img->height, 2, "Num Rows cannot be bigger than img:");
    rv = cc3_pixbuf_read_rows(img->pix, num_rows);
    lua_pushinteger(_l, rv);

    return 1;
}

/**
 * Wrapper function for cc3_pixbuf_frame_set_coi. Doublecheck the lua argument
 * to make sure that it's a legal value, then call cc3_pixbuf_frame_set_coi
 * and put the return value on the lua stack.
 */
int pixbuf_set_coi(lua_State *_l) {
    int rv;
    int coi = luaL_checkinteger(_l, 1);

    luaL_argcheck(_l, (coi >= 0 && coi <= 3), 1, 
                  "COI must be between 0 (single) and 3 (all channels).");
    rv = cc3_pixbuf_frame_set_coi(coi);
    lua_pushinteger(_l, rv);
    
    return 1;
}

/**
 * Simple getter function: get the width of the cc3_pixbuf_frame.
 */
int pixbuf_get_width(lua_State *_l) {
    lua_pushinteger(_l, cc3_g_pixbuf_frame.width);
    return 1; // one return parameter
}

/**
 * Simple getter function: get the height of the cc3_pixbuf_frame.
 */
int pixbuf_get_height(lua_State *_l) {
    lua_pushinteger(_l, cc3_g_pixbuf_frame.height);
    return 1; // one return parameter
}

/**
 * Simple getter function: get the mid point of the cc3_g_pixbuf_frame.width
 */
int pixbuf_get_x_mid(lua_State *_l) {
	uint16_t x_mid;
	x_mid = cc3_g_pixbuf_frame.x0 + (cc3_g_pixbuf_frame.width / 2);
    lua_pushinteger(_l, x_mid);
    return 1; // one return parameter
}

/**
 * Simple getter function: get the mid point of the cc3_g_pixbuf_frame.height
 */
int pixbuf_get_y_mid(lua_State *_l) {
	uint16_t y_mid;
	 y_mid = cc3_g_pixbuf_frame.y0 + (cc3_g_pixbuf_frame.height / 2);
    lua_pushinteger(_l, y_mid);
    return 1; // one return parameter
}
    
//////////////////////////////////////////
// END FUNCTIONS RELATED TO cc3_pixbuf //
/////////////////////////////////////////


/**
 * Sends the picture currently in the pixbuf over serial
 */
int send_picture_msg(lua_State *_l) {
	uint32_t x, y;
  	uint32_t size_x, size_y;

    // only send if we're in debug mode
    if (debug_on) {
        init_packet();
        // First the header
        // first the ID we have to put it in two bytes
        // big endian
        put_slip_char(get_msg_id_most_repr(PPM_MSG_ID));
        put_slip_char(get_msg_id_least_repr(PPM_MSG_ID));
        
        // now the param count. 1 bytes
        put_slip_char(1);
        
        cc3_pixbuf_rewind(); // just in case
        uint8_t *row = cc3_malloc_rows(1);
        
        // now parameters
        size_x = cc3_g_pixbuf_frame.width;
        size_y = cc3_g_pixbuf_frame.height;
        char* head = malloc(20);
        sprintf(head,"P6\n%d %d\n255\n",size_x,size_y );
        uint32_t size = strlen(head)+size_x*size_y*3;
        
        // size is 3 bytes, so we first get
        // rid of most representative byte
        put_slip_char(get_size_most_repr(size));
        put_slip_char(get_size_middle_repr(size));
        put_slip_char(get_size_least_repr(size));
        put_slip_string(head);
        free(head);
        
		bool led_on = false;
        for (y = 0; y < size_y; y++) {
            cc3_pixbuf_read_rows(row, 1);
        	cc3_led_set_state(0, led_on=!led_on);
            for (x = 0; x < size_x * 3U; x++) {
                /* Sleep to prevent byte-loss. Seems a bit extreme, but 
                   I saw problems with even 1000 and 500 as intervals. */
                if (x % 100 == 0) {
                    cc3_timer_wait_ms (5);
                }
                put_slip_char(row[x]);
            }
        }
        cc3_led_set_state(0, false);
        free(row);
        finish_packet();
    }    
    return 0;
}

/**
 *  Function to capture a ppm 
 */
void capture_ppm(FILE *f) {
	uint32_t x, y;
  	uint32_t size_x, size_y;

  	uint32_t time, time2;
 	int write_time;

  	cc3_pixbuf_load ();
  	uint8_t *row = cc3_malloc_rows(1);

  	size_x = cc3_g_pixbuf_frame.width;
  	size_y = cc3_g_pixbuf_frame.height;

  	fprintf(f,"P6\n%d %d\n255\n",size_x,size_y );

  	time = cc3_timer_get_current_ms();
  	for (y = 0; y < size_y; y++) {
    	cc3_pixbuf_read_rows(row, 1);
    	for (x = 0; x < size_x * 3U; x++) {
      		uint8_t p = row[x];
      		if (fputc(p, f) == EOF) {
				perror("fputc failed");
      		}
    	}
    	fprintf(stderr, ".");
    	fflush(stderr);
  	}
  	time2 = cc3_timer_get_current_ms();
  	write_time = time2 - time;

  	free(row);
}  

/**
 * Wrapper of cc3_timer_wait_ms.
 */
int wait(lua_State *_l) {
    int ms = luaL_checkinteger(_l, 1); // make sure it's an integer
    cc3_timer_wait_ms(ms);
    return 0; // no return parameters
}


/**
 * Function to save the image currently in the pixbuf onto the MMC card
 * using the passed filename.
 */
int save_picture(lua_State *_l) {
	char fname[20];
	uint32_t x, y;
  	uint32_t size_x, size_y;
    FILE *f;

    // make sure filename is legal FAT16
	const char *name = luaL_checkstring(_l, 1); /* get the parameter */
    luaL_argcheck(_l, is_format_8_3(name), 1, 
                  "Illegal Filename! Filename must be FAT16 <8,3> format.");

	strcpy(fname,"c:/");
	strcat(fname,name);

	f = fopen(fname, "w");
    cc3_pixbuf_rewind(); // just in case
  	uint8_t *row = cc3_malloc_rows(1);

  	size_x = cc3_g_pixbuf_frame.width;
  	size_y = cc3_g_pixbuf_frame.height;
  	char* head = malloc(20);
  	sprintf(head,"P6\n%d %d\n255\n",size_x,size_y );
	fprintf(f,"P6\n%d %d\n255\n",size_x,size_y );
  	free(head);
	
  	for (y = 0; y < size_y; y++) {
    	cc3_pixbuf_read_rows(row, 1);
    	for (x = 0; x < size_x * 3U; x++) {
      		fputc(row[x], f);
    	}
  	}
  	free(row);
	fclose(f);

    return 0;
}


/*****************************
 * TODO:
 * PPM msg as a stream of bytes.
 * Currently not working.
 * 
 * ***************************/

int get_PPM_size(ParamBodyByteStream* bstream) {
  	// Let's make sure we didn't call this twice!
  	if (bstream->state.blob != NULL ) {
  		return ((PPMData*)(bstream->state.blob))->size;
  	}
  	// OK. First time for this parameter...
  	PPMData* ppm = malloc(sizeof(PPMData));

  	// TODO: We should check that there's no other message using our ONLY buffer
  	cc3_pixbuf_load ();
  	ppm->row = cc3_malloc_rows(1);

  	ppm->size_x = cc3_g_pixbuf_frame.width;
  	ppm->size_y = cc3_g_pixbuf_frame.height;
	
	ppm->x = 0;
	ppm->y = 0;
	ppm->header = malloc(20);
	sprintf(ppm->header,"P6\n%d %d\n255\n",ppm->size_x,ppm->size_y);
	bstream->state.blob = ppm;
	ppm->size = strlen(ppm->header) + ppm->size_x * ppm->size_y * 3;
	return ppm->size;
}

char* get_PPM_char(ParamBodyByteStream* bstream) {
	PPMData* ppm = (PPMData*)bstream->state.blob;
	uint8_t* next_c = NULL;
	if ( bstream->state.idx < 15 ) {
		next_c = (uint8_t*)&ppm->header[bstream->state.idx];
		bstream->state.idx++;
	} else if ( bstream->state.idx == 15 ) {
	    cc3_pixbuf_read_rows(ppm->row, 1);
	    bstream->state.idx++;
	    // y=0, x=0 for the regular loop
	    next_c = &ppm->row[ppm->x++];
	} else if ( ppm->x < ppm->size_x * 3U ) {
	    next_c = &ppm->row[ppm->x++];
	} else if ( ppm->y < ppm->size_y-1 ) {
		// size_y-1 because we come here AFTER we read the row (size_x)
    	cc3_pixbuf_read_rows(ppm->row, 1);
    	ppm->y++;
    	ppm->x = 0;
	    next_c = &ppm->row[ppm->x++];
	}
	return (char*)next_c;
}

/**
 * Frees memory used by the PPM data structure
 */
void dispose_PPM(ParamBodyByteStream* bstream) {
	PPMData* ppm = (PPMData*)bstream->state.blob;
	if ( ppm != NULL ) {
		if ( ppm->header != NULL ) {
			free(ppm->header);
		}
		if ( ppm->row != NULL ) {
			free(ppm->row);
		}
		free(ppm);
	}	
}

/**
 * Tries to send a picture message using the streaming
 * mechanism.
 * TODO: currently not working
 * 
int send_picture_msg_STREAM(lua_State *_l) {
	ScripterMsg* msg = create_msg(PPM_MSG_ID);
	send_debug_msg("created msg");
	msg->param_count = 1;
	MsgParam* param = create_stream_msg_param();
	send_debug_msg("created stream param");
	param->data.data_stream.fp_get_char = &get_PPM_char;
	param->data.data_stream.fp_get_size = &get_PPM_size;
	param->data.data_stream.fp_dispose = &dispose_PPM;
	msg->parameters = param;
	send_debug_msg("before streaming message");
	stream_msg(msg);
	destroy_msg(msg);
	return 0;
}
*/

/**
 * Simple function to check if the passed fname fits into the <8.3> 
 * FAT16 format. For now, not doing any checking of the characters, just 
 * making sure it's 8.3
 */
int is_format_8_3(const char* fname) {
    // check total length first
    if (strlen(fname) > 12 || strlen(fname) < 1) {
        return false;
    }

    /* If there's a dot, it's max 8 before the dot, and max 3 after.
       If no dot, max 8 chars in the name */
    char* dot = strchr(fname, '.');
    if (dot == NULL) {
        return strlen(fname) <= 8;
    } else {
        // dot + 3 other chars = 4
        if ((fname + strlen(fname) - dot) > 4) {
            return false;
        }
        if ((dot - fname) > 8) {
            return false;
        }
    }
    return true;
}

/**
 * Wrapper of cc_3_led_set_state to turn on LED
 */
int turn_on_led(lua_State *_l) {
	uint8_t led = luaL_checkinteger(_l, 1); // make sure it's an integer
	luaL_argcheck(_l, (led == 0 || led == 1 || led == 2), 1, "LED must be either 0, 1, or 2.");
    cc3_led_set_state(led, true);
    return 0; // no return parameters
}

/**
 * Wrapper of cc_3_led_set_state to turn off LED
 */
int turn_off_led(lua_State *_l) {
	uint8_t led = luaL_checkinteger(_l, 1); // make sure it's an integer
	luaL_argcheck(_l, (led == 0 || led == 1 || led == 2), 1, "LED must be either 0, 1 or 2.");
    cc3_led_set_state(led, false);
    return 0; // no return parameters
}

/**
 * Turns LED led on for ms milliseconds. LED led is turned off
 * after that.
 */
void turn_on_led_for( uint8_t led, uint32_t ms ) {
  	cc3_led_set_state(led, true);
	cc3_timer_wait_ms(ms);
  	cc3_led_set_state(led, false);
}

/**
 * Make the led passed in as parameter blink for ms
 * milliseconds.
 * The LED will be turned OFF as the last step of this function
 */
void blink_led( uint8_t led, uint32_t ms ) {
  	uint32_t INTERVAL = 40;
  	uint_least32_t start_time = cc3_timer_get_current_ms ();
  	bool led_on = true;
  	cc3_led_set_state(led, led_on);
	do {
		led_on = !led_on;
		cc3_led_set_state(led, led_on);
		cc3_timer_wait_ms(INTERVAL);
	} while (cc3_timer_get_current_ms () < (start_time + ms));
  	cc3_led_set_state(led, false);
}


/**
 * Get and send information about the specified point in the pixbuf.
 * Look at the 5x5 rectangle within two pixels of x, y, and send back:
 * R, G, B at (x, y)
 * average R, G, B
 */
void send_image_info(int x, int y) {
    int BUFFER = 2;
    cc3_image_t img;
    cc3_pixel_t pixel;
    
    image_info_pkt_t* info_pkt = malloc(sizeof(image_info_pkt_t));
    info_pkt->x = x;
    info_pkt->y = y;

    // make sure we got legal values
    if (x > cc3_g_pixbuf_frame.width || y > cc3_g_pixbuf_frame.height) {
        send_debug_msg("(%d, %d) is outside the bounds of the pixbuf.", x, y);
        return;
    }
    info_pkt->x = x;
    info_pkt->y = y;
    info_pkt->avg_r = info_pkt->avg_g = info_pkt->avg_b = 0;

    // get mins and maxes, in case we're near the edge of an image
    int y_min = y - BUFFER;
    if (y_min < 0) {
        y_min = 0;
    }
    int y_max = y + BUFFER;
    if (y_max > cc3_g_pixbuf_frame.height) {
        y_max = cc3_g_pixbuf_frame.height;
    }
    int x_min = x - BUFFER;
    if (x_min < 0) {
        x_min = 0;
    }
    int x_max = x + BUFFER;
    if (x_max > cc3_g_pixbuf_frame.width) {
        x_max = cc3_g_pixbuf_frame.width;
    }

    // get to the min row first
    cc3_pixbuf_rewind();
    img.pix = cc3_malloc_rows(1);

    int idx = 0;
    while (idx < y_min) {
        cc3_pixbuf_read_rows(img.pix, 1);
        idx++;
    }
    
    int num_pix = 0;
    for (; idx < y_max+1; idx++) {
        cc3_pixbuf_read_rows(img.pix, 1);
        for (int j = x_min; j < x_max+1; j++) {
            cc3_get_pixel(&img, j, 0, &pixel);
            info_pkt->avg_r += pixel.channel[0];
            info_pkt->avg_g += pixel.channel[1];
            info_pkt->avg_b += pixel.channel[2];

            if (idx == y && j == x) {
                info_pkt->r = pixel.channel[0];
                info_pkt->g = pixel.channel[1];
                info_pkt->b = pixel.channel[2];
            } 
            num_pix++;
        }
    }
    info_pkt->avg_r = info_pkt->avg_r / num_pix;
    info_pkt->avg_g = info_pkt->avg_g / num_pix;
    info_pkt->avg_b = info_pkt->avg_b / num_pix;
    
    free(img.pix); // free img.pix  info_pkt gets freed in send_image_info_msg
    send_image_info_msg(info_pkt);
}
