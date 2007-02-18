#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>


#define COLOR_THRESH   25 
#define MIN_BLOB_SIZE  15

#define WIDTH	88
#define HEIGHT	72

// Constants for connected component blob reduce
#define SELECTED	255
#define NOT_SELECTED	0
#define MARKED		2	
#define FINAL_SELECTED	1	


uint8_t p_img[WIDTH][HEIGHT];

void connected_component_reduce(int min_blob_size); 
void generate_histogram(uint8_t hist[]);
void convert_histogram_to_ppm(uint8_t hist[]);
void matrix_to_ppm();
int count(int x, int y,int steps);
int reduce(int x, int y,int steps,int remove);
 
/* simple hello world, showing features and compiling*/
int main (void)
{
    uint32_t last_time,val;
    int cnt;
    char c;
    uint8_t range[WIDTH];
   
    cc3_image_t img;

    // configure uarts
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
    // Make it so that stdout and stdin are not buffered
    val=setvbuf(stdout, NULL, _IONBF, 0 );
    val=setvbuf(stdin, NULL, _IONBF, 0 );
    
    cc3_camera_init ();
   
    cc3_camera_set_colorspace(CC3_RGB);
    cc3_camera_set_resolution(CC3_LOW_RES);
    cc3_camera_set_auto_white_balance(true);
    cc3_camera_set_auto_exposure(true);
    

    cc3_pixbuf_set_subsample( CC3_NEAREST, 2, 2 ); 
    cc3_pixbuf_set_coi(CC3_RED);
    
    cc3_led_set_off (0);
    cc3_led_set_off (1);
    cc3_led_set_off (2);
   
    // sample wait command in ms 
    cc3_timer_wait_ms(1000);
    cc3_led_set_on (0);

    cnt=0;
        // setup an image structure 
    	//img.channels=CC3_GREEN;
    	img.channels=1;
    	img.width=cc3_g_current_frame.width;
    	img.height=cc3_g_current_frame.height;  // image will hold just 1 row for scanline processing
    	//img.pix = cc3_malloc_rows(1);
    	img.pix = cc3_malloc_rows(cc3_g_current_frame.height);
    if(img.pix==NULL ) { printf( "Not enough memory...\n" ); exit(0); }	
    printf( "img size = %d, %d\n",cc3_g_current_frame.width, cc3_g_current_frame.height);

    while(1)
    {
    	cc3_pixel_t mid_pix;
    	cc3_pixel_t right_pix;
    	cc3_pixel_t down_pix;
    for(int y=0; y<HEIGHT; y++ )
	for(int x=0; x<WIDTH; x++ )
		p_img[x][y]=0;
	cc3_pixbuf_load();
	cc3_pixbuf_read_rows(img.pix,cc3_g_current_frame.height);
	
	for(int y=0; y<img.height-1; y++ )
	{
		for(int x=0; x<img.width-1; x++ )
		{
		int tmp;
		int m,r,d;
		cc3_get_pixel( &img, x, y, &mid_pix );
		cc3_get_pixel( &img, x+1, y, &right_pix );
		cc3_get_pixel( &img, x, y+1, &down_pix );
		m=mid_pix.channel[0];
		r=right_pix.channel[0];
		d=down_pix.channel[0];
		if( m<r-COLOR_THRESH ||
		    m>r+COLOR_THRESH )
		   p_img[x][y]=SELECTED;		
		if( m<d-COLOR_THRESH ||
		    m>d+COLOR_THRESH )
		   p_img[x][y]=SELECTED;		
		
		}
	}
  

	connected_component_reduce(MIN_BLOB_SIZE); 
	generate_histogram(range );
	convert_histogram_to_ppm(range );
	//matrix_to_ppm();
	
 printf( "Frame done. time=%d\n",cc3_timer()-last_time );
    last_time=cc3_timer();
 
    }


    free(img.pix);  // don't forget to free!

    while(1);

  return 0;
}

int count(int x, int y,int steps)
{
int size;
int width,height;
width=WIDTH;
height=HEIGHT;
 
   size=0;
   if(p_img[x][y]==SELECTED) size=1;
   steps--;
   if(steps==0) return size;
   p_img[x][y]=MARKED;
   
   if(x>1 && p_img[x-1][y]==SELECTED) size+=count(x-1,y,steps);
   if(x<width && p_img[x+1][y]==SELECTED) size+=count(x+1,y,steps);
   if(y>1 && p_img[x][y-1]==SELECTED) size+=count(x,y-1,steps);
   if(y<height && p_img[x][y+1]==SELECTED) size+=count(x,y+1,steps);

   if(x>1 && y>1 && p_img[x-1][y-1]==SELECTED) size+=count(x-1,y-1,steps);
   if(x<width &&  y>1 && p_img[x+1][y-1]==SELECTED) size+=count(x+1,y-1,steps);
   if(x>1 &&  y<height && p_img[x-1][y+1]==SELECTED) size+=count(x-1,y+1,steps);
   if(x<width &&  y<height && p_img[x+1][y+1]==SELECTED) size+=count(x+1,y+1,steps);

return size;
}

