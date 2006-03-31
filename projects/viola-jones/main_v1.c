#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "main.h"


/* ---global variables----*/

// kind of buffer to get the image rows from fifo to compute integral image
cc3_image_t cc3_img_tmp;

// keep track of detected faces
uint8_t cc3_num_detected_faces = 0;

// row counter into integral image to denote the current "top row"
uint8_t cc3_row_counter_ii;


// function to get the current segment from the actual image
void cc3_get_curr_segment()
{

  cc3_pixel_t pix_temp;

  if (cc3_row_counter_ii == 0)
    {
      // load the upper "CC3_INTEGRAL_IMG_HEIGHT" rows
      for (uint8_t i = 0; i < CC3_INTEGRAL_IMG_HEIGHT; i++)
	{
	  // get a row
	  cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  
	  // copy the row, pixel by pixel into integral image
	  for (uint16_t j = 0; j < cc3_img_tmp.width; j++)
	    {
	      cc3_get_pixel(&cc3_img_tmp, j, 1, &pix_temp);
	      cc3_integral_image[i][j] = pix_temp.channel[1]; // select the green channel 
	    }
	}
    }

  else 
    {

      //   uint16_t row_in_integral_image;

      // load the min no of rows needed...CC3_SCALES[0]
      for (uint8_t i = CC3_WIN_STEPS[0]; i >= 1; i--)
	{
	  // get a row
	  cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  
	  // copy the row, pixel by pixel into integral image
	  for (uint16_t j = 0; j < cc3_img_tmp.width; j++)
	    {
	      cc3_get_pixel(&cc3_img_tmp, j, 1, &pix_temp);
	      // row_in_integral_image = (i+cc3_row_counter) % CC3_INTEGRAL_IMG_HEIGHT;  // find the actual row in integral image
	      cc3_integral_image[cc3_row_counter_ii-i][j] = pix_temp.channel[1]; // select the green channel 
	    }
	}
    }

}


// function to compute the integral image for the current segment
void cc3_get_integral_image()
{
  if (cc3_row_counter_ii ==0)
    {
      // compute the integral image of the whole image

      // cumulative sum over rows
      for (uint16_t i = 0; i < CC3_INTEGRAL_IMG_HEIGHT; i++)
	for (uint16_t j = 1; j < CC3_INTEGRAL_IMG_WIDTH; j++)  // start from second col
	  cc3_integral_image[i][j]+=cc3_integral_image[i][j-1];
      
      // cumulative sum over cols to give the integral image
       for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	for (uint16_t i = 1; i < CC3_INTEGRAL_IMG_HEIGHT; i++)  // start from second row
	  cc3_integral_image[i][j]+=cc3_integral_image[i-1][j];
    }

  else 
    {
      // only need to update the integral image for newly added cols
      uint16_t top_row = cc3_row_counter_ii - CC3_WIN_STEPS[0]; // first row to be updated (can't be neg)

      // cumulative sum over rows
      for (uint16_t i = 0; i < CC3_WIN_STEPS[0]; i++)
	for (uint16_t j = 1; j < CC3_INTEGRAL_IMG_WIDTH; j++)  // start from second col
	  {
	    cc3_integral_image[i+top_row][j]+=cc3_integral_image[i+top_row][j-1]; 
	  }
      
      // cumulative sum over cols to give the integral image
       for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	for (uint16_t i = 0; i < CC3_WIN_STEPS[0]; i++)  
	  {
	  cc3_integral_image[i+top_row][j]+=cc3_integral_image[(i+top_row-1) % CC3_INTEGRAL_IMG_HEIGHT][j];
	   //circular warping of coordinates (CHECK AGAIN, if support available!!!!!!!!)
	  }
    }
}


/*-------------- main starts from here---------------*/
int main (void)
{
  
  int16_t val;

  // "new" image height, after cropping bottom and top rows
  // static uint8_t cropped_image_height = CC3_IMAGE_HEIGHT - top_offset - bottom_offset;

   uint16_t cc3_row_counter_actual_img;  // row index into the actual full image
    // setup system    
    cc3_system_setup ();

    // configure uarts
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
   
    cc3_camera_init ();
 
    cc3_set_colorspace(CC3_RGB);
    cc3_set_resolution(CC3_LOW_RES);
    cc3_set_auto_white_balance(true);
    cc3_set_auto_exposure(true);
     
    printf("Face Detector...\n");
    
    cc3_clr_led (0);
    cc3_clr_led (1);
    cc3_clr_led (2);
   
    // sample wait command in ms 
    cc3_wait_ms(1000);
    cc3_set_led (0);

    // setup integral image structure
    cc3_img_tmp.channels=3; // RGB color 
    cc3_img_tmp.width=cc3_g_current_frame.width;  // equal to Int_Img_Width
    cc3_img_tmp.height = 1;  // image will hold just 1 row for scanline processing
    cc3_img_tmp.pix = malloc(cc3_img_tmp.width*cc3_img_tmp.channels);  // malloc! (to hold one row RGB)

    
    while(1) 
      {
    // This tells the camera to grab a new frame into the fifo and reset
    // any internal location information.
    cc3_pixbuf_load();


    // discard some top rows, given by top_offset
    for (uint8_t i = 0; i < top_offset ; i++)
      {
       cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
      }


    // indicating how many rows have we read in the actual full image
    // top_offset rows are discarded and next "CC3_INTEGRAL_IMG_HEIGHT" rows are read in the first 
    // instance of the following for loop
    cc3_row_counter_actual_img = (top_offset-1) + (CC3_INTEGRAL_IMG_HEIGHT);   // index starts from 0

    // iterating over the different segments of the image
    for (cc3_row_counter_ii = 0; (cc3_row_counter_ii =< CC3_INTEGRAL_IMG_HEIGHT) /*& (cc3_row_counter_actual_img < CC3_IMAGE_HEIGHT - bottom_offset - CC3_WIN_STEPS[0])*/; cc3_row_counter_ii+=CC3_WIN_STEPS[0])
    {
      cc3_row_counter_actual_img+=CC3_WIN_STEPS[0];  

      // get the current segment 
      cc3_get_curr_segment();

      // get/update the current integral image
      cc3_get_integral_image();

      // normalize???
      
      for (uint8_t curr_scale_idx = 0; curr_scale_idx < CC3_NUM_SCALES; curr_scale_idx++)
	{

	  // iterate over all horizontal shifted sub-windows
	  for (uint8_t curr_pos_x =0; curr_pos_x < CC3_INTEGRAL_IMG_WIDTH; curr_pos_x+=CC3_WIN_STEPS[curr_scale_idx])
	    {




	    } // end of iterating over horizontal shifted sub-windows


	} // end of iterating over all scales
      


    } // end of iterating "row_counter"
   
     
    // sample non-blocking serial routine
       if(!cc3_uart_has_data(0) ) break; 
      } // end of while 
   
    free(cc3_img_tmp.pix);  // don't forget to free!
    printf( "You pressed %c to escape\n",fgetc(stdin) );

    // stdio actually works... 
    printf( "Type in a number followed by return to test scanf: " );
    scanf( "%d", &val );
    printf( "You typed %d\n",val );
    
    printf( "Good work, now try something on your own...\n" );
    while(1);

  return 0;
}


