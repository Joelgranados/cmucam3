#include <cc3.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "../../hal/lpc2103-retkodfuncam/LPC2100.h"
#include "../../hal/lpc2103-retkodfuncam/cc3_pin_defines.h"

//#define SERIAL_BAUD_RATE  CC3_UART_RATE_230400
#define SERIAL_BAUD_RATE  CC3_UART_RATE_115200
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_57600
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_38400
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_19200
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_9600
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_4800
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_2400
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_1200
//#define SERIAL_BAUD_RATE  CC3_UART_RATE_300


#define DEFAULT_COLOR 0
#define HSV_COLOR     1

#define MAX_ARGS 10
#define MAX_LINE 128

#define VERSION_BANNER "CMUcam v1.00 c6"

#define I2C_WRITE_BIT (0x80)
#define I2C_I2EN      (0x40)
#define I2C_STA	      (0x20)
#define I2C_STO	      (0x10)
#define I2C_SI	      (0x08)
#define I2C_AA	      (0x04)


typedef enum {
  RESET,
  DUMP_FRAME,
  GET_VERSION,
  CAMERA_REG,
  CAMERA_POWER,
  LED_0,
  GET_BUTTON,

  RETURN,                       // Must be second to last
  CMUCAM1_CMDS_COUNT            // Must be last entry so array sizes are correct
} cmucam1_command_t;

static const char cmucam1_cmds[CMUCAM1_CMDS_COUNT][3] = {
  [RETURN] = "",

  /* Camera Module Commands */
  [CAMERA_REG] = "CR",
  [CAMERA_POWER] = "CP",
  //  CT camera type

  /* Image Windowing Commands */
  [DUMP_FRAME] = "DF",

  /* Auxiliary I/O Commands */
  [GET_BUTTON] = "GB",
  [LED_0] = "L0",
  //  L1 LED control

  [RESET] = "RS",
  [GET_VERSION] = "GV",
};


static int32_t cmucam1_get_command (cmucam1_command_t * cmd,
                                    uint32_t arg_list[]);
static int32_t cmucam1_get_command_raw (cmucam1_command_t * cmd,
                                        uint32_t arg_list[]);

int i2c_test_write_polling(uint8_t addr, uint8_t *data, int len);
void print_num(uint32_t x);
static void print_ACK (void);
static void print_NCK (void);
static void print_prompt (void);
static void print_cr (void);
static void cmucam1_send_image_direct (bool auto_led);

static void raw_print (uint8_t val);

static bool packet_filter_flag;
static uint8_t s_pkt_mask;

static bool raw_mode_output;
static bool raw_mode_no_confirmations;
static bool raw_mode_input;

static cmucam1_command_t command;
static uint32_t arg_list[MAX_ARGS];
static bool error, poll_mode, auto_led, buf_mode,frame_stream_mode;
static int8_t line_mode;

static char line_buf[MAX_LINE];

#define TEST_MSG "test ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"

