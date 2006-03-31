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

// row counter in the integral image to denote the current "top row"
// i.e. the row for which the sub-windows are being evaluated for features
uint8_t cc3_row_counter_ii;

// row counter in the actual but cropped image (i.e. with top & bottom offset)
uint8_t cc3_row_counter_cropped_img;

// row counter in the actual full image
uint16_t cc3_row_counter_actual_img;  

// function to get the current segment from the actual image
void cc3_get_curr_segment()
{

  cc3_pixel_t pix_temp;

  if (cc3_row_counter_cropped_img == 0)  // first time
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
    { // get a row
      cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  
      // copy the row, pixel by pixel into integral image
      for (uint16_t j = 0; j < cc3_img_tmp.width; j++)
	{
	  cc3_get_pixel(&cc3_img_tmp, j, 1, &pix_temp);
	  cc3_integral_image[cc3_row_counter_ii-1][j] = pix_temp.channel[1]; // select the green channel 
	}
    }

}


// function to compute the integral image for the current segment
void cc3_get_integral_image()
{
  if (cc3_row_counter_cropped_img ==0)  // again, only for the first time
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
      // only need to update the integral image for newly added row

      // cumulative sum over newly added row
	for (uint16_t j = 1; j < CC3_INTEGRAL_IMG_WIDTH; j++)  // start from second col
	  {
	    cc3_integral_image[cc3_row_counter_ii-1][j]+=cc3_integral_image[cc3_row_counter_ii-1][j-1]; 
	  }
      
      // cumulative sum over col to give the integral image
       for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	  {
	  cc3_integral_image[cc3_row_counter_ii-1][j]+=cc3_integral_image[(cc3_row_counter_ii-1-1) % CC3_INTEGRAL_IMG_HEIGHT][j];
	   //circular warping of coordinates (CHECK AGAIN, if support available!!!!!!!!)
	  }
    }
}



/* 
 * function to get the feature value for a particular sub-window
 *
 */
int16_t cc3_get_feat_val(uint8_t feat_num, uint8_t curr_scale, uint16_t x, uint16_t y)
{
  int16_t fval = 0;
  uint16_t x_in_ii, y_in_ii;

  // evaluate the subwindow for this feature
  for (uint8_t pt=0; pt < 9 ; pt++)
    {
      x_in_ii = x + CC3_FACE_FEATURES[feat_num][curr_scale].x[pt];
      y_in_ii = y + CC3_FACE_FEATURES[feat_num][curr_scale].y[pt];
      fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES[feat_num][curr_scale].val_at_corners[pt];
    }

  // check if fval > threshold
  if (fval*CC3_FACE_FEATURES[feat_num][curr_scale].parity < CC3_FACE_FEATURES[feat_num][curr_scale].thresh*CC3_FACE_FEATURES[feat_num][curr_scale].parity)
    return CC3_FACE_FEATURES[feat_num][curr_scale].alpha;
  else 
    return 0;

}


/*-------------- main starts from here---------------*/
int main (void)
{
  
  int16_t val; 
// don't know why its here...just dont feel like removing it...has been the sole variable that has stood the test of time...:-)
// i feel bonded ....

  // feature value for a particular sudwindow
  int16_t feat_val;

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
 
    cc3_row_counter_ii = 0;
    cc3_row_counter_cropped_img = 0;

    // set the no. of detected faces as 0
    cc3_num_detected_faces=0;

    // iterating over the rows of the actual image until the bottom offset
    while (cc3_row_counter_actual_img < (CC3_INTEGRAL_IMG_HEIGHT - bottom_offset))
    { 
      // get the current segment 
      cc3_get_curr_segment();

      // get/update the current integral image
      cc3_get_integral_image();

      // normalize??
      
      for (uint8_t curr_scale_idx = 0; curr_scale_idx < CC3_NUM_SCALES; curr_scale_idx++)
	{
	 
	  // check if we need to evaluate subwindows at this scale for this sub-window
	  if (cc3_rows_to_eval_feat[cc3_row_counter_cropped_img+top_offset][curr_scale_idx] == 1)
	    {
	      // iterate over all horizontal shifted sub-window
	      for (uint8_t curr_pos_x =0; curr_pos_x < CC3_INTEGRAL_IMG_WIDTH; curr_pos_x+=CC3_WIN_STEPS[curr_scale_idx])
		{

		  

		  int16_t feat_sum = 0;
		  // compute the val of this sub-window for all the features
		  for (uint8_t curr_feat_idx = 0; curr_feat_idx < CC3_NUM_FEATURES; curr_feat_idx++)
		    {
		      feat_sum+=cc3_get_feat_val(curr_feat_idx, curr_scale_idx, curr_pos_x, cc3_row_counter_ii);
		    }

		  // check if its a face
		  if (feat_sum > CC3_GLOBAL_THRESHOLD)
		    {
		      // found a face....or the code says so..(and hope so!!!)
		      if (cc3_num_detected_faces < CC3_MAX_FACES )
			{
			  cc3_faces[cc3_num_detected_faces-1][0] = curr_pos_x;
			  cc3_faces[cc3_num_detected_faces-1][1] = cc3_row_counter_cropped_img + top_offset;
			  cc3_faces[cc3_num_detected_faces-1][2] = CC3_SCALES[curr_scale_idx];
			  cc3_num_detected_faces++;
			}
		      
		      // as of now, if MAX_FACES already reached, do nothing...(for a change!!)

		    }

		} // end of iterating over horizontal shifted sub-windows
	    }

	} // end of iterating over all scales
      
      // increment the row counters
      cc3_row_counter_actual_img++;
      cc3_row_counter_cropped_img++;
      cc3_row_counter_ii = (cc3_row_counter_ii + 1) % CC3_INTEGRAL_IMG_HEIGHT; // circular wrapping of integral image row counter
    } // end of iterating over all the rows in the actual image (upto bottom_offset)
   

    printf("No. of faces : %d \n", cc3_num_detected_faces);     
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


