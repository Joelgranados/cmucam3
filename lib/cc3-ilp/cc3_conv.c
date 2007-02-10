#include "cc3_conv.h"

/*
  cc3_convolve_img()
  This is a simple integer image convolution.  
  This will take an image and a kernel and convolve the kernel
  across the image replacing its original content.

  The max size of the convolution kernel MAX_KERNEL_SIZE is 
  defined in cc3_conv.h.

  The function returns 1 upon success and 0 on failure.
 */ 
int cc3_convolve_img(cc3_image_t *img, cc3_kernel_t kernel)
{
uint32_t i,j,k,l,mat_div;
cc3_pixel_t p;
if(kernel.size>MAX_KERNEL_SIZE) return 0;
if( img->height<kernel.size+1 ) return 0;
if( img->width<kernel.size+1 ) return 0;

mat_div=kernel.divisor;
for(j=0; j<img->height-kernel.size+1; j++ )
	for(i=0; i<img->width-kernel.size+1; i++ )
	{
	uint32_t tmp;
	tmp=0;	
	for(k=0; k<kernel.size; k++ )
		for(l=0; l<kernel.size; l++ )
		{
		cc3_get_pixel (img, i+k, j+l, &p);		
		tmp+=p.channel[0]*kernel.mat[k][l];
		}

	p.channel[0]=tmp / mat_div;
	cc3_set_pixel (img, i, j, &p);		
	}
return 1;
}