int reduce(int x, int y,int steps,int remove)
{
int size;
int width,height;
width=WIDTH;
height=HEIGHT;

   size=0;
   if(p_img[x][y]==MARKED) size=1;
   steps--;
   if(steps==0) return size;
   
   if(remove) p_img[x][y]=NOT_SELECTED;
	else p_img[x][y]=FINAL_SELECTED;
   
   if(x>1 && p_img[x-1][y]==MARKED) size+=reduce(x-1,y,steps,remove);
   if(x<width && p_img[x+1][y]==MARKED) size+=reduce(x+1,y,steps,remove);
   if(y>1 && p_img[x][y-1]==MARKED) size+=reduce(x,y-1,steps,remove);
   if(y<height && p_img[x][y+1]==MARKED) size+=reduce(x,y+1,steps,remove);

   if(x>1 && y>1 && p_img[x-1][y-1]==MARKED) size+=reduce(x-1,y-1,steps,remove);
   if(x<width &&  y>1 && p_img[x+1][y-1]==MARKED) size+=reduce(x+1,y-1,steps,remove);
   if(x>1 &&  y<height && p_img[x-1][y+1]==MARKED) size+=reduce(x-1,y+1,steps,remove);
   if(x<width &&  y<height && p_img[x+1][y+1]==MARKED) size+=reduce(x+1,y+1,steps,remove);

return size;
}



void connected_component_reduce(int min_blob_size) 
{
int x,y,size;
int width,height;
width=WIDTH;
height=HEIGHT;

    for(y=0; y<height; y++ )
	for(x=0; x<width; x++ )
	{
	
		if(p_img[x][y]==SELECTED)
			{
			size=count(x,y,min_blob_size);
			if(size<min_blob_size) 
				reduce(x,y,min_blob_size,1);  // Delete marked
			else reduce(x,y,min_blob_size,0);  // Finalize marked
			}
	}


	// Translate for PPM
    	for(y=0; y<height; y++ )
	for(x=0; x<width; x++ ) if(p_img[x][y]==FINAL_SELECTED) p_img[x][y]=SELECTED;
}


void generate_histogram(uint8_t hist[] )
{
int x,y;
int width,height;
static int cnt=0;
width=WIDTH;
height=HEIGHT;

for(x=0; x<width; x++ ) hist[x]=0;


	for(x=width/2; x<width; x++ )
	{
    	
	for(y=height-1; y>0; y-- )
		{	
		if(p_img[x][y]==SELECTED)
			break;
		}
	hist[x]=(height-1-y);
	if(y>height-5) break;
	}

	for(x=width/2; x>0; x-- )
	{
    	
	for(y=height-1; y>0; y-- )
		{	
		if(p_img[x][y]==SELECTED)
			break;
		}
	hist[x]=(height-1-y);
	if(y>height-5) break;
	}

	// Downsample Histogram 
  	for(x=0; x<width-5; x+=5)
	{
	int min;
	int j;
        min=100;
	
	for(j=0; j<5; j++ )
		{
		if(hist[x+j]<min) min=hist[x+j];
		}
	for(j=0; j<5; j++ ) hist[x+j]=min;
	}	
	

	


}

void convert_histogram_to_ppm(uint8_t hist[] )
{
int x,y;
int width,height;
width=WIDTH;
height=HEIGHT;
	// Write the range image out	
    	for(y=0; y<height; y++ )
	for(x=0; x<width; x++ ) p_img[x][y]=0;

	for(x=0; x<width; x++ )
	{
    	
	for(y=height-1; y>height-1-(hist[x]); y-- )
		{	
		p_img[x][y]=255;
		}
	}
}



void matrix_to_ppm()
{
static int cnt=0;
char str[32];
FILE *fp;
int width,height;
width=WIDTH;
height=HEIGHT;

   	sprintf( str,"out_%d.pgm",cnt );
   	cnt++; 
   	fp=fopen( str,"w" );
   	if(fp==NULL ) { printf( "Can't open file...\n" ); exit(0); }
   	fprintf( fp,"P5\n%d %d 255\n",width,height);
	for(int y=0; y<height; y++ )
	{
		for(int x=0; x<width; x++ )
		fprintf( fp,"%c",p_img[x][y]);
	}
        fclose(fp);	

}
