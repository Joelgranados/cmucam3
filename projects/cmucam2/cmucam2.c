#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_color_track.h>
#include <cc3_color_info.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ARGS     10
#define MAX_LINE     128 
#define VERSION_BANNER "CMUcam2 v1.00 c6"

typedef enum {
	RETURN,
	RESET,
	TRACK_COLOR,
	SEND_FRAME,
	FRAME_DIFF,
	GET_VERSION,
	GET_MEAN,
	SET_SERVO,
	CAMERA_REG,
	POLL_MODE,
	LINE_MODE,
	VIRTUAL_WINDOW,
	DOWN_SAMPLE,
	CMUCAM2_CMD_END // Must be last entry so array sizes are correct
} cmucam2_command_t;

char *cmucam2_cmds[CMUCAM2_CMD_END];

void cmucam2_get_mean(cc3_color_info_pkt_t *t_pkt, uint8_t poll_mode, uint8_t line_mode  );
void cmucam2_write_s_packet(cc3_color_info_pkt_t *pkt);
void cmucam2_track_color(cc3_track_pkt_t *t_pkt, uint8_t poll_mode, uint8_t line_mode  );
int32_t cmucam2_get_command(int32_t *cmd, int32_t *arg_list);
void set_cmucam2_commands(void);
void print_ACK(void);
void print_NCK(void);
void cmucam2_write_t_packet(cc3_track_pkt_t *pkt);

int main (void)
{
int32_t command;
int32_t val,n;
uint32_t arg_list[MAX_ARGS];
uint8_t error,poll_mode,line_mode;
cc3_track_pkt_t t_pkt;
cc3_color_info_pkt_t s_pkt;
    
    set_cmucam2_commands();

    cmucam2_start: 
    poll_mode=0;
    line_mode=0;
    cc3_system_setup ();

    cc3_uart_init (0, 
		   CC3_UART_RATE_115200,
		   CC3_UART_MODE_8N1,
		   CC3_UART_BINMODE_BINARY);
    val=setvbuf(stdout, NULL, _IONBF, 0 );
    
    cc3_camera_init ();
   
    printf ("%s\r",VERSION_BANNER);
    //cc3_set_led (true);

    cc3_servo_init(); 
    cc3_pixbuf_set_subsample( CC3_NEAREST, 2, 1 ); 
    
   while (1) {
    printf( ":" );
    error=0;
    n=cmucam2_get_command(&command, arg_list );	
    if(n!=-1)
    {
    switch(command)
    {

    	case RESET: 
	    if(n!=0) { error=1; break; } else print_ACK();
	    printf( "\r" );
	    goto cmucam2_start;
	    break; 
	case GET_VERSION:
	    if(n!=0) { error=1; break; } else print_ACK();
	    printf( "%s\r",VERSION_BANNER );	
	    break;
	case POLL_MODE:
	    if(n!=1) { error=1; break; } else print_ACK();
	    if(arg_list[0]==1 ) poll_mode=1;
	    else poll_mode=0;
	    break;
	case LINE_MODE:
	    if(n!=2) { error=1; break; } else print_ACK();
	    // FIXME: Make bitmasks later
	    if(arg_list[0]==0 ) 
	    {
		if(arg_list[1]==1)
		    line_mode=1;
	    	else line_mode=0;
	    }
	    break;
	case SEND_FRAME:
	    if(n==1 && arg_list[0]>4) { error=1; break; } 
	    else if(n>1) { error=1; break; } else print_ACK();
	    cc3_send_image_direct();
	    break;
	case CAMERA_REG:
	    if(n%2!=0 ||  n<2) { error=1; break; } else print_ACK();
	    for(int i=0; i<n; i+=2)
	    	cc3_set_raw_register(arg_list[i], arg_list[i+1] );
	    break;
	case VIRTUAL_WINDOW:
	    if(n!=4) { error=1; break; } else print_ACK();
	    cc3_pixbuf_set_roi( arg_list[0], arg_list[1], arg_list[2], arg_list[3] );
	    break;
	case DOWN_SAMPLE:
	    if(n!=2) { error=1; break; } else print_ACK();
	    cc3_pixbuf_set_subsample( CC3_NEAREST, arg_list[0]+1, arg_list[1]);
	    break; 
	case TRACK_COLOR:
	    if(n!=0 && n!=6) { error=1; break; } else print_ACK();
	    if(n==6)
	    {
	    t_pkt.lower_bound.channel[0]=arg_list[0];
	    t_pkt.upper_bound.channel[0]=arg_list[1];
	    t_pkt.lower_bound.channel[1]=arg_list[2];
	    t_pkt.upper_bound.channel[1]=arg_list[3];
	    t_pkt.lower_bound.channel[2]=arg_list[4];
	    t_pkt.upper_bound.channel[2]=arg_list[5];
	    }
	    cmucam2_track_color(&t_pkt, poll_mode, line_mode  );
	    break;

	case GET_MEAN:
 		if(n!=0) { error=1; break; } else print_ACK();
	    	cmucam2_get_mean(&s_pkt, poll_mode, line_mode  );
	    break;
	case SET_SERVO:
	    if(n!=2) { error=1; break; } else print_ACK();
	    cc3_servo_set(arg_list[0], arg_list[1] ); 
	    break;
	default:
	    print_ACK();
	    break;
    }
    
    }
    else error=1;
    
    if(error==1) print_NCK(); 

    }


    return 0;
}

