#include <stdio.h>
#include <stdlib.h>

int clip(int x)
{
if(x>255) x=255;
if(x<0) x=0;
return x;
}

int main()
{
FILE *fp,*out_fp,*mono_fp;
int x,y,x_size,y_size,depth;
int yc,uc,vc,r,g,b,C,D,E;
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
	//r = yc + 1.403*vc;
	//g = yc - 0.344*uc - 0.714 * vc;
	//b = yc + 1.770 * uc;
	//r = yc + 1.402 * (uc-128);
	//g = yc - 0.34414 * (vc-128) - 0.71414 * (uc-128);
	//b = yc + 1.772 * (vc-128);
	C= yc - 16;
	D= uc - 128;
	E= vc - 128;

	r = ( 298 * C           + 409 * E + 128) >> 8;
	g = ( 298 * C - 100 * D - 208 * E + 128) >> 8;
	b = ( 298 * C + 516 * D           + 128) >> 8;

	r=clip(r);
	g=clip(g);
	b=clip(b);

	fprintf( out_fp,"%c%c%c",r,g,b);
	fprintf( mono_fp,"%c%c%c",yc,yc,yc);
	}
fclose(mono_fp);
fclose(out_fp);
fclose(fp);
}
