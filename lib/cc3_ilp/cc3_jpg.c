/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <stdlib.h>

#include "cc3.h"
#include "cc3_jpg.h"
#include <jpeglib.h>


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
