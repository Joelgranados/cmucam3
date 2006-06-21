#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "LPC2k_ee.h"

void eeprom_test(void);


const struct ee_data eep_ini = {
	(unsigned char)	0xAA,		// id           - 1 byte
	(unsigned int)	1,			// record count - 4 bytes
	(unsigned int)	2,			// counter		- 4 bytes
	(unsigned char)	0xFF		// checksum		- 1 byte
};


/* simple eeprom test
 * Shows how to use FLASH as virtual eeprom
 *
 */
int main (void)
{
    uint32_t start_time,val;
    char c;
    FILE *fp;
    cc3_image_t img;

    // setup system    
    cc3_system_setup ();

    // configure uarts
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
    // Make it so that stdout and stdin are not buffered
    val=setvbuf(stdout, NULL, _IONBF, 0 );
    val=setvbuf(stdin, NULL, _IONBF, 0 );
  
    printf( "calling eeprom_test\n" ); 
    eeprom_test();
    printf( "done.\n" );
    while(1);


    cc3_camera_init ();
   
    cc3_set_colorspace(CC3_RGB);
    cc3_set_resolution(CC3_LOW_RES);
    cc3_set_auto_white_balance(true);
    cc3_set_auto_exposure(true);
   
     
    printf("Hello World...\n");
    
    cc3_clr_led (0);
    cc3_clr_led (1);
    cc3_clr_led (2);
   
    // sample wait command in ms 
    cc3_wait_ms(1000);
    cc3_set_led (0);


    // sample showing how to write to the MMC card
    printf( "Type y to test MMC card, type n if you do not have the card\n" );
    c=getchar();
    if(c=='y' || c=='Y' )
    { 
    	printf("\nMMC test...\n");
    	fp = fopen("c:/test.txt", "w");
    	fprintf( fp, "This will be written to the MMC...\n" ); 
    	fclose(fp);
    	printf( "A string was written to test.txt on the mmc card.\n" );
    }

    // sample showing how to read button
    printf("push button on camera back to continue\n");
    start_time=cc3_timer();
    while(!cc3_read_button());
    cc3_set_led(1);
    // sample showing how to use timer
    printf( "It took you %dms to press the button\n",cc3_timer()-start_time );
   


    // setup an image structure
    img.channels=3;
    img.width=cc3_g_current_frame.width;
    img.height=1;  // image will hold just 1 row for scanline processing
    img.pix = cc3_malloc_rows(1);
  
    printf( "Now we will use image data...\n" ); 
    val=0;
    /* 
     * Track the brightest red spot on the image
     */ 
    while(1)
    {
    int y;
    uint16_t my_x, my_y;
    uint8_t max_red;
    cc3_pixel_t my_pix;
    max_red=0;
    my_x=0;
    my_y=0;
    
    if(val&0x1) cc3_set_led(0); else cc3_clr_led(0); 
    if(val&0x2) cc3_set_led(1); else cc3_clr_led(1); 
    if(val&0x3) cc3_set_led(2); else cc3_clr_led(2); 
    if(val&0x4) cc3_set_led(3); else cc3_clr_led(3);
    val++; 

    // This tells the camera to grab a new frame into the fifo and reset
    // any internal location information.
    cc3_pixbuf_load();

    y = 0;
    while(cc3_pixbuf_read_rows(img.pix, 1)) {
		// FIXME: add cc3_pixbug img read rows
		// read a row into the image picture memory from the camera 
		for(uint16_t x=0; x<img.width; x++ )
		{
		// get a pixel from the img row memory
		cc3_get_pixel( &img, x, 0, &my_pix );
		if(my_pix.channel[CC3_RED]>max_red)
			{
			max_red=my_pix.channel[CC3_RED];
			my_x=x;
			my_y=y;
			}
		}	
	
		y++;
	}

    printf( "Found max red value %d at %d, %d\n",max_red,my_x,my_y );
    // sample non-blocking serial routine
    if(!cc3_uart_has_data(0) ) break; 
    }
    free(img.pix);  // don't forget to free!
    printf( "You pressed %c to escape\n",fgetc(stdin) );

    // stdio actually works... 
    printf( "Type in a number followed by return to test scanf: " );
    scanf( "%d", &val );
    printf( "You typed %d\n",val );
    
    printf( "Good work, now try something on your own...\n" );
    while(1);

  return 0;
}



void eeprom_test(void){
int i;
volatile unsigned int status, records;
volatile unsigned char tmp_id1, tmp_id2;
volatile unsigned int  tmp_rec_count1, tmp_rec_count2, tmp_counter1, tmp_counter2;

struct ee_data ee_record;
struct ee_data *ee_pnt;

unsigned int command_ee, response_ee[2];

/*
//erase EEPROM
//initial EEPROM erase
	ee_erase(command_ee,response_ee);
	status = response_ee[0];					//reading of status is not mandatory
	printf( "erase: status = %d \n",status);

//copy initial data into Flash
//an address of eep_ini data is passed in command_ee parameter
	command_ee=(unsigned int) (&eep_ini);		
	ee_write(command_ee,response_ee);
	status = response_ee[0];
	printf( "copy: status = %d \n",status);

//write 10 records into EEPROM
//ee_record is used as a temporary holder of data that are programmed into EEPROM
//in every call, an address of ee_record structure is passed in command_ee parameter
	command_ee=(unsigned int) (&ee_record);
	for(i=0;i<10;i++){
		ee_record._id 		 = EE_REC_ID;		//user's code MUST provide valid record ID!
		ee_record._rec_count = (unsigned int)i;
		ee_record._counter 	 = i; //(unsigned int)(i*512);
		ee_write(command_ee,response_ee);
		status = response_ee[0];
		printf( "write: status = %d \n",status);
		
	}

//read the last record from EEPROM
//response_ee[1] contains the Flash address where the last EEPROM record is actually storred
	ee_read(command_ee,response_ee);
	status = response_ee[0];
	ee_pnt = (struct ee_data *) response_ee[1];
	tmp_id1			= (*ee_pnt)._id;
	tmp_rec_count1	= (*ee_pnt)._rec_count;
	tmp_counter1	= (*ee_pnt)._counter;

	printf( "last record: status = %d counter = %d\n",status,tmp_counter1);


//count records in EEPROM
//response[1] returns the total number of records of ee_data type in EEPROM
	ee_count(command_ee,response_ee);
	status = response_ee[0];
	records = response_ee[1];
	printf( "count: status = %d records = %d\n",status,records );
*/
//read the 5th record in EEPROM
//response_ee[1] contains the Flash address where the 5th EEPROM record is actually storred
	command_ee=5;
	ee_readn(command_ee,response_ee);
	status = response_ee[0];
	ee_pnt = (struct ee_data *) response_ee[1];
	tmp_id2			= (*ee_pnt)._id;
	tmp_rec_count2	= (*ee_pnt)._rec_count;
	tmp_counter2	= (*ee_pnt)._counter;

	printf( "record: status = %d counter = %d\n",status,tmp_counter2);
//erase EEPROM
//EEPROM is erased and ready for use
/*	ee_erase(command_ee,response_ee);
	status = response_ee[0];
	printf( "erase: status = %d\n",status);
*/
}

