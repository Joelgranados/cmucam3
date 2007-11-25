/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


/******************************************************************************
 *
 *  Initial CMUcam3 (cc3) data types and functions.
 *
 *****************************************************************************/
#include <stdbool.h>
#include <stdlib.h>

#include "cc3.h"
#include "cc3_pin_defines.h"
#include "cc3_hal.h"
#include "serial.h"
#include "devices.h"

char *_cc3_virtual_cam_path_prefix;

// Globals used by CMUCam functions
cc3_pixel_t cc3_g_current_pixel;        // global that gets updated with pixbuf calls
cc3_frame_t cc3_g_pixbuf_frame;        // global that keeps clip, stride


static inline void _cc3_seek_left (void);
static inline void _cc3_seek_top (void);

static inline void _cc3_pixbuf_skip_pixels (uint32_t size);

static inline uint8_t _cc3_pixbuf_read_subpixel (void);

static inline void _cc3_pixbuf_read_pixel (uint8_t * pixel,
                                           uint8_t * saved,
                                           uint8_t off0,
                                           uint8_t off1, uint8_t off2);

static inline void _cc3_pixbuf_skip_subpixel (void);


// Move to the next byte in the FIFO
static inline void _cc3_fifo_read_inc (void);

static uint8_t _cc3_second_green;
static bool _cc3_second_green_valid;

static void _cc3_update_frame_bounds (cc3_frame_t *);

void cc3_pixbuf_load ()
{
  //  uint32_t start_time;
  FILE *fp;
  char filename[255];
  int i,val,r,g,b,r2,b2,g2,t,depth,x,y,col_cnt,k;
  char c;
  static int img_cnt=0;

  
  if( _cc3_virtual_cam_path_prefix==NULL )
	  	{
		printf( "*Virtual-Camera Error: No Virtual Cam Path defined!, make sure you call cc3_camera_init()!\n" );
		exit(0);
		}
  printf( "cc3_pixbuf_loaded()\n" );
  if(_cc3_g_current_camera_state.colorspace==CC3_COLORSPACE_YCRCB)
	{
	printf( "*Virtual-Camera Error: Das YCrCb Colorspace ist Verboten...\n" );
	exit(0);
	}
  
  sprintf(filename, "%s/IMG%.5d.PPM",_cc3_virtual_cam_path_prefix, img_cnt);
  img_cnt++;
  fp=fopen(filename,"r" );
  if(fp==NULL )
	{
	printf( "*Virtual-Camera Error: No more test images...\n" );
	printf( "Last tried img: %s\n",filename );
	exit(0);
	}

  // skip first 3 rows of ppm
  // read first row
  do { c=fgetc(fp); }while(c!='\n' );
  val=fscanf( fp, "%d %d\n%d\n",&x, &y, &depth  ); 
  if(val==EOF) 
	{ 
	printf( "*Virtual-Camera Error: Malformed img file\n");   
	exit(0); 
	} 

   if(x!=352 && y!=288)
	{
	printf( "*Virtual-Cam Error: Bad Image Resolution, it must be at the max image size!\n" );
	exit(0);
	}

   col_cnt=0;
   i=0;
   do{
   // skip every other row in low-res mode
     if(_cc3_g_current_camera_state.resolution==CC3_CAMERA_RESOLUTION_LOW  && col_cnt>=176 ) 
	{
	for(k=0; k<352; k++ )
		{
		  val = r = fgetc(fp);
		  if(val==EOF) break;
		  val = g = fgetc(fp);
		  if(val==EOF) break;
		  val = b = fgetc(fp);
		  if(val==EOF) break;
		  val = r2 = fgetc(fp);
		  if(val==EOF) break;
		  val = g2 = fgetc(fp);
		  if(val==EOF) break;
		  val = b2 = fgetc(fp);
		  if(val==EOF) break;
		  //  val=fscanf( fp, "%d %d %d %d %d %d ",&r,&g,&b,&r2,&g2,&b2);
		}
	col_cnt=0;
	}
   //  val=fscanf( fp, "%d %d %d %d %d %d ",&r,&g,&b,&r2,&g2,&b2);
   r = fgetc(fp);
   g = fgetc(fp);
   b = fgetc(fp);
   r2 = fgetc(fp);
   g2 = fgetc(fp);
   val = b2 = fgetc(fp);
  // skip every other pixel in low-res mode
   if(_cc3_g_current_camera_state.resolution ==CC3_CAMERA_RESOLUTION_LOW  ) {
    r = fgetc(fp);
    g = fgetc(fp);
    b = fgetc(fp);
    r2 = fgetc(fp);
    g2 = fgetc(fp);
    val = b2 = fgetc(fp);
    //	val=fscanf( fp, "%d %d %d %d %d %d ",&r,&g,&b,&r2,&g2,&b2);
   }

   col_cnt++;
   virtual_fifo[i++]=g;
   virtual_fifo[i++]=r;
   virtual_fifo[i++]=g2;
   virtual_fifo[i++]=b;
   if(i>=VIRTUAL_FIFO_SIZE)
	{
	  printf( "*Virtual-Camera Error: FIFO ran out of data during frame load.\n" );
         fclose(fp);
	 abort();
	}
   } while(val!=EOF);
   
   printf( "Virtual FIFO Loaded %d bytes.\n", i);
   virtual_fifo_index=0;
   cc3_g_pixbuf_frame.y_loc = 0;
   fclose(fp);
}

