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

// integral image counter (for many rows, integral image has been calculated)
uint16_t cc3_row_counter_calc_ii;

uint32_t mean_val;       // mean for a subwinow
uint64_t sum_pix;        // sum of pixels in a sub-window
uint64_t sum_pix_sq; // sum of squares of pixels in a subwindow
uint32_t std;        // standard deviation
uint64_t var;        // variance

// array to store rows while transfering b/w fifo and ii
static uint8_t image_row[176*3];


FILE* fp;

// function to get the current segment from the actual image
void cc3_get_curr_segment()
{
  cc3_pixel_t pix_temp;
  
 

  if (cc3_row_counter_cropped_img == 0)  // first time
    {
      // load the upper "CC3_INTEGRAL_IMG_HEIGHT" rows
      for (uint8_t i = 0; i < CC3_INTEGRAL_IMG_HEIGHT; i++)
	{
	  // get a row..
	  cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  
	  // copy the first pixel
	  cc3_get_pixel(&cc3_img_tmp, 0, 0, &pix_temp);
	  cc3_integral_image[i][0] = pix_temp.channel[1]; // only green channel
	  //	  cc3_integral_image[i][0] = (3*pix_temp.channel[0]+6*pix_temp.channel[1]+pix_temp.channel[2])/10; // rgb->gray 

	  printf("writing to file %u \n\r", i);
	  fprintf(fp, "%d ", cc3_integral_image[i][0]);
	  printf("wrot to file %u \n\r", i);
	  
	  // start copying from the next pixel and calculate cumulative sum at the same time
	  for (uint16_t j = 1; j < cc3_img_tmp.width; j++)
	    {
	      cc3_get_pixel(&cc3_img_tmp, j, 0, &pix_temp);
	      cc3_integral_image[i][j] = pix_temp.channel[1]; // only green channel
	      //  cc3_integral_image[i][j] = (3*pix_temp.channel[0]+6*pix_temp.channel[1]+pix_temp.channel[2])/10; // rgb->gray 
	      fprintf( fp,"%d ",cc3_integral_image[i][j] );
	      
	      // compute cumulative sum across the row
	      cc3_integral_image[i][j] += cc3_integral_image[i][j-1];
	    }
	  fprintf( fp, "\n" );
	}
      
      // find cumulative sum along columns to complete the integral image computation
      for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	 {
	   for (uint16_t i = 1; i < CC3_INTEGRAL_IMG_HEIGHT; i++)  // start from second row
	     {
	       cc3_integral_image[i][j]+=cc3_integral_image[i-1][j];
	     }
	 }


      // printf the integral image
      for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_HEIGHT; j++)
	{
	   for (uint16_t i = 0; i < CC3_INTEGRAL_IMG_WIDTH; i++)
	     {
	       //	       printf("%ld ", cc3_integral_image[j][i] );
	     }
	   //	   printf("\n\r");
	}
         
    }
  
  else 
    { 
      // get a new row
      cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  
      uint8_t newly_added_row = cc3_row_counter_calc_ii % CC3_INTEGRAL_IMG_HEIGHT;
      uint8_t prev_row = (cc3_row_counter_calc_ii - 1 + CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT; 
       
      // copy the first pixel
      cc3_get_pixel(&cc3_img_tmp, 0, 0, &pix_temp);
      cc3_integral_image[newly_added_row][0] = pix_temp.channel[1];
      //      cc3_integral_image[newly_added_row][0] = (3*pix_temp.channel[0]+6*pix_temp.channel[1]+pix_temp.channel[2])/10; // rgb->gray 
      fprintf( fp,"%d ",cc3_integral_image[newly_added_row][0] );
      
      // read the row, from next pixel onward and compute cum sum across the row
      for (uint16_t j = 1; j < cc3_img_tmp.width; j++)
	{
	  cc3_get_pixel(&cc3_img_tmp, j, 0, &pix_temp);
	  cc3_integral_image[newly_added_row][j] = pix_temp.channel[1];
	  //	  cc3_integral_image[newly_added_row][j] = (3*pix_temp.channel[0] + 6*pix_temp.channel[1] + pix_temp.channel[2])/10;
	  fprintf( fp,"%d ",cc3_integral_image[newly_added_row][j] );

	  // compute cumulative sum across the row
	  cc3_integral_image[newly_added_row][j] += cc3_integral_image[newly_added_row][j-1];
	}
       fprintf( fp, "\n" );

       printf("%s %d \n\r", "New Row...", cc3_row_counter_calc_ii);
       // find cumulative sum along columns to complete the integral image computation
       for (uint16_t j = 0; j < CC3_INTEGRAL_IMG_WIDTH; j++)
	 {
	   cc3_integral_image[newly_added_row][j]+=cc3_integral_image[prev_row][j];
	   //   printf("%ld ", cc3_integral_image[newly_added_row][j] );
	 }
       //            printf("\n\r");
      
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

  // evaluate the subwindow for this feature
  for (uint8_t pt=0; pt < 9 ; pt++)
    {
      x_in_ii = x + CC3_FACE_FEATURES[feat_num][curr_scale].x[pt];
      y_in_ii = (y + CC3_FACE_FEATURES[feat_num][curr_scale].y[pt]) % CC3_INTEGRAL_IMG_HEIGHT; // circular warping
      fval+=cc3_integral_image[y_in_ii][x_in_ii]*CC3_FACE_FEATURES[feat_num][curr_scale].val_at_corners[pt];
    }

  // taking into account the effect of normalization
  printf("%s %d", "fval before std", fval);
  fval = fval/((int32_t)std);
  printf("%s %d", "fval after std", fval);

  // check if fval < threshold
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
  // int16_t feat_val;

   uint8_t x2, y2; // lowe right sub-window coordinates
   uint64_t num_pixels; // no. of pixels in a subwindow

   int32_t temp1, temp2; // temp variables used for computations
  
    // setup system    
    cc3_system_setup ();

    // configure uarts
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
   
    cc3_camera_init ();
 
    cc3_set_colorspace(CC3_RGB);
    cc3_set_resolution(CC3_LOW_RES);
    cc3_set_auto_white_balance(true);
    cc3_set_auto_exposure(true);
     
    printf("Face Detector...\n\r");
    
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
    // if ((cc3_img_tmp.pix = malloc(cc3_img_tmp.width*cc3_img_tmp.channels)) == NULL)
    //      printf("Cannot allocate memory \n\r");  // malloc! (to hold one row RGB)
     cc3_img_tmp.pix = &image_row;

    if (cc3_img_tmp.width != CC3_INTEGRAL_IMG_WIDTH)
      printf("Error...image width and integral image width different \n\r");

    
    fp=fopen("c:/test.pgm","w" );
    fprintf( fp, "P2\n%d %d\n255\n", cc3_g_current_frame.width, cc3_g_current_frame.height-top_offset-bottom_offset );

    while(1) 
      {
	cc3_num_detected_faces = 0;

	printf(" New Frame...\n\r");
	// This tells the camera to grab a new frame into the fifo and reset
	// any internal location information.
	cc3_pixbuf_load();
	
	printf(" captured the image \n\r");
	// discard some top rows, given by top_offset
	for (uint8_t i = 0; i < top_offset ; i++)
	  {
	    cc3_pixbuf_read_rows(cc3_img_tmp.pix, cc3_img_tmp.width, cc3_img_tmp.height);
	  }
	
	
	printf(" read top offset \n\r");
	// indicating how many rows have we read in the actual full image
	// top_offset rows are discarded and next "CC3_INTEGRAL_IMG_HEIGHT" rows are read in the first 
	// instance of the following for loop
	cc3_row_counter_actual_img = (top_offset-1)+1;   // index starts from 0
	cc3_row_counter_ii = 0;
	cc3_row_counter_cropped_img = 0;
	cc3_row_counter_calc_ii = (CC3_INTEGRAL_IMG_HEIGHT - 1); 
	
	printf(" reached the main while loop \n\r");
	// iterating over the rows of the actual image until the bottom offset
	while (cc3_row_counter_actual_img < (CC3_IMAGE_HEIGHT - bottom_offset))
	  { 
	    //  printf("%s %d \n\r", "CURRENT ROW ",cc3_row_counter_cropped_img);
	    if (cc3_row_counter_calc_ii < CC3_IMAGE_HEIGHT - top_offset - bottom_offset) // check if we need to load any more rows?
	      {
		// get the current segment and its integral image
		cc3_get_curr_segment();
	      }
	    
	    printf("got the curr seg \n\r");
	    for (uint8_t curr_scale_idx = 0; curr_scale_idx < CC3_NUM_SCALES; curr_scale_idx++)
	      {
		
		//	printf (" Current scale %d \n\r", CC3_SCALES[curr_scale_idx]);
		// check if we need to evaluate subwindows at this scale for this sub-window
		if (cc3_rows_to_eval_feat[cc3_row_counter_cropped_img][curr_scale_idx] == 1)
		  { 
		    // iterate over all horizontal shifted sub-window
		    for (uint8_t curr_pos_x =0; curr_pos_x < CC3_INTEGRAL_IMG_WIDTH - CC3_SCALES[curr_scale_idx]; curr_pos_x+=CC3_WIN_STEPS[curr_scale_idx])
		      { 
			//	printf ("current pos_x %d \n\r",curr_pos_x);
			// compute the standard deviation for this window
			x2 = curr_pos_x + CC3_SCALES[curr_scale_idx]-1 ;
			y2 = (cc3_row_counter_ii + CC3_SCALES[curr_scale_idx]-1) % CC3_INTEGRAL_IMG_HEIGHT;
			if (x2 >= CC3_INTEGRAL_IMG_WIDTH ) 
			  {
			    printf("Error....width outside limits!! \n\r");
			  }
			

			// Don't consider the first row and first column in the sub-window for calculating 
			// std
			num_pixels = (CC3_SCALES[curr_scale_idx]-1)*(CC3_SCALES[curr_scale_idx]-1);
			
			temp1 = cc3_integral_image[cc3_row_counter_ii][curr_pos_x] -  cc3_integral_image[cc3_row_counter_ii][x2];
			temp2 = cc3_integral_image[y2][x2] - cc3_integral_image[y2][curr_pos_x];
			sum_pix = (uint32_t) (temp1 + temp2); 
			mean_val = sum_pix/num_pixels;
			
			//	printf("%s %u %u %u %u \n\r"," II ",  cc3_integral_image[cc3_row_counter_ii+1][curr_pos_x+1], cc3_integral_image[cc3_row_counter_ii+1][x2], cc3_integral_image[y2][curr_pos_x+1], cc3_integral_image[y2][x2]);

			printf("%s %d \n\r", "Mean: ", mean_val);
			printf("%s %d \n\r", "Num Pixels: ", num_pixels);
			sum_pix_sq = 0;
			for (uint8_t i = (cc3_row_counter_ii+1) % CC3_INTEGRAL_IMG_HEIGHT; i != (y2+1)% CC3_INTEGRAL_IMG_HEIGHT; )
			  {
			    for (uint8_t j = curr_pos_x+1; j <= x2; j++)
			      {
				temp2 = (i-1+CC3_INTEGRAL_IMG_HEIGHT) % CC3_INTEGRAL_IMG_HEIGHT;
				temp1 = cc3_integral_image[i][j] - cc3_integral_image[i][j-1];
				temp2 = -cc3_integral_image[temp2][j] + cc3_integral_image[temp2][j-1];
				temp1 = temp1+temp2;
				
				//	if (cc3_row_counter_ii == 0)
				//	 printf("%s %u \n\r", "  ",temp1);

				sum_pix_sq+=temp1*temp1;
			      }
			    //	    printf("\n");
			    i = (i+1) % CC3_INTEGRAL_IMG_HEIGHT; 
			  }
			
			uint64_t numer, denom;
			printf("%s %d %d %d \n\r","current wind ",cc3_row_counter_cropped_img, curr_pos_x, CC3_SCALES[curr_scale_idx]);
			printf("%s %u \n\r", "Sum_pix ",sum_pix);
			printf("%s %u \n\r", "Square Sum Pix: ",sum_pix_sq); 
			
			numer = (num_pixels*sum_pix_sq);
			numer =  (numer - sum_pix*sum_pix);
			//	printf("%s %u \n\r", "var: numerator", numer);
			
			denom = (num_pixels*num_pixels);
			//	printf("%s %u \n\r", "var: denom", denom);
			
			var = numer/denom;            //variance
			//	var = (sum_pix_sq/num_pixels) - mean_val*mean_val;
			//	mean_val = sum_pix/num_pixels;
			
			printf("%s %u \n\r", "Var ",var);
			//printf("%s %u \n\r", "Mean: ", mean_val);
			
			if (var < 25)
			  {
			    std = 1;  // basically reject the windows with this variance
			  }
			
			else
			  {
			    // find the standard deviation
			    for (uint64_t i = 4; i < var/2 ; i++)
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
			
			printf("%s %u \n\r\n", "STD ",std);
			
			//	std = 5;
			
			if ( (std > 0) & (mean_val > 10) & (mean_val < 260)) // check for face only if std > 3, otherwise its too uniform 
			  {
			    //	    printf("%s \n\r\n", "Checking for face ");
			    
			    int16_t feat_sum = 0;
			    int16_t curr_sum;
			    // compute the val of this sub-window for all the features
			    for (uint8_t curr_feat_idx = 0; curr_feat_idx < CC3_NUM_FEATURES; curr_feat_idx++)
			      {
				curr_sum =cc3_get_feat_val(curr_feat_idx, curr_scale_idx, curr_pos_x, cc3_row_counter_ii);
				printf("%s %d \n\r", "feat alpha ", curr_sum);
				feat_sum+=curr_sum;
			      }
			    
			    //	    printf("After finding feat val \n\r");
			    
			    // check if its a face
			    if (feat_sum > CC3_GLOBAL_THRESHOLD)
			      {
				printf("Found a face \n\r");
				//				printf(" global thersh %d \n\r",CC3_GLOBAL_THRESHOLD);
				// found a face....or the code says so..(and hope so!!!)
				if (cc3_num_detected_faces < CC3_MAX_FACES )
				  {
				    //				    printf(" Face found... %d \n\r", cc3_num_detected_faces);
				    
				    cc3_faces[cc3_num_detected_faces][0] = curr_pos_x;
				    cc3_faces[cc3_num_detected_faces][1] = cc3_row_counter_cropped_img; // + top_offset;
				    cc3_faces[cc3_num_detected_faces][2] = CC3_SCALES[curr_scale_idx];
				    cc3_num_detected_faces++;
				  }
				
				// as of now, if MAX_FACES already reached, do nothing...(for a change, Relax!!)
				
			      }
			    
			    //	    printf("end of detecing face \n\r");
			  } // end of if (std > 5)
			
			//	printf(" end of iterating over horizintally shisted win \n\r");
		      } // end of iterating over horizontally shifted sub-windows
		  }
		
		//	printf(" End of iterating over all scales \n\r");
		//		printf(" Printing the val of cc3_row_counter_cropped_img %d \n\r",cc3_row_counter_cropped_img);
	      } // end of iterating over all scales
	    
	    //	       printf("End for curr cropped row %d \n\r", cc3_row_counter_cropped_img);
	    
	    // increment the row counters
	    cc3_row_counter_actual_img++;
	    cc3_row_counter_cropped_img++;
	    cc3_row_counter_ii = (cc3_row_counter_ii + 1) % CC3_INTEGRAL_IMG_HEIGHT; // circular wrapping of integral image row counter
	    cc3_row_counter_calc_ii++;
	    
	    
	  } // end of iterating over all the rows in the actual image (upto bottom_offset)
	
	printf("No. of faces : %d \n\r", cc3_num_detected_faces);     
	
	for (uint8_t i = 0; i < cc3_num_detected_faces; i++)
	  printf ( " Face at x=%d, y=%d scale=%d \n\r",cc3_faces[i][0], cc3_faces[i][1], cc3_faces[i][2]);
	
	fclose(fp);
	//	printf(" file closed...");
	
	break;
	
	// sample non-blocking serial routine
		if(!cc3_uart_has_data(0) ) break; 
      } // end of while 
    
    
    
    free(cc3_img_tmp.pix);  // don't forget to free!
    printf( "You pressed %c to escape \n\r",fgetc(stdin) );
    
    // stdio actually works... 
    printf( "Type in a number followed by return to test scanf: " );
    scanf( "%d", &val );
    printf( "You typed %d\n",val );
    
    printf( "Good work, now try something on your own...\n" );
    while(1);
    
    return 0;
}


