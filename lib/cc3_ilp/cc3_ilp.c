#include "cc3.h"
#include "cc3_ilp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void cc3_send_image_direct (void)
{
  uint32_t x, y;
  uint32_t size_x, size_y;
  uint8_t *row = cc3_malloc_rows(1);

  cc3_set_led (1);
  size_x = cc3_g_current_frame.width;
  size_y = cc3_g_current_frame.height;
  cc3_pixbuf_load ();
  putchar (1);
  putchar (size_x);
  if (size_y > 255)
    size_y = 255;
  putchar (size_y);
  for (y = 0; y < size_y; y++) {
    putchar (2);
    
    cc3_pixbuf_read_rows(row, 1);
    for (x = 0; x < size_x * 3; x++) {
      putchar (row[x]);
    }
  }
  putchar (3);
  
  free(row);
}

uint8_t cc3_load_img_rows (cc3_image_t * img, uint16_t rows)
{


}

void cc3_get_pixel (cc3_image_t * img, uint16_t x, uint16_t y,
                    cc3_pixel_t * out_pix)
{
  if (img->channels > 1) {
    out_pix->channel[0] = ((uint8_t *) img->pix)[y * img->width + (x * 3)];
    out_pix->channel[1] =
      ((uint8_t *) img->pix)[y * img->width + (x * 3) + 1];
    out_pix->channel[2] =
      ((uint8_t *) img->pix)[y * img->width + (x * 3) + 2];
  }
  else {
    out_pix->channel[0] = ((uint8_t *) img->pix)[y * img->width + x];
  }

}


void cc3_set_pixel (cc3_image_t * img, uint16_t x, uint16_t y,
                    cc3_pixel_t * in_pix)
{
  if (img->channels > 1) {
    ((uint8_t *) img->pix)[y * img->width + (x * 3)] = in_pix->channel[0];
    ((uint8_t *) img->pix)[y * img->width + (x * 3) + 1] = in_pix->channel[1];
    ((uint8_t *) img->pix)[y * img->width + (x * 3) + 2] = in_pix->channel[2];
  }
  else {
    ((uint8_t *) img->pix)[y * img->width + x] = in_pix->channel[0];
  }


}