void _cc3_fifo_read_inc (void)
{
  //printf( "cc3_fifo_read_inc\n" );
  if(virtual_fifo_index<VIRTUAL_FIFO_SIZE)
  	virtual_fifo_index++;
  else {
	  printf( "Virtual Cam Error: FIFO ran out during read.\n" );
          abort();
	}
}

void _cc3_pixbuf_skip_pixels (uint32_t size)
{
  uint32_t i;

  for (i = 0; i < size; i++) {
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
  }
}


void _cc3_seek_top ()
{
  if (cc3_g_pixbuf_frame.y_loc < cc3_g_pixbuf_frame.y0) {
    _cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.raw_width / 2
			     * cc3_g_pixbuf_frame.y0);

    cc3_g_pixbuf_frame.y_loc = cc3_g_pixbuf_frame.y0;
  }
}


void _cc3_seek_left ()
{
  _cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.x0 / 2);
}

uint8_t _cc3_pixbuf_read_subpixel (void)
{
  //uint8_t result = REG (GPIO_IOPIN) >> 24;
  uint8_t result;
  result=virtual_fifo[virtual_fifo_index];
  _cc3_fifo_read_inc ();
  return result;
}

void _cc3_pixbuf_skip_subpixel (void)
{
  _cc3_fifo_read_inc ();
}

void _cc3_pixbuf_read_pixel (uint8_t * pixel,
                             uint8_t * saved,
                             uint8_t off0, uint8_t off1, uint8_t off2)
{
  if (cc3_g_pixbuf_frame.x_step == 1) {
    if (_cc3_second_green_valid) {
      // use the second green
      _cc3_second_green_valid = false;
      *(pixel + off0) = *(saved + off0);
      *(pixel + off1) = _cc3_second_green;
      *(pixel + off2) = *(saved + off2);

      return;
    }

    // otherwise, load a new thing
    *(pixel + off1) = _cc3_pixbuf_read_subpixel ();   // G
    *(pixel + off0) = _cc3_pixbuf_read_subpixel ();   // R
    _cc3_second_green = _cc3_pixbuf_read_subpixel (); // G
    *(pixel + off2) = _cc3_pixbuf_read_subpixel ();   // B

    _cc3_second_green_valid = true;
  } else {
    _cc3_pixbuf_skip_subpixel ();
    *(pixel + off0) = _cc3_pixbuf_read_subpixel ();
    *(pixel + off1) = _cc3_pixbuf_read_subpixel ();
    *(pixel + off2) = _cc3_pixbuf_read_subpixel ();
  }
}

/**
 * cc3_pixbuf_rewind():
 * Rewinds the fifo.
 * Calling this and then changing parameters such as the
 * region of interest, channel of interest, virtual frame, and
 * subsampling will allow rapid reprocessing of a new frame.
 */
void cc3_pixbuf_rewind ()
{
  printf( "Fifo rewind!\n" );
  virtual_fifo_index=0;
  _cc3_second_green_valid = false;
  cc3_g_pixbuf_frame.y_loc = 0;
}


