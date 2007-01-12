#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "polly.h"

//#define VIRTUAL_CAM
//#define MMC_DEBUG

#define WIDTH	88
#define HEIGHT	72

// Constants for connected component blob reduce
#define SELECTED	255
#define NOT_SELECTED	0
#define MARKED		2
#define FINAL_SELECTED	1


/*
 * WARNING: Polly only works in low-res mode with 2x2 downsampling.
 * 	    Previous CMUcam State will be lost!
 *
 */
void polly( uint8_t color_thresh, uint8_t min_blob_size )
{
  uint32_t last_time, val;
  char c;
  cc3_pixel_t p;
  cc3_image_t polly_img;
  cc3_image_t img;
  uint8_t range[WIDTH];
  uint8_t p_img[HEIGHT * WIDTH];

 if(min_blob_size>30 ) return;

  cc3_set_colorspace (CC3_RGB);
  cc3_set_resolution (CC3_LOW_RES);
  cc3_set_auto_white_balance (true);
  cc3_set_auto_exposure (true);


  cc3_pixbuf_set_subsample (CC3_NEAREST, 2, 2);
  cc3_pixbuf_set_coi (CC3_GREEN);


  // setup an image structure 
  //img.channels=CC3_GREEN;
  img.channels = 1;
  img.width = cc3_g_current_frame.width;
  img.height = cc3_g_current_frame.height;      // image will hold just 1 row for scanline processing
  //img.pix = cc3_malloc_rows(1);
  img.pix = cc3_malloc_rows (cc3_g_current_frame.height);
  if (img.pix == NULL) {
    printf ("Not enough memory...\n");
    exit (0);
  }

  // setup polly temporary image
  polly_img.width = WIDTH;
  polly_img.height = HEIGHT;
  polly_img.channels = 1;
  polly_img.pix = &p_img;

    cc3_pixel_t mid_pix;
    cc3_pixel_t right_pix;
    cc3_pixel_t down_pix;

#ifdef MMC_DEBUG
	cc3_set_led(2);
	 while(!cc3_read_button());
	cc3_clr_led(2);
#endif  

    // clear polly working image
    p.channel[0] = 0;
    for (int y = 0; y < HEIGHT; y++)
      for (int x = 0; x < WIDTH; x++)
        cc3_set_pixel (&polly_img, x, y, &p);


    cc3_pixbuf_load ();

#ifdef MMC_DEBUG
    cc3_pixbuf_set_coi (CC3_ALL);
    write_raw_fifo_ppm();
    cc3_pixbuf_set_coi (CC3_GREEN);
    cc3_pixbuf_rewind();
#endif

    cc3_pixbuf_read_rows (img.pix, cc3_g_current_frame.height);

    p.channel[0] = SELECTED;
    for (int y = 0; y < img.height - 1; y++) {
      for (int x = 0; x < img.width - 1; x++) {
        int m, r, d;
        cc3_get_pixel (&img, x, y, &mid_pix);
        cc3_get_pixel (&img, x + 1, y, &right_pix);
        cc3_get_pixel (&img, x, y + 1, &down_pix);
        m = mid_pix.channel[0];
        r = right_pix.channel[0];
        d = down_pix.channel[0];
        if (m < r - color_thresh || m > r + color_thresh)
          cc3_set_pixel (&polly_img, x, y, &p);
        if (m < d - color_thresh || m > d + color_thresh)
          cc3_set_pixel (&polly_img, x, y, &p);

      }
    }

    connected_component_reduce (&polly_img, min_blob_size);
#ifdef MMC_DEBUG
    matrix_to_pgm (&polly_img);
#endif
    generate_histogram (&polly_img, range);
    convert_histogram_to_ppm (&polly_img, range);
#ifdef MMC_DEBUG
    matrix_to_pgm (&polly_img);
#endif
 //   printf( "Frame done, time=%d\n",cc3_timer()-last_time );
    // send a histogram packet
    printf( "H " );
    for(int i=5; i<WIDTH; i+=5)
	    printf( "%d ",HEIGHT-1-range[i] );
    printf( "\r" );


  free (img.pix);               // don't forget to free!

}

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

  return size;
}