int main (void)
{
  int32_t val, n;
	uint8_t red,green,blue,d0,d1;
	int32_t row,col,row_max,col_max,j;

      	cc3_uart_init (0,
                 SERIAL_BAUD_RATE,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_BINARY);

  cc3_timer_wait_ms(1000);

  cc3_uart0_write("going to do camera_init\r\n");

  if (!cc3_camera_init ()) {
    cc3_led_set_state (0, true);
    exit (1);
  }


  uint8_t data[] = {0x02, 0x40};
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x0d;
  data[1] = 0x01;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x0b;
  data[1] = 0x00;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x6d;
  data[1] = 0xa1;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x58;
  data[1] = 0x10;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x05;
  data[1] = 0x80;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x1a;
  data[1] = 0xff;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x1b;
  data[1] = 0xb3;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x0e;
  data[1] = 0x14;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x11;
  data[1] = 0x6a;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x14;
  data[1] = 0x33;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x1f;
  data[1] = 0x0b;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x1e;
  data[1] = 0xe7;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  data[0] = 0x04;
  data[1] = 0x18;
  n=i2c_test_write_polling(0x3d, data, sizeof data);

  //while(1);

cmucam1_start:
  auto_led = true;
  poll_mode = false;
  frame_stream_mode = false;
  line_mode = 0;
  buf_mode = false;
  packet_filter_flag = false;
  s_pkt_mask = 0xFF;

  raw_mode_output = false;
  raw_mode_no_confirmations = false;
  raw_mode_input = false;



  cc3_camera_set_power_state (true);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);

  cc3_uart0_write(VERSION_BANNER "\r");

  //cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 1);

  while (true) {
    cc3_channel_t old_coi;

    print_prompt ();
    error = false;

    if (raw_mode_input) {
      n = cmucam1_get_command_raw (&command, arg_list);
    }
    else {
      n = cmucam1_get_command (&command, arg_list);
    }

    //cc3_uart0_write("got command\r\n");

    if (n != -1) {
      switch (command) {

      case RESET:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        print_cr ();
        goto cmucam1_start;
        break;

      case GET_VERSION:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        // no different in raw mode
        cc3_uart0_write(VERSION_BANNER);
        break;

      case LED_0:
        if (n != 1 || arg_list[0] > 2) {
          error = true;
          break;
        }

        print_ACK ();
        auto_led = false;
        if (arg_list[0] == 0)
          cc3_led_set_state (0, false);
        if (arg_list[0] == 1)
          cc3_led_set_state (0, true);
        if (arg_list[0] == 2)
          auto_led = true;
        break;

      case DUMP_FRAME:

        print_ACK ();
        print_cr ();
	row_max=200;
	col_max=200;
        cc3_uart0_putchar (1);
  	while (REG (GPIO_IOPIN) & _CC3_CAM_VBLK);   
	while (!(REG (GPIO_IOPIN) & _CC3_CAM_VBLK)); 
	for(row=0; row<row_max; row++ )
	{
        cc3_uart0_putchar (2);
	// wait until vblk goes low to high

		for(col=0; col<col_max; col++ )
		{
		// wait until blk goes high
		while ((REG (GPIO_IOPIN) & _CC3_CAM_HBLK));
		while (!(REG (GPIO_IOPIN) & _CC3_CAM_HBLK));
		// traverse row*3 pix into row 
		 for(j=0; j<row+1; j++ )
			{
			while (!(REG (GPIO_IOPIN) & _CC3_CAM_DCLK)); 
  			while (REG (GPIO_IOPIN) & _CC3_CAM_DCLK);
		     	d0=REG(GPIO_IOPIN)>>24; 	
			while (!(REG (GPIO_IOPIN) & _CC3_CAM_DCLK)); 
  			while (REG (GPIO_IOPIN) & _CC3_CAM_DCLK);
		     	d1=REG(GPIO_IOPIN)>>24; 	
			}
		
		// grab d0,d1
		//d0=0xff;
		//d1=0xff;
		blue=d0&0x1f<<3; 
		green=((d0&0xe0)>>5) | (d1<<5); 
		red=d1&0xe0;
		if(red<16) red=16;
		if(green<16) green=16;
		if(blue<16) blue=16;
        	cc3_uart0_putchar (red);
        	cc3_uart0_putchar (green);
        	cc3_uart0_putchar (blue);
		}
	}
        cc3_uart0_putchar (3);
        /*old_coi = cc3_g_pixbuf_frame.coi;
        if (n == 1) {
          if (arg_list[0] > 4) {
            error = true;
            break;
          }
          cc3_pixbuf_frame_set_coi (arg_list[0]);
        }
        else if (n > 1) {
          error = true;
          break;
        }

        print_ACK ();
        do {
		cmucam1_send_image_direct (auto_led);
		// Check to see if data is on the UART to break from frame_stream_mode
   		if (!cc3_uart_has_data (0)) {
      		if (cc3_uart0_getchar () == '\r')
        		break;
    		}
  	} while (frame_stream_mode);
	
        cc3_pixbuf_frame_set_coi (old_coi);
	*/
	
        break;

      case CAMERA_REG:
        if (n % 2 != 0 || n < 2) {
          error = true;
          break;
        }

        print_ACK ();
        for (int i = 0; i < n; i += 2)
          cc3_camera_set_raw_register (arg_list[i], arg_list[i + 1]);
        break;

      case CAMERA_POWER:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        {
          // save
          uint16_t x_0 = cc3_g_pixbuf_frame.x0;
          uint16_t y_0 = cc3_g_pixbuf_frame.y0;
          uint16_t x_1 = cc3_g_pixbuf_frame.x1;
          uint16_t y_1 = cc3_g_pixbuf_frame.y1;
          uint8_t x_step = cc3_g_pixbuf_frame.x_step;
          uint8_t y_step = cc3_g_pixbuf_frame.y_step;
          cc3_subsample_mode_t ss_mode = cc3_g_pixbuf_frame.subsample_mode;

          cc3_camera_set_power_state (arg_list[0]);

          // restore
          cc3_pixbuf_frame_set_roi (x_0, y_0, x_1, y_1);
          cc3_pixbuf_frame_set_subsample (ss_mode, x_step, y_step);
        }
        break;


      case GET_BUTTON:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();

        {
          int button = cc3_button_get_and_reset_trigger ()? 1 : 0;
          if (raw_mode_output) {
            cc3_uart0_putchar (255);
            raw_print (button);
          }
          else {
	    cc3_uart0_write(button ? "1" : "0");
          }
        }
        break;


      default:
        print_ACK ();
        break;
      }
    }
    else
      error = true;

    if (error)
      print_NCK ();
  }


  return 0;
}