void cc3_led_set_state (uint8_t select, bool state)
{
  if (state) {
    switch (select) {
    case 0:
      printf( "led 0 on\n" );
      break;
    case 1:
      printf( "led 1 on\n" );
      break;
    case 2:
      printf( "led 2 on\n" );
      break;
    case 3:
      printf( "led 3 on\n" );
      break;
    }
  } else {
    switch (select) {
    case 0:
      printf( "led 0 off\n" );
      break;
    case 1:
      printf( "led 1 off\n" );
      break;
    case 2:
      printf( "led 2 off\n" );
      break;
    case 3:
      printf( "led 3 off\n" );
      break;
    }
  }
}

uint8_t *cc3_malloc_rows (uint32_t rows)
{
  int channels = cc3_g_pixbuf_frame.channels;
  int width = cc3_g_pixbuf_frame.width;

  return (uint8_t *) malloc (width * channels * rows);
}


/**
 * cc3_pixbuf_read_rows():
 * Using the cc3_frame_t reads rows taking into account the virtual window and subsampling.
 * This function copies a specified number of rows from the camera FIFO into a block
 * of cc3_pixel_t memory.
 * This should be the lowest level call that the user directly interacts with.
 * Returns number of rows read. (May be zero if error.)
 */
int cc3_pixbuf_read_rows (void * mem, uint32_t rows)
{

  int16_t j;
  uint16_t r;

  uint8_t off0, off1, off2;

  int width = cc3_g_pixbuf_frame.width;

  unsigned int row_limit = (cc3_g_pixbuf_frame.y1 - cc3_g_pixbuf_frame.y_loc)
    / cc3_g_pixbuf_frame.y_step;

  if (row_limit < rows) {
    rows = row_limit;
  }

  if (_cc3_g_current_camera_state.colorspace == CC3_COLORSPACE_RGB) {
    off0 = 0;
    off1 = 1;
    off2 = 2;
  }
  else if (_cc3_g_current_camera_state.colorspace == CC3_COLORSPACE_YCRCB) {
    off0 = 1;
    off1 = 0;
    off2 = 2;
  }
  else {
    off0 = 0;
    off1 = 1;
    off2 = 2;
  }

  // First read into frame
  _cc3_seek_top ();

  for (r = 0; r < rows; r++) {
    int x = cc3_g_pixbuf_frame.x0;

    // First read into line
    _cc3_seek_left ();

    switch (cc3_g_pixbuf_frame.coi) {
    case CC3_CHANNEL_ALL:
      _cc3_second_green_valid = false;
      for (j = 0; j < width; j++) {
        uint8_t *p = ((uint8_t *) mem) +
          (r * width + j * 3);
        _cc3_pixbuf_read_pixel (p, p - 3, off0, off1, off2);

	// advance by x_step
	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }

      break;

    case CC3_CHANNEL_RED:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  _cc3_pixbuf_skip_subpixel ();
	  *p = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	} else {
	  *p = *(p - 1);
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;

    case CC3_CHANNEL_GREEN:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  *p = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_second_green = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	} else {
	  *p = _cc3_second_green;
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;

    case CC3_CHANNEL_BLUE:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  *p = _cc3_pixbuf_read_subpixel ();
	} else {
	  *p = *(p - 1);
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;
    }
    _cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.raw_width - x) / 2);


    // advance by y_step
    _cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.y_step - 1) * cc3_g_pixbuf_frame.raw_width / 2);
    cc3_g_pixbuf_frame.y_loc += cc3_g_pixbuf_frame.y_step;
  }
  return rows;
}

/**
 * cc3_wait_ms():
 *
 */
void cc3_timer_wait_ms (uint32_t delay)
{
  uint32_t start;
  //start = cc3_timer ();
  //while (cc3_timer () < (start + delay));
  printf( "cc3_wait_ms not implemented!\n" );
}


/**
 * cc3_timer():
 *
 * This function returns the time since startup in ms as a uint32
 */
uint32_t cc3_timer_get_current_ms ()
{
  //return (REG (TIMER0_TC));     // REG in milliseconds
  printf( "cc3_timer not implemented!\n" );
  return 0;
}

