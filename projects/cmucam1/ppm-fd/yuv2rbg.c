#include <stdio.h>
#include <stdlib.h>


int main()
{
FILE *fp,*out_fp,*mono_fp;
int x,y,x_size,y_size,depth;
int yc,uc,vc,r,g,b;
char type[32];

out_fp=fopen("rgb.ppm","w" );
mono_fp=fopen("mono.ppm","w" );
fp=fopen("out.ppm","r" );
if(fp==NULL) {
	printf( "File not found\n" );
	exit(0);
}
fscanf( fp,"%s\n%d %d\n%d\n",&type, &x_size,&y_size, &depth );
printf( "type: %s\nSize: %d, %d\nDepth: %d\n",type, x_size, y_size, depth );
fprintf( out_fp,"%s\n%d %d\n%d\n",type, x_size,y_size,depth );
fprintf( mono_fp,"%s\n%d %d\n%d\n",type, x_size,y_size,depth );
for(x=0; x<x_size; x++ )
for(y=0; y<y_size; y++ )
	{
	yc=fgetc(fp);
	uc=fgetc(fp);
	vc=fgetc(fp);
	//b = 1.164*(yc - 16) + 2.018*(uc - 128);
	//g = 1.164*(yc - 16) - 0.813*(vc - 128) - 0.391*(uc - 128);
	//r = 1.164*(yc - 16) + 1.596*(vc - 128);
	r = yc + 1.403*vc;
	g = yc - 0.344*uc - 0.714 * vc;
	b = yc + 1.770 * uc;
	if (r > 255) r = 255;
   	if (g > 255) g = 255;
   	if (b > 255) b = 255;
   	if (r < 0) r = 0;
   	if (g < 0) g = 0;
   	if (b < 0) b = 0;

	fprintf( out_fp,"%c%c%c",r,g,b);
	fprintf( mono_fp,"%c%c%c",yc,yc,yc);
	}
fclose(mono_fp);
fclose(out_fp);
fclose(fp);
}
