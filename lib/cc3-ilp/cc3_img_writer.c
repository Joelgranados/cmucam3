#include "cc3_img_writer.h"

/*
 * This will generate a ppm for 3 channel images or 
 * a pgm for single channel images.
 *
 */
int cc3_img_write_file_create( cc3_image_t *img)
{
char filename[32];
uint8_t ppm,val;
FILE *f;
static uint32_t ppm_cnt=0;
if(img->channels==CC3_CHANNEL_ALL ) ppm=1;
else ppm=0;
do {
#ifdef VIRTUAL_CAM
	if(ppm)
    	sprintf(filename, "img%.5d.ppm", ppm_cnt);
	else sprintf(filename, "img%.5d.pgm", ppm_cnt);
#else
	if(ppm)
    	sprintf(filename, "c:/img%.5d.ppm", ppm_cnt);
	else sprintf(filename, "c:/img%.5d.pgm", ppm_cnt);
#endif
	f = fopen(filename, "r");
    	if(f!=NULL ) { 
		printf( "%s already exists...\n",filename ); 
		ppm_cnt++; 
		fclose(f);
		}
    } while(f!=NULL);
ppm_cnt++;
if(ppm) val= cc3_ppm_img_write(img, filename);
else val= cc3_pgm_img_write(img, filename);
return val;
}

int cc3_ppm_img_write(cc3_image_t *img, char *filename)
{
FILE *fp;
uint32_t size_x, size_y,x,y;
cc3_pixel_t p;

size_x = img->width;
size_y = img->height;
if(img->channels!=CC3_CHANNEL_ALL) 
{
	printf( "ppm_write only works with 3 channel images\n" );
	return 0;
}
    fp = fopen(filename, "w"); 
    if(fp==NULL )
    {
    	fprintf(stderr,"Can't open file %s\r\n", filename);
    	return 0;
    }
  fprintf(fp,"P3\n%d %d\n255\n",size_x,size_y );
  
  for (y = 0; y < size_y; y++) {
    for (x = 0; x < size_x * 3; x++) {
     	cc3_get_pixel(img,x,y,&p); 
	fprintf(fp,"%d %d %d ",p.channel[0],p.channel[1],p.channel[2]);
    }
  fprintf(fp,"\n");
  }

  fclose(fp);
return 1;
}


int cc3_pgm_img_write(cc3_image_t *img, char *filename)
{
FILE *fp;
uint32_t size_x, size_y,x,y;
cc3_pixel_t p;

size_x = img->width;
size_y = img->height;
    fp = fopen(filename, "w"); 
    if(fp==NULL )
    {
    	fprintf(stderr,"Can't open file %s\r\n", filename);
    	return 0;
    }
  fprintf(fp,"P5\n%d %d\n255\n",size_x,size_y );
  
  for (y = 0; y < size_y; y++) {
    for (x = 0; x < size_x; x++) {
     	cc3_get_pixel(img,x,y,&p); 
	fprintf(fp,"%c",p.channel[0]);
    }
  }

  fclose(fp);
return 1;
}