/**
 * cc3_pixbuf_frame_set_roi():
 * Sets the region of interest in cc3_frame_t for virtual windowing.
 * This function changes the way data is read from the FIFO.
 * Returns 1 upon success and 0 on an out of bounds failure.
 */
bool cc3_pixbuf_frame_set_roi (int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  int w = cc3_g_pixbuf_frame.raw_width;
  int h = cc3_g_pixbuf_frame.raw_height;

  // constrain
  if (x0 < 0) {
    x0 = 0;
  }
  if ((x0 & 0x1) == 1) {
    x0++;          // x0 must be even!
  }
  if (y0 < 0) {
    y0 = 0;
  }
  if (x0 > w) {
    x0 = w;
  }
  if (y0 > h) {
    y0 = h;
  }

  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x1 > w) {
    x1 = w;
  }
  if (y1 > h) {
    y1 = h;
  }

  // check bounds
  if (x0 >= x1 || y0 >= y1) {
    return false;
  }

  // set if ok
  cc3_g_pixbuf_frame.x0 = x0;
  cc3_g_pixbuf_frame.y0 = y0;
  cc3_g_pixbuf_frame.x1 = x1;
  cc3_g_pixbuf_frame.y1 = y1;

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);

  return true;
}

/**
 * cc3_pixbuf_frame_set_subsample():
 * Sets the subsampling step and mode in cc3_frame_t.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_frame_set_subsample (cc3_subsample_mode_t mode, uint8_t x_step,
                              uint8_t y_step)
{
  bool result = 1;

  if (x_step == 0) {
    x_step = 1;
    result = 0;
  }
  if (y_step == 0) {
    y_step = 1;
    result = 0;
  }

  // only allow even subsampling (or 1) for x
  if (x_step != 1 && x_step % 2 != 0) {
    x_step++;
    result = 0;
  }

  cc3_g_pixbuf_frame.x_step = x_step;
  cc3_g_pixbuf_frame.y_step = y_step;
  //cc3_g_pixbuf_frame.x0 = 0;
  //cc3_g_pixbuf_frame.y0 = 0;
  //cc3_g_pixbuf_frame.x1 = cc3_g_pixbuf_frame.raw_width;
  //cc3_g_pixbuf_frame.y1 = cc3_g_pixbuf_frame.raw_height;
  cc3_g_pixbuf_frame.subsample_mode = mode;

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);

  return result;
}

/**
 * cc3_pixbuf_frame_set_coi():
 * Sets the channel of interest 1 or all.
 * This function changes the way data is read from the FIFO.
 * Returns 1 upon success and 0 on failure.
 */
bool cc3_pixbuf_frame_set_coi (cc3_channel_t chan)
{
  if (chan > 4)
    return 0;                   // Sanity check on bounds
  cc3_g_pixbuf_frame.coi = chan;

  cc3_g_pixbuf_frame.channels = (chan == CC3_CHANNEL_ALL ? 3 : 1);

  return 1;
}



/**
 * cc3_camera_init():
 * First Enable Camera & FIFO Power, next Reset Camera, then call cc3_set functions for default state
 *
 * Return:
 * 	1 successfully got acks back
 * 	0 failure (probably due to hardware?)
 */
bool cc3_camera_init ()
{

  _cc3_camera_reset ();
  _cc3_fifo_reset ();

  _cc3_g_current_camera_state.camera_type = _CC3_OV6620;        // XXX add autodetect code
  _cc3_g_current_camera_state.clock_divider = 0;
  _cc3_g_current_camera_state.brightness = -1;
  _cc3_g_current_camera_state.contrast = -1;
  _cc3_g_current_camera_state.auto_exposure = true;
  _cc3_g_current_camera_state.auto_white_balance = false;
  _cc3_g_current_camera_state.colorspace = CC3_COLORSPACE_RGB;
  _cc3_set_register_state ();

  printf( "cc3_camera_init()\n" );
  _cc3_virtual_cam_path_prefix = getenv("CC3_VCAM_PATH"); 
  if( _cc3_virtual_cam_path_prefix == NULL )
	  {
		printf( "*Virtual-Cam Error:  No CC3_VCAM_PATH defined.\n\nSet CC3_VCAM_PATH in your terminal to specify the directory that contains your virtual-camera test images.\n" );
	  exit(0);
	  }
  
  return 1;
}