int i2c_test_write_polling(uint8_t addr, uint8_t *data, int len)
{
  int i = 0; // first byte is sent by state 0x18, remaining is state 0x28
  uint8_t state,last_state,done,blink;

  cc3_uart0_write("Testing i2c\r\n");

  REG (GPIO_IODIR) = _CC3_DEFAULT_PORT_DIR;

  REG(I2C_I2CONCLR)=I2C_I2EN | I2C_STA | I2C_SI | I2C_AA;   // 0x6c;  // clear all flags
  REG(I2C_I2CONSET)=I2C_I2EN;  // enable I2C 
  REG (I2C_I2SCLH) = 100;
  REG (I2C_I2SCLL) = 60;

  //REG (GPIO_IOSET) = _CC3_CAM_RESET;
  //  cc3_timer_wait_ms(1000);

  last_state=REG(I2C_I2STAT);
  cc3_uart0_write("starting state:");
  print_num(last_state);
  cc3_uart0_write("\r\n");
  //  REG(I2C_I2ADR)= addr | I2C_WRITE_BIT;  // set slave address and write bit

  REG(I2C_I2CONSET)=I2C_I2EN | I2C_STA;  // Send Start Bit 
  done=0;
  blink=0;
  // I2C state machine
  while(!done) {
     // Poll for status change, this can be inside an interrupt later
     do {
  	  state=REG(I2C_I2STAT);
          cc3_led_set_state (0, blink); blink=!blink;
     } while(!(REG(I2C_I2CONSET) & I2C_SI));

      switch(state)
      {
	case 0x00:
  		cc3_uart0_write("zero state\r\n");
		break;
	case 0x08:
		REG(I2C_I2DAT)=addr << 1;  // set slave address and write bit
		REG(I2C_I2CONCLR)=I2C_STO | I2C_SI;
  		cc3_uart0_write("0x08 state\r\n");
		break;
	case 0x18:
		// Ack received from slave for slave address
		// set the data
		cc3_uart0_write_hex(data[i]);
		REG(I2C_I2DAT)=data[i++];
		REG(I2C_I2CONCLR)=I2C_STA | I2C_STO | I2C_SI;
  		cc3_uart0_write("0x18 state\r\n");
		break;
	case 0x28:
		// Ack received from slave for byte transmitted from master.
		if (i < len) {
			// continue sending
			cc3_uart0_write_hex(data[i]);
			REG(I2C_I2DAT)=data[i++];
			REG(I2C_I2CONCLR)=I2C_STA | I2C_STO | I2C_SI;
			cc3_uart0_write("0x28 state\r\n");
		} else {
			// Stop condition is transmitted in this state signaling the end of transmission
			REG(I2C_I2CONSET)=I2C_STO;  // Transmit stop condition
			REG(I2C_I2CONCLR)=I2C_SI;  // clear SI	
			done=1;
			cc3_uart0_write("0x28 done state\r\n");
		}
		break;
	case 0xF8:
  		cc3_uart0_write("No relevant state data state\r\n");
		break;
	default:
  		cc3_uart0_write("unknown state:");
		cc3_uart0_write_hex(state);
  		cc3_uart0_write("\r\n");
		break;
      }
	last_state=state;

  } 

  cc3_led_set_state (0, false); 
  cc3_uart0_write("done...\r\n");
 



return 1;  // return ack bit eventually

}

void cmucam1_send_image_direct (bool auto_led)
{
  uint32_t x, y;
  uint32_t size_x, size_y;
  uint8_t *row = cc3_malloc_rows (1);
  uint8_t num_channels = cc3_g_pixbuf_frame.coi == CC3_CHANNEL_ALL ? 3 : 1;

  

  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;

  cc3_uart0_putchar (1);
  cc3_uart0_putchar (size_x);
  if (size_y > 255)
    size_y = 255;
  cc3_uart0_putchar (size_y);
  for (y = 0; y < size_y; y++) {
    cc3_uart0_putchar (2);
    if (auto_led) {
      if (y % 4 == 0)
        cc3_led_set_state (0, true);
      else
        cc3_led_set_state (0, false);
    }
    cc3_pixbuf_read_rows (row, 1);

    for (x = 0; x < size_x * num_channels; x++) {
      uint8_t p = row[x];

      // avoid confusion from FIFO corruptions
      if (p < 16) {
        p = 16;
      }
      else if (p > 240) {
        p = 240;
      }
      cc3_uart0_putchar (p);
    }
  }
  cc3_uart0_putchar (3);

  cc3_led_set_state (0, false);
  free (row);
}