void cmucam2_get_mean(cc3_color_info_pkt_t *s_pkt, uint8_t poll_mode, uint8_t line_mode  )
{
cc3_image_t img;
uint16_t i;
	    img.channels=3;
	    img.width=cc3_g_current_frame.width;
	    img.height=1;  // image will hold just 1 row for scanline processing
	    img.pix = malloc(3 * img.width);
	    do 
	    {
	     	cc3_pixbuf_load();
     		if(cc3_color_info_scanline_start(s_pkt)!=0 )
		{
			for(i=0; i<cc3_g_current_frame.height; i++ )
			{
			cc3_pixbuf_read_rows(img.pix, img.width, 1);	
			cc3_color_info_scanline(&img, s_pkt);
			}
			cc3_color_info_scanline_finish(s_pkt);
			cmucam2_write_s_packet(s_pkt);
		}
		if(!cc3_uart_has_data(0) ) break;
	    } while(poll_mode!=1);
 	    
	    free(img.pix); 
}

void cmucam2_track_color(cc3_track_pkt_t *t_pkt, uint8_t poll_mode, uint8_t line_mode  )
{
cc3_image_t img;
uint16_t i;
	    img.channels=3;
	    img.width=cc3_g_current_frame.width;
	    img.height=1;  // image will hold just 1 row for scanline processing
	    img.pix = malloc(3 * img.width);
	    do 
	    {
	     	cc3_pixbuf_load();
     		if(cc3_track_color_scanline_start(t_pkt)!=0 )
		{
		uint8_t lm_width,lm_height;
		uint8_t *lm;
		lm_width=0;
		lm_height=0;
		if(line_mode==1)
		{
				lm=&t_pkt->binary_scanline;
				lm_width=cc3_g_current_frame.width/8;
				if(cc3_g_current_frame.width%8!=0 ) lm_width++;
				putchar(0xAA);
				if(cc3_g_current_frame.height>255)
					lm_height=255;
				else
					lm_height=cc3_g_current_frame.height;
				
				//putchar(lm_width);
				putchar(cc3_g_current_frame.width);
				putchar(lm_height);

		}	
			for(i=0; i<cc3_g_current_frame.height; i++ )
			{
			cc3_pixbuf_read_rows(img.pix, img.width, 1);	
			cc3_track_color_scanline(&img, t_pkt);
			if(line_mode==1)
				{
				
				for(int j=0; j<lm_width; j++ )
					{
				//	printf( "%d ",lm[j] );
					if(lm[j]==0xAA ) putchar(0xAB);
					else
						putchar( lm[j] );
					}
				}
			}
			cc3_track_color_scanline_finish(t_pkt);
	    	if(line_mode==1)
		{
			putchar(0xAA);
			putchar(0xAA);
		}	
			
			cmucam2_write_t_packet(t_pkt);
		}
		if(!cc3_uart_has_data(0) ) break;
	    } while(poll_mode!=1);
 	    
	    free(img.pix); 
}


