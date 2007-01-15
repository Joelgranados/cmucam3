#include "conv.h"

int convolve(cc3_image_t img, filter_t filter)
{
uint32_t i,j,k,l,mat_div;
cc3_pixel_t p;

mat_div=filter.size*filter.size;
printf( "convolution\n" );
printf( "img = %d, %d\n",img.width,img.height );
printf( "filter = %d\n",filter.size);

for(j=0; j<img.height-filter.size+1; j++ )
	for(i=0; i<img.width-filter.size+1; i++ )
	{
	uint32_t tmp;
	tmp=0;	
	for(k=0; k<filter.size; k++ )
		for(l=0; l<filter.size; l++ )
		{
		cc3_get_pixel (&img, i+k, j+l, &p);		
		tmp+=p.channel[0]*filter.mat[k][l];
		}
	p.channel[0]=tmp / mat_div;
	cc3_set_pixel (&img, i, j, &p);		
	}

}
