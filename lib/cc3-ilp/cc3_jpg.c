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


#include <stdio.h>
#include <stdlib.h>

#include "cc3.h"
#include "cc3_jpg.h"
#include <jpeglib.h>

typedef struct {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
} cc3_jpeg_t;


static void destroy_jpeg(cc3_jpeg_t *cj);
static cc3_jpeg_t *init_jpeg (void);
static void capture_current_jpeg(cc3_jpeg_t *cj, FILE *f);

void cc3_jpeg_send_simple(void) {
  // capture a frame to the FIFO
  cc3_pixbuf_load();

  // init jpeg (allocates memory based on FIFO contents)
  cc3_jpeg_t *cj = init_jpeg();

  capture_current_jpeg(cj, stdout);

  destroy_jpeg(cj);
}



static cc3_jpeg_t *init_jpeg(void) {
  cc3_jpeg_t *cj = malloc(sizeof(cc3_jpeg_t));
  if (cj == NULL) {
    return NULL;
  }

  // init error structure
  cj->cinfo.err = jpeg_std_error(&cj->jerr);

  // init jpeg structure
  jpeg_create_compress(&cj->cinfo);

  // parameters for jpeg image
  cj->cinfo.image_width = cc3_g_pixbuf_frame.width;
  cj->cinfo.image_height = cc3_g_pixbuf_frame.height;

  //printf( "image width=%d image height=%d\n", cinfo.image_width, cinfo.image_height );
  cj->cinfo.input_components = 3;
 // cinfo.in_color_space = JCS_YCbCr;
  cj->cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_defaults(&cj->cinfo);
  jpeg_set_quality(&cj->cinfo, 85, true);

  // return
  return cj;
}

static void capture_current_jpeg(cc3_jpeg_t *cj, FILE *f) {
  JSAMPROW row_pointer[1];

  // allocate memory for 1 row
  row_pointer[0] = cc3_malloc_rows(1);
  if (row_pointer[0] == NULL) {
    return;
  }

  // output is file
  jpeg_stdio_dest(&cj->cinfo, f);

  // read and compress
  jpeg_start_compress(&cj->cinfo, TRUE);
  while (cj->cinfo.next_scanline < cj->cinfo.image_height) {
    cc3_pixbuf_read_rows(row_pointer[0], 1);
    jpeg_write_scanlines(&cj->cinfo, row_pointer, 1);
  }

  // finish
  jpeg_finish_compress(&cj->cinfo);

  free(row_pointer[0]);
}


static void destroy_jpeg(cc3_jpeg_t *cj) {
  jpeg_destroy_compress(&cj->cinfo);
  free(cj);
}