void cmucam2_write_t_packet(cc3_track_pkt_t *pkt)
{
if(pkt->centroid_x>255) pkt->centroid_x=255;
if(pkt->centroid_y>255) pkt->centroid_y=255;
if(pkt->x0>255) pkt->x0=255;
if(pkt->x1>255) pkt->x1=255;
if(pkt->y1>255) pkt->y1=255;
if(pkt->y0>255) pkt->y0=255;
if(pkt->num_pixels>255) pkt->num_pixels=255;
if(pkt->int_density>255) pkt->int_density=255;

if( pkt->num_pixels==0 ) printf( "T 0 0 0 0 0 0 0 0\r" );
else
printf( "T %d %d %d %d %d %d %d %d\r", pkt->centroid_x, pkt->centroid_y,
		pkt->x0, pkt->y0, pkt->x1, pkt->y1, pkt->num_pixels, pkt->int_density );

}

void cmucam2_write_s_packet(cc3_color_info_pkt_t *pkt)
{
printf( "S %d %d %d %d %d %d\r", pkt->mean.channel[0], pkt->mean.channel[1],
		pkt->mean.channel[2], pkt->deviation.channel[0], pkt->deviation.channel[1], 
		pkt->deviation.channel[2]);

}

void print_ACK()
{
printf( "ACK\r" );
}

void print_NCK()
{
printf( "NCK\r" );
}


void set_cmucam2_commands(void)
{
cmucam2_cmds[RETURN]="**"; 
cmucam2_cmds[RESET]="RS"; 
cmucam2_cmds[TRACK_COLOR]="TC"; 
cmucam2_cmds[SEND_FRAME]="SF";  
cmucam2_cmds[FRAME_DIFF]="FD";  
cmucam2_cmds[GET_VERSION]="GV";
cmucam2_cmds[CAMERA_REG]="CR";  
cmucam2_cmds[POLL_MODE]="PM"; 
cmucam2_cmds[GET_MEAN]="GM"; 
cmucam2_cmds[SET_SERVO]="SV"; 
cmucam2_cmds[VIRTUAL_WINDOW]="VW"; 
cmucam2_cmds[DOWN_SAMPLE]="DS"; 
cmucam2_cmds[LINE_MODE]="LM"; 
}

//int32_t cmucam2_get_command(cmucam2_command_t *cmd, int32_t *arg_list)
int32_t cmucam2_get_command(int32_t *cmd, int32_t *arg_list)
{
char line_buf[MAX_LINE];
char c;
char *token;
int32_t fail,length,argc;
uint32_t i;

fail=0;
length=0;
*cmd=0;
c=0;
while(c!='\r' &&  c!='\n' )
{
c=fgetc(stdin);
if(length<(MAX_LINE-1)) 
	{
	line_buf[length]=c;
	length++;
	}
else fail=1;
}
// wait until a return and then fail
if(fail==1) return -1;
line_buf[length]='\0';

if(line_buf[0]=='\r' || line_buf[0]=='\n' )
	{
	*cmd=RETURN;
	return 0;
	}

token = strtok( line_buf, " \r\n" );

if(token==NULL ) return -1;
for(i=0; i<strlen(token); i++ )
	token[i]=toupper(token[i]);
fail=1;
for(i=0; i<CMUCAM2_CMD_END; i++ )
	{
	if(strcmp(token, cmucam2_cmds[i])==0 ) 
		{
		fail=0;
		*cmd=i;
		break;
		}
	
	}
if(fail==1) return -1;
argc=0;
while (1)
        {
                /* extract string from string sequence */
                token = strtok(NULL, " \r\n");
                /* check if there is nothing else to extract */
                if (token==NULL )
                {
                       // printf("Tokenizing complete\n");
                        return argc; 
                }
		for(i=0; i<strlen(token); i++ )
		{
		if(!isdigit(token[i])) return -1;
		}
		arg_list[argc]=atoi(token);
                argc++; 
        }

return -1;


}