void print_ACK ()
{
  if (!raw_mode_no_confirmations)
    cc3_uart0_write("ACK\r");
}

void print_NCK ()
{
  if (!raw_mode_no_confirmations)
    cc3_uart0_write("NCK\r");
}

void print_prompt ()
{
  cc3_uart0_write(":");
}

void print_cr ()
{
  if (!raw_mode_output) {
    cc3_uart0_write("\r");
  }
}

int32_t cmucam1_get_command (cmucam1_command_t * cmd, uint32_t * arg_list)
{
  int c;
  char *token;
  bool fail = false;
  uint32_t length, argc;
  uint32_t i;

  length = 0;
  c = 0;
  while (c != '\r') {
    c = cc3_uart0_getchar ();

    if (length < (MAX_LINE - 1) && c != EOF) {
      line_buf[length] = c;
      length++;
    }
    else {
      // too long or EOF
      return -1;
    }
  }
  // null terminate
  line_buf[length] = '\0';

  // check for empty command
  if (line_buf[0] == '\r' || line_buf[0] == '\n') {
    *cmd = RETURN;
    return 0;
  }

  // start looking for command
  token = strtok (line_buf, " \r\n");

  if (token == NULL) {
    // no command ?
    return -1;
  }

  // get name of the command
  for (i = 0; i < strlen (token); i++) {
    token[i] = toupper (token[i]);
  }

  // do lookup of command
  fail = true;
  for (i = 0; i < CMUCAM1_CMDS_COUNT; i++) {
    if (strcmp (token, cmucam1_cmds[i]) == 0) {
      fail = false;
      *cmd = i;
      break;
    }
  }
  if (fail) {
    return -1;
  }

  // now get the arguments
  argc = 0;
  while (true) {
    // extract string from string sequence
    token = strtok (NULL, " \r\n");
    // check if there is nothing else to extract
    if (token == NULL) {
      // printf("Tokenizing complete\n");
      return argc;
    }

    // make sure the argument is fully numeric
    for (i = 0; i < strlen (token); i++) {
      if (!isdigit (token[i]))
        return -1;
    }

    // we have a valid token, add it
    arg_list[argc] = atoi (token);
    argc++;
  }

  return -1;
}

int32_t cmucam1_get_command_raw (cmucam1_command_t * cmd, uint32_t * arg_list)
{
  bool fail;
  int c;
  unsigned int i;
  uint32_t argc;

  char cmd_str[3];
  cmd_str[2] = '\0';

  // read characters
  for (i = 0; i < 2; i++) {
    c = cc3_uart0_getchar ();
    if (c == EOF) {
      return -1;
    }

    cmd_str[i] = c;
  }

  // do lookup of command
  fail = true;
  for (i = 0; i < CMUCAM1_CMDS_COUNT; i++) {
    if (strcmp (cmd_str, cmucam1_cmds[i]) == 0) {
      fail = false;
      *cmd = i;
      break;
    }
  }
  if (fail) {
    return -1;
  }

  // read argc
  c = cc3_uart0_getchar ();
  if (c == EOF) {
    return -1;
  }
  argc = c;
  if (argc > MAX_ARGS) {
    return -1;
  }

  // read args
  for (i = 0; i < argc; i++) {
    c = cc3_uart0_getchar ();
    if (c == EOF) {
      return -1;
    }

    arg_list[i] = toupper (c);
  }

  // done
  return argc;
}

void print_num(uint32_t x)
{       
	uint8_t t,set;
	uint32_t div;

	set=0;
	for(div=10000; div>=10; div/=10 )
	{
	  if((x>=div) | (set==1)) { t=x/div; cc3_uart0_putchar('0'+t); set=1; }
		x=x%div;
	}

	x=x%10;
	cc3_uart0_putchar('0'+x);

}

void raw_print (uint8_t val)
{
  if (val == 255) {
    cc3_uart0_putchar (254);              // avoid confusion
  }
  else {
    cc3_uart0_putchar (val);
  }
}