void connected_component_reduce (cc3_image_t * img, int min_blob_size)
{
  int x, y, size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED) {
        size = count (img, x, y, min_blob_size);
        if (size < min_blob_size)
          reduce (img, x, y, min_blob_size, 1); // Delete marked
        else
          reduce (img, x, y, min_blob_size, 0); // Finalize marked
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


void generate_histogram (cc3_image_t * img, uint8_t * hist)
{
  int x, y;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  for (x = 0; x < width; x++)
    hist[x] = 0;


  for (x = width / 2; x < width; x++) {

    for (y = height - 1; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        //if(p_img[x][y]==SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  for (x = width / 2; x > 0; x--) {

    for (y = height - 1; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        //if(p_img[x][y]==SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  // Downsample Histogram 
  for (x = 0; x < width - 5; x += 5) {
    int min;
    min = 100;
    for (int i = 0; i < 5; i++) {
      if (hist[x + i] < min)
        min = hist[x + i];
    }
    for (int i = 0; i < 5; i++)
      hist[x + i] = min;
  }





}

void convert_histogram_to_ppm (cc3_image_t * img, uint8_t * hist)
{
  int x, y;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  // Write the range image out    
  p.channel[0] = 0;
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++)
      cc3_set_pixel (img, x, y, &p);

  p.channel[0] = 255;
  for (x = 0; x < width; x++) {

    for (y = height - 1; y > height - 1 - (hist[x]); y--) {
      cc3_set_pixel (img, x, y, &p);
    }
  }
}



void matrix_to_pgm (cc3_image_t * img)
{
  static uint32_t pgm_cnt = 0;
  char filename[32];
  FILE *fp;
  int width, height;
  cc3_pixel_t p;

 
  do { 
#ifdef VIRTUAL_CAM
	  sprintf(filename, "img%.5d.pgm", pgm_cnt);
#else
	  sprintf(filename, "c:/img%.5d.pgm", pgm_cnt);
#endif
    	fp = fopen(filename, "r");
    	if(fp!=NULL ) { 
		printf( "%s already exists...\n",filename ); 
		pgm_cnt++; 
		fclose(fp);
		}
    } while(fp!=NULL);
    pgm_cnt++; 

    // print file that you are going to write to stderr
    fprintf(stderr,"%s\r\n", filename);
    fp = fopen(filename, "w");  
    if(fp==NULL || pgm_cnt>200 )
    {
	printf( "PGM Can't open file\n" );
	cc3_set_led(3);
	while(1);
    }
  
  width = img->width;
  height = img->height;
  fprintf (fp, "P5\n%d %d 255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      fprintf (fp, "%c", p.channel[0]);
    }
  }
  fclose (fp);

}


void write_raw_fifo_ppm()
{
  uint32_t x, y;
  uint32_t size_x, size_y;
  FILE *f;
  static uint32_t ppm_cnt=0;
  char filename[32];
  uint8_t *row = cc3_malloc_rows(1);

   do { 
#ifdef VIRTUAL_CAM
    	sprintf(filename, "img%.5d.ppm", ppm_cnt);
#else
    	sprintf(filename, "c:/img%.5d.ppm", ppm_cnt);
#endif
	f = fopen(filename, "r");
    	if(f!=NULL ) { 
		printf( "%s already exists...\n",filename ); 
		ppm_cnt++; 
		fclose(f);
		}
    } while(f!=NULL);
    ppm_cnt++; 

    // print file that you are going to write to stderr
    fprintf(stderr,"%s\r\n", filename);
    f = fopen(filename, "w"); 
    if(f==NULL || ppm_cnt>200 )
    {
	printf( "PPM Can't open file\n" );
	cc3_set_led(3);
	while(1);
    }

  size_x = cc3_g_current_frame.width;
  size_y = cc3_g_current_frame.height;

  fprintf(f,"P3\n%d %d\n255\n",size_x,size_y );
  
  for (y = 0; y < size_y; y++) {
    cc3_pixbuf_read_rows(row, 1);
    for (x = 0; x < size_x * 3U; x++) {
      uint8_t p = row[x];
      fprintf(f,"%d ",p);
    }
  fprintf(f,"\n");
  }
  fclose(f); 
  free(row);
}
