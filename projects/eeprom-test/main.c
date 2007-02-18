#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "LPC2k_ee.h"

// Leave this commented for the first execution of the code to write a record
// Remove later to read the record from eeprom
#define ERASE_AND_WRITE
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

    // configure uarts
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_TEXT);
    // Make it so that stdout and stdin are not buffered
    val=setvbuf(stdout, NULL, _IONBF, 0 );
    //val=setvbuf(stdin, NULL, _IONBF, 0 );
  


    cc3_camera_init ();
   
    cc3_camera_set_colorspace(CC3_COLORSPACE_RGB);
    cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);
    cc3_camera_set_auto_white_balance(true);
    cc3_camera_set_auto_exposure(true);
   
     
    printf("Hello World...\n");
    
    printf( "calling eeprom_test\n" ); 
    eeprom_test();
    printf( "done.\n" );
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

#ifdef ERASE_AND_WRITE
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
#endif
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

