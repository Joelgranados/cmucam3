#include "cc3_connected_component.h"
#include "cc3_ilp.h"

// Constants for connected component blob reduce
#define SELECTED        255
#define NOT_SELECTED    0
#define MARKED          2
#define FINAL_SELECTED  1



int count (cc3_image_t * img, int x, int y, int steps)
{
  int size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  size = 0;
  cc3_get_pixel (img, x, y, &p);
  if (p.channel[0] == SELECTED)
    size = 1;
  steps--;
  if (steps == 0)
    return size;
  p.channel[0] = MARKED;
  cc3_set_pixel (img, x, y, &p);

  if (x > 1) {
    cc3_get_pixel (img, x - 1, y, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y, steps);
  }
  if (x < width) {
    cc3_get_pixel (img, x + 1, y, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y, steps);
  }
  if (y > 1) {
    cc3_get_pixel (img, x, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x, y - 1, steps);
  }
  if (y < height) {
    cc3_get_pixel (img, x, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x, y + 1, steps);
  }

// stored in a global so it doesn't get pushed onto the stack
if( g_cc_conf.connectivity==L8_CONNECTED)
{
  if (x > 1 && y > 1) {
    cc3_get_pixel (img, x - 1, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y - 1, steps);
  }
  if (x < width && y > 1) {
    cc3_get_pixel (img, x + 1, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y - 1, steps);
  }
  if (x > 1 && y < height) {
    cc3_get_pixel (img, x - 1, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y + 1, steps);
  }
  if (x < width && y < height) {
    cc3_get_pixel (img, x + 1, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y + 1, steps);
  }
}

  return size;
}

int reduce (cc3_image_t * img, int x, int y, int steps, int remove)
{
  int size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;


  size = 0;
  cc3_get_pixel (img, x, y, &p);
  if (p.channel[0] == MARKED)
    size = 1;
  steps--;
  if (steps == 0)
    return size;

  if (remove)
    p.channel[0] = NOT_SELECTED;
  else
    p.channel[0] = FINAL_SELECTED;

  cc3_set_pixel (img, x, y, &p);

  if (x > 1) {
    cc3_get_pixel (img, x - 1, y, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y, steps, remove);
  }
  if (x < width) {
    cc3_get_pixel (img, x + 1, y, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y, steps, remove);
  }
  if (y > 1) {
    cc3_get_pixel (img, x, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x, y - 1, steps, remove);
  }
  if (y < height) {
    cc3_get_pixel (img, x, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x, y + 1, steps, remove);
  }
// stored in a global so it doesn't get pushed onto the stack
if( g_cc_conf.connectivity==L8_CONNECTED)
{
  if (x > 1 && y > 1) {
    cc3_get_pixel (img, x - 1, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y - 1, steps, remove);
  }
  if (x < width && y > 1) {
    cc3_get_pixel (img, x + 1, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y - 1, steps, remove);
  }
  if (x > 1 && y < height) {
    cc3_get_pixel (img, x - 1, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y + 1, steps, remove);
  }
  if (x < width && y < height) {
    cc3_get_pixel (img, x + 1, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y + 1, steps, remove);
  }
}
  return size;
}


void cc3_connected_component_reduce (cc3_image_t * img, ccr_config_t conf)
{
  int x, y, size;
  int width, height;
  cc3_pixel_t p;

  // Only uses connectivity globally, but the other values are
  // copied in case we need them around later
  g_cc_conf.connectivity=conf.connectivity;
  g_cc_conf.max_depth=conf.connectivity;
  g_cc_conf.min_blob_size=conf.connectivity;

  width = img->width;
  height = img->height;

  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED) {
        size = count (img, x, y, conf.max_depth);
        if (size < conf.min_blob_size)
          reduce (img, x, y, conf.max_depth,1); // Delete marked
        else
          reduce (img, x, y,conf.max_depth, 0); // Finalize marked
      }
    }


  // Translate for PPM
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == FINAL_SELECTED) {
        p.channel[0] = SELECTED;
        cc3_set_pixel (img, x, y, &p);
      }
    }
}

