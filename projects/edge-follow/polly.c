#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_conv.h>
#include <cc3_img_writer.h>
#include "polly.h"

//#define SAVE_EDGE_IMAGE 


ccr_config_t g_cc_conf;

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
int polly( polly_config_t config, cc3_image_t *img, cc3_image_t *mask )
{
  uint32_t last_time, val,i;
  char c;
  cc3_kernel_t blur;
  cc3_pixel_t p;
  cc3_image_t polly_img;
  
 

 // limit the recursive depth
 if(config.min_blob_size>30 ) return 0;

 


  // setup polly temporary image
  polly_img.width = img->width;
  polly_img.height = img->height;
  polly_img.channels = 1;
  polly_img.pix = malloc(polly_img.width*polly_img.height);
  if(polly_img.pix==NULL)
	return 0;
  
  if(config.histogram==NULL)
	return 0;


#ifdef MMC_DEBUG
  cc3_led_set_state(2, true);
  while(!cc3_button_get_state());
  cc3_led_set_state(2, false);
#endif  

    // clear polly working image
    p.channel[0] = 0;
    for (int y = 0; y < polly_img.height; y++)
      for (int x = 0; x < polly_img.width; x++)
        cc3_set_pixel (&polly_img, x, y, &p);

/*
    cc3_pixbuf_load ();

#ifdef MMC_DEBUG
    cc3_pixbuf_frame_set_coi (CC3_ALL);
    write_raw_fifo_ppm();
    cc3_pixbuf_frame_set_coi (CC3_GREEN);
    cc3_pixbuf_rewind();
#endif

    cc3_pixbuf_read_rows (img.pix, cc3_g_pixbuf_frame.height);
*/

 	if(config.blur==1)
	{
    	blur.size=3;
    	blur.mat[0][0]=1; blur.mat[0][1]=1; blur.mat[0][2]=1;
    	blur.mat[1][0]=1; blur.mat[1][1]=1; blur.mat[1][2]=1;
    	blur.mat[2][0]=1; blur.mat[2][1]=1; blur.mat[2][2]=1;
	blur.divisor=9;
    	val=cc3_convolve_img(img,blur);
	//val=cc3_img_write_file_create(mask);
    	if(val==0)
		{
		printf( "convolve failed\n" );
		exit(0);
		}
	} 
    p.channel[0] = SELECTED;
    for (int y = 0; y < img->height - 3; y++) {
      for (int x = 0; x < img->width - 3; x++) {
        int m, r, d,maskOK;
  	cc3_pixel_t mid_pix, right_pix, down_pix,mask_pix;
	maskOK=1;
	cc3_get_pixel (mask, x, y, &mask_pix); if(mask_pix.channel[0]==0) maskOK=0;	
	cc3_get_pixel (mask, x+2, y, &mask_pix); if(mask_pix.channel[0]==0) maskOK=0;	
	cc3_get_pixel (mask, x+1, y, &mask_pix); if(mask_pix.channel[0]==0) maskOK=0;	
	cc3_get_pixel (mask, x, y+1, &mask_pix); if(mask_pix.channel[0]==0) maskOK=0;	
	cc3_get_pixel (mask, x, y+2, &mask_pix); if(mask_pix.channel[0]==0) maskOK=0;	
	
	if(maskOK)
	{
	cc3_get_pixel (img, x, y, &mid_pix);
        cc3_get_pixel (img, x + 1, y, &right_pix);
        cc3_get_pixel (img, x, y + 1, &down_pix);
        m = mid_pix.channel[0];
	if(config.horizontal_edges==1)
		{
        	r = right_pix.channel[0];
        	if (m < r - config.color_thresh || m > r + config.color_thresh)
          		cc3_set_pixel (&polly_img, x, y, &p);
		}
	if(config.vertical_edges==1)
		{
        	d = down_pix.channel[0];
        	if (m < d - config.color_thresh || m > d + config.color_thresh)
          		cc3_set_pixel (&polly_img, x, y, &p);
		}
	}
      }
    }

    if(config.min_blob_size>1)
	{
	ccr_config_t cc_config;
	cc_config.max_depth=30;
	cc_config.min_blob_size=config.min_blob_size;
	cc_config.connectivity=config.connectivity;
    	connected_component_reduce (&polly_img, cc_config);
	}
#ifdef SAVE_EDGE_IMAGE 
    matrix_to_pgm (&polly_img);
#endif
    generate_polly_histogram (&polly_img, config.histogram);
  
    /*x_axis = malloc(polly_img.width);
    for(i=0; i<polly_img.width; i++ )
    	x_axis[i]=i;
    

     lreg(x_axis, config.histogram, polly_img.width,&reg_line);

     printf( "a=%f\n",reg_line.a );     
     printf( "b=%f\n",reg_line.b );     
     printf( "r^2=%f\n",reg_line.r_sqr );     

     double distance;
     distance=reg_line.a+reg_line.b*(polly_img.width/2);
     printf( "distance = %f\n",distance ); 
    
    convert_histogram_to_ppm (&polly_img, config.histogram);


#ifdef MMC_DEBUG
//    matrix_to_pgm (&polly_img);
#endif
 //   printf( "Frame done, time=%d\n",cc3_timer()-last_time );
    // send a histogram packet
    printf( "H " );
    for(int i=5; i<polly_img.width; i+=5)
 	    printf( "%d ",polly_img.height-config.histogram[i] );
    printf( "\r" );
*/

  //free (img.pix);               // don't forget to free!
  free (polly_img.pix);               
 // free (x_axis);             

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



void connected_component_reduce (cc3_image_t * img, ccr_config_t conf)
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


void generate_polly_histogram (cc3_image_t * img, int8_t * hist)
{
  int x, y;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  for (x = 0; x < width; x++)
    hist[x] = 0;


  for (x = width / 2; x < width-3; x++) {

    for (y = height - 3; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  for (x = width / 2; x > 0; x--) {

    for (y = height - 3; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  // Downsample Histogram 
//  for (x = 0; x < width - 5; x += 5) {
//    int min;
//    min = 100;
//    for (int i = 0; i < 5; i++) {
//      if (hist[x + i] < min)
//        min = hist[x + i];
//    }
//    for (int i = 0; i < 5; i++)
//      hist[x + i] = min;
// }


}
/*
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
*/


void matrix_to_pgm (cc3_image_t * img)
{
  static uint32_t pgm_cnt = 0;
  char filename[32];
  FILE *fp;
  int width, height;
  cc3_pixel_t p;

 
  do { 
#ifdef VIRTUAL_CAM
	  sprintf(filename, "img%.5da.pgm", pgm_cnt);
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
	cc3_led_set_state(3, true);
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
    printf("%s\r\n", filename);
    f = fopen(filename, "w"); 
    if(f==NULL || ppm_cnt>200 )
    {
	printf( "PPM Can't open file\n" );
	cc3_led_set_state(3, true);
	while(1);
    }

  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;

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
