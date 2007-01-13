/*
 * Copyright 2006  Anthony Rowe and Adam Goode
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


#include <stdio.h>
#include <stdlib.h>

#include "cc3.h"
#include "cc3_jpg.h"
#include <jpeglib.h>


static void destroy_jpeg(void);
static void init_jpeg(void); 
static void capture_current_jpeg(FILE *f); 

void cc3_jpeg_send_simple(void) {


  // init jpeg
  init_jpeg();

  capture_current_jpeg(stdout);

  destroy_jpeg();
}


static struct jpeg_compress_struct cinfo;
static struct jpeg_error_mgr jerr;

//static cc3_pixel_t *row;
uint8_t *row;

static void init_jpeg(void) {
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // parameters for jpeg image
  cinfo.image_width = cc3_g_current_frame.width;
  cinfo.image_height = cc3_g_current_frame.height;
  //printf( "image width=%d image height=%d\n", cinfo.image_width, cinfo.image_height );
  cinfo.input_components = 3;
 // cinfo.in_color_space = JCS_YCbCr;
  cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 85, true);

  // allocate memory for 1 row
  row = cc3_malloc_rows(1);
  if(row==NULL) printf( "FUCK, out of memory!\n" );
}

static void capture_current_jpeg(FILE *f) {
  JSAMPROW row_pointer[1];
  row_pointer[0] = row;

  // output is file
  jpeg_stdio_dest(&cinfo, f);

  // capture a frame to the FIFO
  cc3_pixbuf_load();

  // read and compress
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    cc3_pixbuf_read_rows(row, 1);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  // finish
  jpeg_finish_compress(&cinfo);
}



static void destroy_jpeg(void) {
  jpeg_destroy_compress(&cinfo);
  free(row);
}