void cc3_pixbuf_frame_reset ()
{
  cc3_g_pixbuf_frame.x_step = 1;
  cc3_g_pixbuf_frame.y_step = 1;
  cc3_g_pixbuf_frame.x0 = 0;
  cc3_g_pixbuf_frame.y0 = 0;
  cc3_g_pixbuf_frame.x1 = cc3_g_pixbuf_frame.raw_width;
  cc3_g_pixbuf_frame.y1 = cc3_g_pixbuf_frame.raw_height;
  cc3_g_pixbuf_frame.y_loc = 0;
  cc3_g_pixbuf_frame.subsample_mode = CC3_SUBSAMPLE_NEAREST;

  cc3_pixbuf_frame_set_coi(CC3_CHANNEL_ALL);

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);
}

void cc3_camera_set_power_state (bool state)
{

}


static void _cc3_set_cam_ddr_i2c_idle (void)
{
  //REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_IDLE;
  //_cc3_delay_i2c ();
}

static void _cc3_set_cam_ddr_i2c_write (void)
{
  //REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_WRITE;
  //_cc3_delay_i2c ();
}


static void _cc3_set_cam_ddr (volatile unsigned long val)
{
  //DDR(I2C_PORT,val);
  //REG (GPIO_IODIR) = val;
  //_cc3_delay_i2c ();
}



static unsigned int _cc3_i2c_send (unsigned int num, unsigned int *buffer)
{

  return 1;
}

/**
 * cc3_set_raw_register():
 * This will take an address and a value from the OmniVision manual
 * and set it on the camera.  This should be used for advanced low level
 * manipulation of the camera modes.  Currently, this will not set the
 * corresponding cc3 internal data structure that keeps record of the camera
 * mode.  Use with CAUTION.
 *
 * For basic manipulation of camera parameters see other cc3_set_xxxx functions.
 */
bool cc3_camera_set_raw_register (uint8_t address, uint8_t value)
{
  unsigned int data[3];
  int to;
  data[0] = _cc3_g_current_camera_state.camera_type;
  data[1] = address;
  data[2] = value;
  to = 0;
  while (_cc3_i2c_send (3, data)) {
    to++;
    if (to > 3)
      return false;
  }
  _cc3_delay_us_4 (1);
  return true;
}


/**
 * Sets the resolution, also updates cc3_g_pixbuf_frame width and height
 * Takes enum CC3_LOW_RES and CC3_HIGH_RES.
 */
void cc3_camera_set_resolution (cc3_camera_resolution_t cam_res)
{
  _cc3_g_current_camera_state.resolution = cam_res;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void _cc3_update_frame_bounds (cc3_frame_t *f)
{
  f->width = (f->x1 - f->x0) / f->x_step;
  f->height = (f->y1 - f->y0) / f->y_step;
}

/**
 * This sets the hardware colorspace that comes out of the camera.
 * You can choose between CC3_RGB or CC3_YCRCB.  In RGB mode, then
 * address pixels with CC3_RED, CC3_GREEN, CC3_BLUE, and CC3_GREEN2
 * in YCrCb mode, use CC3_CR, CC3_Y, CC3_CB, CC3_Y2 when indexing
 * the pixel array.
 */
void cc3_camera_set_colorspace (cc3_colorspace_t colorspace)
{
  _cc3_g_current_camera_state.colorspace = colorspace;
  _cc3_set_register_state ();
}


void cc3_camera_set_framerate_divider (uint8_t rate_divider)
{
  _cc3_g_current_camera_state.clock_divider = rate_divider;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_camera_set_auto_exposure (bool exp)
{
  _cc3_g_current_camera_state.auto_exposure = exp;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_camera_set_auto_white_balance (bool awb)
{
  _cc3_g_current_camera_state.auto_white_balance = awb;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_camera_set_brightness (uint8_t level)
{
  _cc3_g_current_camera_state.brightness = level;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_camera_set_contrast (uint8_t level)
{
  _cc3_g_current_camera_state.contrast = level;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}


bool cc3_button_get_state (void)
{
  return 1;
}

bool cc3_button_get_and_reset_trigger (void)
{
  return 1;
}

void cc3_filesystem_init (void)
{
  // nothing to initialize
  return;
}
