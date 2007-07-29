#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "vj.h"

/* define for saving the images to mmc */
#define SAVE_IMAGES

/* ---global variables----*/

/* buffer to get the image rows from fifo to compute integral image */
cc3_image_t cc3_img_tmp;

/* keep track of detected faces */
uint8_t cc3_num_detected_faces = 0;

/* row counter in the integral image to denote the current "top row"
   i.e. the row for which the sub-windows are being evaluated for features */
uint8_t cc3_row_counter_ii;

/* row counter in the cropped image (i.e. with top & bottom offset) */
uint8_t cc3_row_counter_cropped_img;

/* row counter in the actual full image */
uint16_t cc3_row_counter_actual_img;  

/* integral image counter (for many rows, integral image has been calculated) */
uint16_t cc3_row_counter_calc_ii;

/* keep track of the current cascade stage for the current sub-window */
int8_t curr_cascade;

/* keep track of no. of frames (or rather no. of images taken) */
int16_t num_frames;

uint32_t mean_val;       /* mean for a subwinow */
uint32_t sum_pix;        /* sum of pixels in a sub-window */
uint32_t sum_pix_sq; /* sum of squares of pixels in a subwindow */
uint32_t std;        /* standard deviation */
uint32_t var;        /* variance  */

uint32_t vert_past_sum_sq_pix[4]; /* length should be equal to CC3_NUM_SCALES */
uint32_t horz_past_sum_sq_pix[4]; 

/* array to store rows while transfering b/w fifo and ii */
static uint8_t image_row[CC3_LO_RES_WIDTH*3];

#ifdef SAVE_IMAGES
FILE* fp;
FILE* fout;
#endif

/*
 * function to get the current segment from the actual image and simultaneously 
 * compute imtegral image for the new segment added 
 */
void cc3_get_curr_segment()
{
  cc3_pixel_t pix_temp;
  

  if (cc3_row_counter_cropped_img == 0)  /* first time */
    {
      // load the upper "CC3_INTEGRAL_IMG_HEIGHT" rows
      for (uint8_t i = 0; i < CC3_INTEGRAL_IMG_HEIGHT; i++)
	{
	  /* get a row from the fifo */
	  cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.height);
	  
	  /* copy the first pixel */
	  cc3_get_pixel(&cc3_img_tmp, 0, 0, &pix_temp);
	  cc3_integral_image[i][0] = pix_temp.channel[1]; // only green channel

	  
	  #ifdef SAVE_IMAGES
	  fprintf(fp, "%d ", cc3_integral_image[i][0]);
	  #endif

	  // start copying from the next pixel and calculate cumulative sum at the same time
	  for (uint16_t j = 1; j < cc3_img_tmp.width; j++)
	    {
	      cc3_get_pixel(&cc3_img_tmp, j, 0, &pix_temp);
	      cc3_integral_image[i][j] = pix_temp.channel[1]; // only green channel

	      #ifdef SAVE_IMAGES
	      fprintf( fp,"%d ",cc3_integral_image[i][j] );
	      #endif 
	      
	      // compute cumulative sum across the row
	      cc3_integral_image[i][j] += cc3_integral_image[i][j-1];
	    }

          #ifdef SAVE_IMAGES
	  fprintf( fp, "\n" );
	  #endif
	}
      
      // find cumulative sum along columns to complete the integral image computation
      for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	 {
	   for (uint16_t i = 1; i < CC3_INTEGRAL_IMG_HEIGHT; i++)  // start from second row
	     {
	       cc3_integral_image[i][j]+=cc3_integral_image[i-1][j];
	     }
	 }       
    }
  
  else 
    { 
      // get a new row
      cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.height);
	  
      uint8_t newly_added_row = cc3_row_counter_calc_ii % CC3_INTEGRAL_IMG_HEIGHT;
      uint8_t prev_row = (cc3_row_counter_calc_ii - 1 + CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT; 
       
      // copy the first pixel
      cc3_get_pixel(&cc3_img_tmp, 0, 0, &pix_temp);
      cc3_integral_image[newly_added_row][0] = pix_temp.channel[1];

      #ifdef SAVE_IMAGES
      fprintf( fp,"%d ",cc3_integral_image[newly_added_row][0] );
      #endif
      
      // read the row, from next pixel onward and compute cum sum across the row
      for (uint16_t j = 1; j < cc3_img_tmp.width; j++)
	{
	  cc3_get_pixel(&cc3_img_tmp, j, 0, &pix_temp);
	  cc3_integral_image[newly_added_row][j] = pix_temp.channel[1];

	  #ifdef SAVE_IMAGES
	  fprintf( fp,"%d ",cc3_integral_image[newly_added_row][j] );
          #endif

	  // compute cumulative sum across the row
	  cc3_integral_image[newly_added_row][j] += cc3_integral_image[newly_added_row][j-1];
	}
     
       #ifdef SAVE_IMAGES
       fprintf( fp, "\n" );
       #endif 

       // find cumulative sum along columns to complete the integral image computation
       for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	 {
	   cc3_integral_image[newly_added_row][j]+=cc3_integral_image[prev_row][j];
	 }
      
    }
}


