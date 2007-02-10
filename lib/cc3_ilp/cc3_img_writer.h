#ifndef _CC3_IMG_WRITER_H_
#define _CC3_IMG_WRITER_H_

#include "cc3_ilp.h"
#include <stdint.h>


int cc3_pgm_img_write(cc3_image_t *img, char *filename);
int cc3_ppm_img_write(cc3_image_t *img, char *filename);
int cc3_img_write_file_create(cc3_image_t *img);

#endif