/* 
 * function to get the feature value for a particular sub-window
 *
 */
int16_t cc3_get_feat_val(uint8_t feat_num, uint8_t curr_scale, uint16_t x, uint16_t y)
{
  int32_t fval = 0;
  uint16_t x_in_ii, y_in_ii;

  /* for stage1 cascade */
  if (curr_cascade == 0)
    {
      // evaluate the subwindow for this feature
      for (uint8_t pt=0; pt < 9 ; pt++)
	{
	  x_in_ii = x + CC3_FACE_FEATURES0[feat_num][curr_scale].x[pt];
	  y_in_ii = (y + CC3_FACE_FEATURES0[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
	  fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES0[feat_num][curr_scale].val_at_corners[pt];
	}

      // mult by scaling factor
      fval = fval * CC3_SCALING_FACTOR;
      
      // taking into account the effect of normalization
      fval = fval/((int32_t)std);
      
      // check if fval < threshold
      if (fval*CC3_FACE_FEATURES0[feat_num][curr_scale].parity < CC3_FACE_FEATURES0[feat_num][curr_scale].thresh*CC3_FACE_FEATURES0[feat_num][curr_scale].parity)
	return CC3_FACE_FEATURES0[feat_num][curr_scale].alpha;
      else 
	return 0;
    }

  /* for stage2 cascade */
  else if (curr_cascade == 1)
    {
       // evaluate the subwindow for this feature
      for (uint8_t pt=0; pt < 9 ; pt++)
	{
	  x_in_ii = x + CC3_FACE_FEATURES1[feat_num][curr_scale].x[pt];
	  y_in_ii = (y + CC3_FACE_FEATURES1[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
	  fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES1[feat_num][curr_scale].val_at_corners[pt];
	}
      
      // mult by scaling factor
      fval = fval * CC3_SCALING_FACTOR;

      // taking into account the effect of normalization
      fval = fval/((int32_t)std);
      
      // check if fval < threshold
      if (fval*CC3_FACE_FEATURES1[feat_num][curr_scale].parity < CC3_FACE_FEATURES1[feat_num][curr_scale].thresh*CC3_FACE_FEATURES1[feat_num][curr_scale].parity)
	return CC3_FACE_FEATURES1[feat_num][curr_scale].alpha;
      else 
	return 0;
    }

  /* for stage3 cascade */
  else if (curr_cascade == 2)
    {
       // evaluate the subwindow for this feature
      for (uint8_t pt=0; pt < 9 ; pt++)
	{
	  x_in_ii = x + CC3_FACE_FEATURES2[feat_num][curr_scale].x[pt];
	  y_in_ii = (y + CC3_FACE_FEATURES2[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
	  fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES2[feat_num][curr_scale].val_at_corners[pt];
	}
      
      // mult by scaling factor
      fval = fval * CC3_SCALING_FACTOR;

      // taking into account the effect of normalization
      fval = fval/((int32_t)std);
      
      // check if fval < threshold
      if (fval*CC3_FACE_FEATURES2[feat_num][curr_scale].parity < CC3_FACE_FEATURES2[feat_num][curr_scale].thresh*CC3_FACE_FEATURES2[feat_num][curr_scale].parity)
	return CC3_FACE_FEATURES2[feat_num][curr_scale].alpha;
      else 
	return 0;
     
    }

  /* for stage4 cascade */
  else if (curr_cascade == 3)
    {
       // evaluate the subwindow for this feature
      for (uint8_t pt=0; pt < 9 ; pt++)
	{
	  x_in_ii = x + CC3_FACE_FEATURES3[feat_num][curr_scale].x[pt];
	  y_in_ii = (y + CC3_FACE_FEATURES3[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
	  fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES3[feat_num][curr_scale].val_at_corners[pt];
	}
      
      // mult by scaling factor
      fval = fval * CC3_SCALING_FACTOR;

      // taking into account the effect of normalization
      fval = fval/((int32_t)std);
      
      // check if fval < threshold
      if (fval*CC3_FACE_FEATURES3[feat_num][curr_scale].parity < CC3_FACE_FEATURES3[feat_num][curr_scale].thresh*CC3_FACE_FEATURES3[feat_num][curr_scale].parity)
	return CC3_FACE_FEATURES3[feat_num][curr_scale].alpha;
      else 
	return 0;
    }

  /* for stage5 cascade */
  else if (curr_cascade == 4)
    {
       // evaluate the subwindow for this feature
      for (uint8_t pt=0; pt < 9 ; pt++)
	{
	  x_in_ii = x + CC3_FACE_FEATURES4[feat_num][curr_scale].x[pt];
	  y_in_ii = (y + CC3_FACE_FEATURES4[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
	  fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES4[feat_num][curr_scale].val_at_corners[pt];
	}
      
      // mult by scaling factor
      fval = fval * CC3_SCALING_FACTOR;

      // taking into account the effect of normalization
      fval = fval/((int32_t)std);
      
      // check if fval < threshold
      if (fval*CC3_FACE_FEATURES4[feat_num][curr_scale].parity < CC3_FACE_FEATURES4[feat_num][curr_scale].thresh*CC3_FACE_FEATURES4[feat_num][curr_scale].parity)
	return CC3_FACE_FEATURES4[feat_num][curr_scale].alpha;
      else 
	return 0;
    }

  else 
    return 0;
}


/*-------------- main starts from here---------------*/
int main (void)
{

  // name of the curr image
  char img_name[50];
  
   uint8_t x2, y2; // lower right sub-window coordinates
   uint16_t num_pixels; // no. of pixels in a subwindow
   
   int32_t temp1, temp2; // temp variables used for computations
   
   cc3_filesystem_init();
   
   // configure uarts
   cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
   
   cc3_camera_init ();
   
   cc3_camera_set_colorspace(CC3_COLORSPACE_RGB);
   cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);
   cc3_camera_set_auto_white_balance(true);
   cc3_camera_set_auto_exposure(true);
   
   printf("Face Detector...\n\r");
   
   cc3_led_set_state(0,0);
   cc3_led_set_state(1,0);
   cc3_led_set_state(2,0);
   
   /* sample wait command in ms  */
   cc3_timer_wait_ms(1000);
   cc3_led_set_state(0,1);
   
   /* setup integral image structure */
   cc3_img_tmp.channels=3; // RGB color 
   cc3_img_tmp.width=cc3_g_pixbuf_frame.width;  // equal to Int_Img_Width
   cc3_img_tmp.height = 1;  // image will hold just 1 row for scanline processing
   cc3_img_tmp.pix = &image_row;
   
   if (cc3_img_tmp.width != CC3_INTEGRAL_IMG_WIDTH)
     {
       printf("Error...image width and integral image width different \n\r");
     }
   
   num_frames = 0;
   
   while(1) 
     {
       cc3_num_detected_faces = 0;
       /* initliaze the past records of sum_sq_pix to zero */
       for (uint8_t i = 0; i < CC3_NUM_SCALES; i++)
	 {
	   vert_past_sum_sq_pix[i] = (uint32_t) 0;
	   horz_past_sum_sq_pix[i] = (uint32_t) 0;
	 }
       
       
      // printf(" New Frame...\n\r");
       printf( "START\r" ); 
       /* new image  */
       
       #ifdef SAVE_IMAGES

       sprintf(img_name, "%s%04d%s","c:/img",num_frames, ".pgm");
       fp=fopen(img_name,"w" );
       if (fp == NULL)
	 {
	   printf("%s %s\n\r", "Error Opening: ",img_name);
	  
	 }
       

       fprintf( fp, "P2\n%d %d\n255\n", cc3_g_pixbuf_frame.width, cc3_g_pixbuf_frame.height-top_offset-bottom_offset );
       sprintf(img_name, "%s%04d%s","c:/img",num_frames,".txt");
       fout = fopen(img_name, "w");
       if (fout == NULL)
	 {
	   printf("%s %s\n\r", "Error Opening  ",img_name);
	 }

       #endif 

       /* This tells the camera to grab a new frame into the fifo and reset
	  any internal location information */
       cc3_pixbuf_load();
       
       /* discard some top rows, given by top_offset */
       for (uint8_t i = 0; i < top_offset ; i++)
	 {
	   cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.height);
	 }
       
       
       /* indicating how many rows have we read in the actual full image
	  top_offset rows are discarded and next "CC3_INTEGRAL_IMG_HEIGHT" rows are read in the first 
	  instance of the following for loop */
       cc3_row_counter_actual_img = (top_offset-1)+1;   /* index starts from 0 */
       cc3_row_counter_ii = 0;
       cc3_row_counter_cropped_img = 0;
       cc3_row_counter_calc_ii = (CC3_INTEGRAL_IMG_HEIGHT - 1); 
       
       /* iterating over the rows of the actual image until the bottom offset */
       while (cc3_row_counter_actual_img < (CC3_IMAGE_HEIGHT - bottom_offset))
	 { 
	   if (cc3_row_counter_calc_ii < CC3_IMAGE_HEIGHT - top_offset - bottom_offset) /* check if we need to load any more rows? */
	      {
		/* get the current segment and its integral image */
		cc3_get_curr_segment();
	      }
	    
	    for (uint8_t curr_scale_idx = 0; curr_scale_idx < CC3_NUM_SCALES; curr_scale_idx++)
	      {
		/* check if we need to evaluate subwindows at this scale for this sub-window */
		if (cc3_rows_to_eval_feat[cc3_row_counter_cropped_img][curr_scale_idx] == 1)
		  { 
		    /* iterate over all horizontal shifted sub-window */
		    for (uint8_t curr_pos_x =0; curr_pos_x < CC3_INTEGRAL_IMG_WIDTH - CC3_SCALES[curr_scale_idx]; curr_pos_x+=CC3_WIN_STEPS[curr_scale_idx])
		      { 
			/* compute the bottom right coordinates for this window */
			x2 = curr_pos_x + CC3_SCALES[curr_scale_idx]-1 ;
			y2 = (cc3_row_counter_ii + CC3_SCALES[curr_scale_idx]-1) % CC3_INTEGRAL_IMG_HEIGHT;
			if (x2 >= CC3_INTEGRAL_IMG_WIDTH ) 
			  {
			    printf("*Error....width outside limits!! \n\r");
			  }
			
			/* Don't consider the first row and first column in the sub-window for calculating std & mean */
			num_pixels = (CC3_SCALES[curr_scale_idx]-1)*(CC3_SCALES[curr_scale_idx]-1);
			
			temp1 = cc3_integral_image[cc3_row_counter_ii][curr_pos_x] -  cc3_integral_image[cc3_row_counter_ii][x2];
			temp2 = cc3_integral_image[y2][x2] - cc3_integral_image[y2][curr_pos_x];
			sum_pix = (uint32_t) (temp1 + temp2); 
			mean_val = sum_pix/num_pixels;
			
			/* if first column --> use the past vertical sum_sq_pix */
			/* first time, calculate sum_sq_pix for every scale */
			if ( (curr_pos_x == 0) && (cc3_row_counter_cropped_img == 0) )
			  {
			    vert_past_sum_sq_pix[curr_scale_idx] = (uint32_t) 0;
			    horz_past_sum_sq_pix[curr_scale_idx] = (uint32_t) 0;
			    
			    sum_pix_sq = 0;
			    for (uint8_t i = (cc3_row_counter_ii+1) % CC3_INTEGRAL_IMG_HEIGHT; i != (y2+1)% CC3_INTEGRAL_IMG_HEIGHT; )
			      {
				for (uint8_t j = curr_pos_x+1; j <= x2; j++)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    
				    /* update horizontal past sum_sq_pix */	    
				    if ( j > (uint8_t) (curr_pos_x + CC3_WIN_STEPS[curr_scale_idx]) )
				      {
					horz_past_sum_sq_pix[curr_scale_idx]+= temp1;
				      }
				    
				    /* update vertical past sum_sq_pix */
				    if ( i > (uint8_t) (cc3_row_counter_ii + CC3_WIN_STEPS[curr_scale_idx]) )
				      {
					vert_past_sum_sq_pix[curr_scale_idx]+= temp1;
				      }
				    
				    /* update the sum_sq_pix for this window */
				    sum_pix_sq+= (uint32_t) temp1;
				    
				  }
				
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
			      }
			    
			  }	
			
			/* if the first col but not the first crow --> use vertical past sum_sq_pix
			   - need to update both vert past & horiz past sum_sq_pix */
			
			else if (curr_pos_x == 0)
			  {
			    uint8_t y11 = (cc3_row_counter_ii+1) % CC3_INTEGRAL_IMG_HEIGHT;
			    
			    /* use the past vert sum_sq_pix */
			    sum_pix_sq = vert_past_sum_sq_pix[curr_scale_idx];
			    
			    /* calcuate sum_sq_pix for top horizontal block */
			    uint32_t top_horz = (uint32_t) 0;
			    uint8_t i = y11;
			    
			    for (uint8_t k = 0; k < CC3_WIN_STEPS[curr_scale_idx]; k++)
			      {
				for (uint8_t j = curr_pos_x+1; j <= x2; j++)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    
				    top_horz+=temp1;
				    
				  }
				
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
			      }
			    
			    /* calculate left vertical block */
			    uint32_t left_vert = (uint32_t) 0;
			    
			    for (i = y11; i != (y2+1)% CC3_INTEGRAL_IMG_HEIGHT; )
			      {
				for (uint8_t j = curr_pos_x+1; j <= curr_pos_x + CC3_WIN_STEPS[curr_scale_idx]; j++)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    
				    left_vert+= temp1;
				  }
				
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
				
			      }
			    
			    /* calculate sum_sq_pix for new pixels */
			    
			    i = (y2 - CC3_WIN_STEPS[curr_scale_idx] + 1 + CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
			    for ( ; i != (y2 +1 ) % CC3_INTEGRAL_IMG_HEIGHT; )
			      {
				for (uint8_t j = curr_pos_x+1; j <= x2; j++)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    
				    sum_pix_sq+= temp1;
				    
				  }
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
				
			      }
			    
			    /* update vert past sum_sq_pix */
			    vert_past_sum_sq_pix[curr_scale_idx] = (sum_pix_sq - top_horz);
			    
			    /* update horz past sum_sq_pix */
			    horz_past_sum_sq_pix[curr_scale_idx] = (sum_pix_sq - left_vert);
			    
			  }
			
			
			/* else use horizontal past sum_sq_pix, no need to update vert past sum_sq_pix */
			else 
			  {
			    /* use the past horz sum_sq_pix */
			    sum_pix_sq = horz_past_sum_sq_pix[curr_scale_idx];
			    
			    uint8_t y11 = (cc3_row_counter_ii+1) % CC3_INTEGRAL_IMG_HEIGHT;
			    
			    /* claculate left vertical block */
			    uint32_t left_vert = (uint32_t) 0;
			    
			    for (uint8_t i = y11; i != (y2+1)% CC3_INTEGRAL_IMG_HEIGHT; )
			      {
				for (uint8_t j = curr_pos_x+1; j <= curr_pos_x + CC3_WIN_STEPS[curr_scale_idx]; j++)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    left_vert+= (uint32_t) temp1;
				    
				  }
				
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
			      }
			    
			    
			    /* calculate right vertical block and add to sum_sq_pix */
			    for (uint8_t i = y11; i != (y2+1)% CC3_INTEGRAL_IMG_HEIGHT; )
			      {
				for (uint8_t j = x2; j >= x2 - CC3_WIN_STEPS[curr_scale_idx] + 1; j--)
				  {
				    temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				    temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				    temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				    temp1 = temp1+temp2;
				    temp1 = temp1*temp1;  /* square of pixel intensity  */
				    
				    sum_pix_sq+= (uint32_t)temp1;
				  }
				
				i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
				
			      }
			    
			    /* update horz past sum_sq_pix */
			    horz_past_sum_sq_pix[curr_scale_idx] = (sum_pix_sq - left_vert);
			  }
			
			
			/* after sum_sq_pixels has been calculated */
			uint64_t numer, denom;
			
			numer = ((uint64_t)num_pixels*(uint64_t)sum_pix_sq);
			numer =  (numer - (uint64_t)sum_pix*(uint64_t)sum_pix);
			denom = ((uint64_t)num_pixels*(uint64_t)num_pixels);				
			var = (uint32_t)(numer/denom);
			
			
			if (var < 200)
			  {
			    std = 1;  /* basically reject the windows with this small variance */
			  }
			
			else
			  {
			    /* find the standard deviation */
			    for (uint32_t i = 14; i < var/2 ; i++)
			      {
				if ((i*i <= var) & ((i+1)*(i+1) > var))
				  {
				    if ((var - i*i) < ((i+1)*(i+1) - var))
				      std = i;
				    else 
				      std = i+1;
				    
				    break;
				  }
			      }
			  }
			
			/* check for face only if following conditions are satisfied, otherwise its too uniform  */
			if ( (std > 13) & ( (mean_val > 30) | (mean_val < 200)) )  
			  {
			    int16_t feat_sum = 0;
			    int8_t face = 1;
			    
			    /* check if this sub-window is a face */
			    
			    curr_cascade = 0; 
			    face = 1;
			    while ( (curr_cascade < CC3_NUM_CASCADES) & (face))
			      {
				feat_sum = 0;
				for (uint8_t curr_feat_idx = 0; curr_feat_idx < CC3_NUM_FEATURES[curr_cascade]; curr_feat_idx++)
				  {
				    feat_sum+=cc3_get_feat_val(curr_feat_idx, curr_scale_idx, curr_pos_x, cc3_row_counter_ii);
				  }
				
				/* if its a face, go to next cascade otherwise quit */
				if (feat_sum > CC3_GLOBAL_THRESHOLD[curr_cascade])
				  {
				    face = 1; 
				    curr_cascade++;
				  }
				else 
				  {
				    face = 0;
				  }
			      }
                             
			   
			    if (face)
			      {
                                #ifdef SAVE_IMAGES
				fprintf(fout, "%d %d %d \n",curr_pos_x+1, cc3_row_counter_cropped_img+1, CC3_SCALES[curr_scale_idx]-1);
				#endif
		
				printf("Face Detected at: %d %d, Size: %d \n\r",curr_pos_x+1, cc3_row_counter_cropped_img+1, CC3_SCALES[curr_scale_idx]-1);
				cc3_num_detected_faces++;
				
			      }
			    
			    
			  } /* end of if (std > lower_bound) and ( lower_bound < mean < upper_bound) */
			
		      } /* end of iterating over horizontally shifted sub-windows */
		    
		  } /* end of if loop for this particular scale at this row */
		
		
	      } /* end of iterating over all scales */
	    
	    /*  increment the row counters */
	    cc3_row_counter_actual_img++;
	    cc3_row_counter_cropped_img++;
	    cc3_row_counter_ii = (cc3_row_counter_ii + 1) % CC3_INTEGRAL_IMG_HEIGHT; /* circular wrapping of integral image row counter */
	    cc3_row_counter_calc_ii++;
	    
	    
	  } /* end of iterating over all the rows in the actual image (upto bottom_offset) */
	

       //	printf("No. of faces : %d \n\r", cc3_num_detected_faces);     
       
       /* end of frame */
       printf ("Frame Done..\n\r");
	#ifdef SAVE_IMAGES

       fprintf( fout, "%d %d %d \n",0,0,0);
       fclose(fout);
       fclose(fp);
       #endif
	
	num_frames++;

       	
   	cc3_led_set_state(0,0);
   	cc3_led_set_state(2,1);
	while(!cc3_button_get_state());
   	cc3_led_set_state(0,1);
   	cc3_led_set_state(2,0);
	// wait for the button to be pressed for the next frame
	

	// sample non-blocking serial routine
	if(!cc3_uart_has_data(0) ) break; 
     } // end of while 
   
    free(cc3_img_tmp.pix);  // don't forget to free!

    while(1);
    
    return 0;
}


