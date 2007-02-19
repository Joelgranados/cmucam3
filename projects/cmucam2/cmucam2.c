#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_color_track.h>
#include <cc3_color_info.h>
#include <cc3_histogram.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <cc3_jpg.h>
#include <cc3_math.h>

// Uncomment line below to reverse servo direction for auto-servo and demo mode 
//#define SERVO_REVERSE_DIRECTION

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

#define SERVO_MIN 0
#define SERVO_MID 128
#define SERVO_MAX 255
// Define a jitter guard such that more than SERVO_GUARD pixels are required
// for the servo to move.

static const int MAX_ARGS = 10;
static const int MAX_LINE = 128;

static const char *VERSION_BANNER = "CMUcam2 v1.00 c6";

typedef enum {
  RETURN,
  RESET,
  TRACK_COLOR,
  SEND_FRAME,
  HI_RES,
  FRAME_DIFF,
  GET_VERSION,
  GET_MEAN,
  SET_SERVO,
  CAMERA_REG,
  CAMERA_POWER,
  POLL_MODE,
  LINE_MODE,
  SEND_JPEG,
  VIRTUAL_WINDOW,
  DOWN_SAMPLE,
  GET_HISTOGRAM,
  TRACK_WINDOW,
  GET_TRACK,
  GET_WINDOW,
  LED_0,
  NOISE_FILTER,
  TRACK_INVERT,
  SERVO_MASK,
  SERVO_PARAMETERS,
  SERVO_OUTPUT,
  GET_SERVO,
  SET_INPUT,
  GET_INPUT,
  SET_TRACK,
  BUF_MODE,
  READ_FRAME,
  OUTPUT_MASK,
  PACKET_FILTER,
  CONF_HISTOGRAM,
  CMUCAM2_CMD_END               // Must be last entry so array sizes are correct
} cmucam2_command_t;

typedef struct {
uint8_t pan_step,tilt_step;
uint8_t pan_range_near,tilt_range_near;
uint8_t pan_range_far,tilt_range_far;
int16_t x;
int16_t y;
bool y_control;
bool x_control;
bool y_report;
bool x_report;
} cmucam2_servo_t;


char *cmucam2_cmds[CMUCAM2_CMD_END];

static void set_cmucam2_commands (void)
{
  cmucam2_cmds[RETURN] = "**";

  /* Buffer Commands */
  cmucam2_cmds[BUF_MODE] = "BM";
  cmucam2_cmds[READ_FRAME] = "RF";

  /* Camera Module Commands */
  cmucam2_cmds[CAMERA_REG] = "CR";
  cmucam2_cmds[CAMERA_POWER] = "CP";
  //  CT camera type

  /* Data Rate Commands */
  //  DM delay mode
  cmucam2_cmds[POLL_MODE] = "PM";
  //  PS packet skip
  //  RM raw mode
  cmucam2_cmds[PACKET_FILTER] = "PF";
  cmucam2_cmds[OUTPUT_MASK] = "OM";

  /* Servo Commands */
  cmucam2_cmds[SET_SERVO] = "SV";
  //  SP servo parameters
  cmucam2_cmds[GET_SERVO] = "GS";
  cmucam2_cmds[SERVO_OUTPUT] = "SO";
  cmucam2_cmds[SERVO_MASK] = "SM";
  // SP servo parameters
  cmucam2_cmds[SERVO_PARAMETERS] = "SP";

  /* Image Windowing Commands */
  cmucam2_cmds[SEND_FRAME] = "SF";
  cmucam2_cmds[DOWN_SAMPLE] = "DS";
  cmucam2_cmds[VIRTUAL_WINDOW] = "VW";
  //  FS frame stream
  cmucam2_cmds[HI_RES] = "HR";
  cmucam2_cmds[GET_WINDOW] = "GW";
  //  PD pixel difference

  /* Auxiliary I/O Commands */
  cmucam2_cmds[GET_INPUT] = "GI";
  cmucam2_cmds[SET_INPUT] = "SI";  // new for cmucam3
  //  GB get button
  cmucam2_cmds[LED_0] = "L0";
  //  L1 LED control

  /* Color Tracking Commands */
  cmucam2_cmds[TRACK_COLOR] = "TC";
  cmucam2_cmds[TRACK_INVERT] = "TI";
  cmucam2_cmds[TRACK_WINDOW] = "TW";
  cmucam2_cmds[NOISE_FILTER] = "NF";
  cmucam2_cmds[LINE_MODE] = "LM";
  cmucam2_cmds[GET_TRACK] = "GT";
  cmucam2_cmds[SET_TRACK] = "ST";

  /* Histogram Commands */
  cmucam2_cmds[GET_HISTOGRAM] = "GH";
  cmucam2_cmds[CONF_HISTOGRAM] = "HC";
  //  HC histogram config
  //  HT histogram track

  /* Frame Differencing Commands */
  cmucam2_cmds[FRAME_DIFF] = "FD";
  //  DC difference channel
  //  LF load frame
  //  MD mask difference
  //  UD upload difference
  //  HD hires difference

  /* Color Statistics Commands */
  cmucam2_cmds[GET_MEAN] = "GM";

  /* System Level Commands */
  //  SD sleep deeply
  //  SL sleep
  cmucam2_cmds[RESET] = "RS";
  cmucam2_cmds[GET_VERSION] = "GV";

  /* CMUcam3 New Commands */
  cmucam2_cmds[SEND_JPEG] = "SJ";
}


static void cmucam2_get_histogram (cc3_histogram_pkt_t * h_pkt,
                                   bool poll_mode, bool buf_mode, bool quiet);
static void cmucam2_get_mean (cc3_color_info_pkt_t * t_pkt, bool poll_mode,
                              bool line_mode, bool buf_mode, bool quiet);
static void cmucam2_write_s_packet (cc3_color_info_pkt_t * pkt);
static void cmucam2_track_color (cc3_track_pkt_t * t_pkt,
				 bool poll_mode,
				 bool line_mode, bool auto_led, cmucam2_servo_t *servo_settings,bool buf_mode, bool quiet);
static int32_t cmucam2_get_command (int32_t * cmd, int32_t * arg_list);
static void print_ACK (void);
static void print_NCK (void);
static void cmucam2_write_t_packet (cc3_track_pkt_t * pkt, cmucam2_servo_t *servo_settings);
static void cmucam2_write_h_packet (cc3_histogram_pkt_t * pkt);
static void cmucam2_send_image_direct (bool auto_led);

bool packet_filter_flag;
uint8_t t_pkt_mask;
uint8_t s_pkt_mask;

int main (void)
{
  int32_t command;
  int32_t val, n;
  uint32_t arg_list[MAX_ARGS], start_time;
  bool error, poll_mode, line_mode, auto_led, demo_mode,buf_mode;
  cc3_track_pkt_t t_pkt;
  cc3_color_info_pkt_t s_pkt;
  cc3_histogram_pkt_t h_pkt;
  cmucam2_servo_t servo_settings;

  set_cmucam2_commands ();

  cc3_filesystem_init ();

  cc3_uart_init (0,
                 SERIAL_BAUD_RATE,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_BINARY);
  val = setvbuf (stdout, NULL, _IONBF, 0);

  if (!cc3_camera_init ()) {
    cc3_led_set_on (0);
    exit (1);
  }

  servo_settings.x_control=false;
  servo_settings.y_control=false;
  servo_settings.x_report=false;
  servo_settings.y_report=false;
  demo_mode = false;
 
  start_time = cc3_timer_get_current_ms ();
  
  do {
    if (cc3_button_get_state () == 1) {
      // Demo Mode flag
      demo_mode = true;
      servo_settings.x_control=true;
      servo_settings.y_control=true;
      servo_settings.x_report=true;
      servo_settings.y_report=true;
      // Debounce Switch
      cc3_led_set_off (0);
      cc3_timer_wait_ms (500);
      break;
    }

  } while (cc3_timer_get_current_ms () < (start_time + 2000));


cmucam2_start:
  auto_led = true;
  poll_mode = false;
  line_mode = false;
  buf_mode = false;
  packet_filter_flag = false;
  t_pkt_mask= 0xFF;
  s_pkt_mask= 0xFF;
  h_pkt.bins = 28;
  t_pkt.track_invert = false;
  t_pkt.noise_filter = 0;
  t_pkt.lower_bound.channel[0] = 16;
  t_pkt.upper_bound.channel[0] = 240;
  t_pkt.lower_bound.channel[1] = 16;
  t_pkt.upper_bound.channel[1] = 240;
  t_pkt.lower_bound.channel[2] = 16;
  t_pkt.upper_bound.channel[2] = 240;
 
  
  servo_settings.x=SERVO_MID;
  servo_settings.y=SERVO_MID;
  servo_settings.pan_range_far=16;
  servo_settings.pan_range_near=8;
  servo_settings.pan_step=10;
  servo_settings.tilt_range_far=30;
  servo_settings.tilt_range_near=15;
  servo_settings.tilt_step=10;


  cc3_camera_set_power_state (true);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);

  cc3_pixbuf_load ();

  printf ("%s\r", VERSION_BANNER);

  cc3_gpio_set_mode (0, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (2, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (3, CC3_GPIO_MODE_SERVO);
  
  cc3_gpio_set_servo_position (0, SERVO_MID);
  cc3_gpio_set_servo_position (1, SERVO_MID);
  cc3_gpio_set_servo_position (2, SERVO_MID);
  cc3_gpio_set_servo_position (3, SERVO_MID);

  cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 1);

  if (demo_mode) {
    // Wait for second button press as target lock
    while (1) {
      cc3_led_set_on (0);
      cc3_timer_wait_ms (100);
      cc3_led_set_off (0);
      cc3_timer_wait_ms (100);
      if (cc3_button_get_state () == 1)
        break;
    }

  }
  while (true) {
    cc3_channel_t old_coi;

    printf (":");
    error = false;
    if (demo_mode == true) {
      n = 0;
      command = TRACK_WINDOW;
    }
    else
      n = cmucam2_get_command (&command, arg_list);
    if (n != -1) {
      switch (command) {

      case RESET:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        printf ("\r");
        goto cmucam2_start;
        break;

     case READ_FRAME:
        if (n != 0) {
          error = true;
          break;
        }
        print_ACK ();
        cc3_pixbuf_load ();
        break;
   
     case OUTPUT_MASK:
        if (n != 2 || arg_list[0]>1) {
          error = true;
          break;
        }
	if(arg_list[0]==0) t_pkt_mask=arg_list[1];
	if(arg_list[0]==1) s_pkt_mask=arg_list[1];
        print_ACK ();
        break;


      case GET_VERSION:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        printf ("%s\r", VERSION_BANNER);
        break;

      case LED_0:
        if (n != 1 || arg_list[0] > 2) {
          error = true;
          break;
        }

        print_ACK ();
        auto_led = false;
        if (arg_list[0] == 0)
          cc3_led_set_off (0);
        if (arg_list[0] == 1)
          cc3_led_set_on (0);
        if (arg_list[0] == 2)
          auto_led = true;
        break;
      
      case BUF_MODE:
        if (n != 1 || arg_list[0]>1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 1)
          buf_mode = true;
        else
          buf_mode = false;
        break;

      case PACKET_FILTER:
        if (n != 1 || arg_list[0]>1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 1)
          packet_filter_flag= true;
        else
          packet_filter_flag= false;
        break;

      case POLL_MODE:
        if (n != 1 || arg_list[0]>1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 1)
          poll_mode = true;
        else
          poll_mode = false;
        break;

	case SERVO_PARAMETERS:
        if (n != 6) {
          error = true;
          break;
        }
        else
        print_ACK ();
	servo_settings.pan_range_far=arg_list[0];
	servo_settings.pan_range_near=arg_list[1];
	servo_settings.pan_step=arg_list[2];
	servo_settings.tilt_range_far=arg_list[3];
	servo_settings.tilt_range_near=arg_list[4];
	servo_settings.tilt_step=arg_list[5];
        break;
	
 	case SERVO_MASK:
        if (n != 1) {
          error = true;
          break;
        }
        else
        print_ACK ();
	servo_settings.x_control=!!(arg_list[0]&0x1);
	servo_settings.y_control=!!(arg_list[0]&0x2);
	servo_settings.x_report=!!(arg_list[0]&0x4);
	servo_settings.y_report=!!(arg_list[0]&0x8);
        break;

	case HI_RES:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 1)
          cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_HIGH);
        else
          cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);

        // re-init fifo
        cc3_pixbuf_load ();
        cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 1);
        break;


      case CONF_HISTOGRAM:
        if (n != 1 || arg_list<1 ) {
          error = true;
          break;
        }
  	h_pkt.bins = arg_list[0];
        print_ACK ();
        
	break;	

	
      case TRACK_INVERT:
        if (n != 1 || arg_list[0] > 1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 0)
          t_pkt.track_invert = 0;
        else
          t_pkt.track_invert = 1;
        break;

      case NOISE_FILTER:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        t_pkt.noise_filter = arg_list[0];
        break;

      case LINE_MODE:
        if (n != 2) {
          error = true;
          break;
        }

        print_ACK ();
        // FIXME: Make bitmasks later
        if (arg_list[0] == 0) {
          if (arg_list[1] == 1)
            line_mode = true;
          else
            line_mode = false;
        }
        break;


      case SEND_JPEG:
        if (n != 0 && n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        //init_jpeg();
        // cc3_set_resolution(CC3_RES_HIGH);
        //cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, 1, 1);

        cc3_jpeg_send_simple ();
        printf ("JPG_END\r");
        break;


      case SEND_FRAME:
        old_coi = cc3_g_pixbuf_frame.coi;
        if (n == 1) {
          if (arg_list[0] > 4) {
            error = true;
            break;
          }
          cc3_pixbuf_set_coi (arg_list[0]);
        }
        else if (n > 1) {
          error = true;
          break;
        }

        print_ACK ();
        cmucam2_send_image_direct (auto_led);
        cc3_pixbuf_set_coi (old_coi);
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
        cc3_camera_set_power_state(arg_list[0]);
        break;

      case VIRTUAL_WINDOW:
        if (n != 4) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_pixbuf_set_roi (arg_list[0] * 2,
                            arg_list[1], arg_list[2] * 2, arg_list[3]);
        break;

      case GET_TRACK:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        printf ("%d %d %d %d %d %d\r", t_pkt.lower_bound.channel[0],
                t_pkt.lower_bound.channel[1], t_pkt.lower_bound.channel[2],
                t_pkt.upper_bound.channel[0], t_pkt.upper_bound.channel[1],
                t_pkt.upper_bound.channel[2]);
        break;

      case GET_WINDOW:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        printf ("%d %d %d %d\r", cc3_g_pixbuf_frame.x0 / 2,
                cc3_g_pixbuf_frame.y0, cc3_g_pixbuf_frame.x1 / 2,
                cc3_g_pixbuf_frame.y1);
        break;

      case DOWN_SAMPLE:
        if (n != 2) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, arg_list[0] * 2,
                                  arg_list[1]);
        break;

     case SET_TRACK:
        if (n != 0 && n != 6) {
          error = true;
          break;
        }
        print_ACK ();
        if (n == 6) {
          t_pkt.lower_bound.channel[0] = arg_list[0];
          t_pkt.upper_bound.channel[0] = arg_list[1];
          t_pkt.lower_bound.channel[1] = arg_list[2];
          t_pkt.upper_bound.channel[1] = arg_list[3];
          t_pkt.lower_bound.channel[2] = arg_list[4];
          t_pkt.upper_bound.channel[2] = arg_list[5];
        }
        break;

      case TRACK_COLOR:
        if (n != 0 && n != 6) {
          error = true;
          break;
        }

        print_ACK ();
        if (n == 6) {
          t_pkt.lower_bound.channel[0] = arg_list[0];
          t_pkt.upper_bound.channel[0] = arg_list[1];
          t_pkt.lower_bound.channel[1] = arg_list[2];
          t_pkt.upper_bound.channel[1] = arg_list[3];
          t_pkt.lower_bound.channel[2] = arg_list[4];
          t_pkt.upper_bound.channel[2] = arg_list[5];
        }
        cmucam2_track_color (&t_pkt, poll_mode, line_mode,auto_led,&servo_settings,buf_mode, 0);
        break;

      case TRACK_WINDOW:
        if (n != 0 && n != 1) {
          error = true;
          break;
        }
        else {
          uint32_t threshold, x0, y0, x1, y1;
          int32_t tmp;
          threshold = 30;
          if (n == 1)
            threshold = arg_list[0];
          print_ACK ();
          // set window to 1/2 size
          x0 = cc3_g_pixbuf_frame.x0 + cc3_g_pixbuf_frame.width / 4;
          x1 = cc3_g_pixbuf_frame.x1 - cc3_g_pixbuf_frame.width / 4;
          y0 = cc3_g_pixbuf_frame.y0 + cc3_g_pixbuf_frame.width / 4;
          y1 = cc3_g_pixbuf_frame.y1 - cc3_g_pixbuf_frame.width / 4;
          cc3_pixbuf_set_roi (x0, y0, x1, y1);
          // call get mean
          cmucam2_get_mean (&s_pkt, 1, line_mode,buf_mode, 1);
          // set window back to full size
          x0 = 0;
          x1 = cc3_g_pixbuf_frame.raw_width;
          y0 = 0;
          y1 = cc3_g_pixbuf_frame.raw_height;
          cc3_pixbuf_set_roi (x0, y0, x1, y1);
          // fill in parameters and call track color
          tmp = s_pkt.mean.channel[0] - threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.lower_bound.channel[0] = tmp;
          tmp = s_pkt.mean.channel[0] + threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.upper_bound.channel[0] = tmp;
          tmp = s_pkt.mean.channel[1] - threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.lower_bound.channel[1] = tmp;
          tmp = s_pkt.mean.channel[1] + threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.upper_bound.channel[1] = tmp;
          tmp = s_pkt.mean.channel[2] - threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.lower_bound.channel[2] = tmp;
          tmp = s_pkt.mean.channel[2] + threshold;
          if (tmp < 16)
            tmp = 16;
          if (tmp > 240)
            tmp = 240;
          t_pkt.upper_bound.channel[2] = tmp;
          cmucam2_track_color (&t_pkt, poll_mode, line_mode, auto_led,&servo_settings,buf_mode, 0);
        }
        demo_mode = false;
        break;


      case GET_MEAN:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        cmucam2_get_mean (&s_pkt, poll_mode, line_mode,buf_mode, 0);
        break;


      case GET_HISTOGRAM:
        if (n != 1 || arg_list[0] > 2) {
          error = true;
          break;
        }

        print_ACK ();
        h_pkt.channel = arg_list[0];
        cmucam2_get_histogram (&h_pkt, poll_mode, buf_mode, 0);
        break;


      case SET_SERVO:
        if (n != 2 || arg_list[0]>4) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_gpio_set_mode (arg_list[0], CC3_GPIO_MODE_SERVO);
        cc3_gpio_set_servo_position (arg_list[0], arg_list[1]);
	if(arg_list[0]==0) servo_settings.x=arg_list[1];
	if(arg_list[0]==1) servo_settings.y=arg_list[1];
        break;

      case GET_SERVO:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        printf("%d\r", cc3_gpio_get_servo_position(arg_list[0]));
        break;
	
      case GET_INPUT:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        printf("%d\r",  cc3_gpio_get_value(arg_list[0])|
			(cc3_gpio_get_value(arg_list[1])<<1)|
			(cc3_gpio_get_value(arg_list[2])<<2)|
			(cc3_gpio_get_value(arg_list[3])<<3)  );
        break;


      case SET_INPUT:
        if (n != 1) {
          error = true;
          break;
        }
        print_ACK ();
        cc3_gpio_set_mode (arg_list[0], CC3_GPIO_MODE_INPUT);

        break;

      case SERVO_OUTPUT:
        if (n != 2) {
          error = true;
          break;
        }
        print_ACK ();
        cc3_gpio_set_mode (arg_list[0], CC3_GPIO_MODE_OUTPUT);
        cc3_gpio_set_value (arg_list[0], arg_list[1]);

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


void cmucam2_send_image_direct (bool auto_led)
{
  cc3_pixbuf_load ();

  uint32_t x, y;
  uint32_t size_x, size_y;
  uint8_t *row = cc3_malloc_rows (1);
  uint8_t num_channels = cc3_g_pixbuf_frame.coi == CC3_CHANNEL_ALL ? 3 : 1;


  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;

  putchar (1);
  putchar (size_x);
  if (size_y > 255)
    size_y = 255;
  putchar (size_y);
  for (y = 0; y < size_y; y++) {
    putchar (2);
    if (auto_led) {
      if (y % 4 == 0)
        cc3_led_set_on (0);
      else
        cc3_led_set_off (0);
    }
    cc3_pixbuf_read_rows (row, 1);
    for (x = 0; x < size_x * num_channels; x++) {
      uint8_t p = row[x];
      putchar (p);
    }
  }
  putchar (3);

  cc3_led_set_off (0);
  free (row);
}



void cmucam2_get_histogram (cc3_histogram_pkt_t * h_pkt, bool poll_mode, bool buf_mode,
                            bool quiet)
{
  cc3_image_t img;
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (3 * img.width);
  h_pkt->hist = malloc (h_pkt->bins * sizeof (uint32_t));
  if(img.pix==NULL || h_pkt->hist==NULL )
	{
	printf( "INTERNAL ERROR\r" );
	return;
	}
  do {
    if(!buf_mode) cc3_pixbuf_load ();
    else cc3_pixbuf_rewind();
    if (cc3_histogram_scanline_start (h_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
        cc3_histogram_scanline (&img, h_pkt);
      }
      cc3_histogram_scanline_finish (h_pkt);
      while (!cc3_uart_has_data (0)) {
        if (fgetc (stdin) == '\r')
	{
          free (img.pix);
          free (h_pkt->hist);
          return;
	}
      }
      if (!quiet)
        cmucam2_write_h_packet (h_pkt);
    }
    if (!cc3_uart_has_data (0)) {
      if (fgetc (stdin) == '\r')
        break;
    }
  } while (!poll_mode);

  free (img.pix);
  free (h_pkt->hist);


}


void cmucam2_get_mean (cc3_color_info_pkt_t * s_pkt,
                       bool poll_mode, bool line_mode,bool buf_mode, bool quiet)
{
  cc3_image_t img;
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (3 * img.width);
  do {
    if(!buf_mode) cc3_pixbuf_load ();
    else cc3_pixbuf_rewind();
    if (cc3_color_info_scanline_start (s_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
        cc3_color_info_scanline (&img, s_pkt);
      }
      cc3_color_info_scanline_finish (s_pkt);
      while (!cc3_uart_has_data (0)) {
        if (fgetc (stdin) == '\r')
          free (img.pix);
        return;
      }
      if (!quiet)
        cmucam2_write_s_packet (s_pkt);
    }
    if (!cc3_uart_has_data (0)) {
      if (fgetc (stdin) == '\r')
        break;
    }
  } while (!poll_mode);

  free (img.pix);
}

void cmucam2_track_color (cc3_track_pkt_t * t_pkt,
			  bool poll_mode,
                          bool line_mode, bool auto_led,cmucam2_servo_t *servo_settings,bool buf_mode, bool quiet)
{
  cc3_image_t img;
  uint16_t i,x_mid,y_mid;

  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = cc3_malloc_rows (1);
  if (img.pix == NULL) {
    return;
  }

  x_mid=cc3_g_pixbuf_frame.x0 + (cc3_g_pixbuf_frame.width/2);
  y_mid=cc3_g_pixbuf_frame.y0 + (cc3_g_pixbuf_frame.height/2);

  
  do {
    if(!buf_mode) cc3_pixbuf_load ();
    else cc3_pixbuf_rewind();
    if (cc3_track_color_scanline_start (t_pkt) != 0) {
      uint8_t lm_width, lm_height;
      uint8_t *lm;
      lm_width = 0;
      lm_height = 0;
      if (line_mode) {
        lm = &t_pkt->binary_scanline;
        lm_width = img.width / 8;
        if (img.width % 8 != 0)
          lm_width++;
        if (!quiet)
          putchar (0xAA);
        if (cc3_g_pixbuf_frame.height > 255)
          lm_height = 255;
        else
          lm_height = cc3_g_pixbuf_frame.height;
        if (!quiet)
          putchar (img.width);
        if (!quiet)
          putchar (lm_height);
      }
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
        cc3_track_color_scanline (&img, t_pkt);
        if (line_mode) {
          // keep this check here if you don't want the CMUcam2 GUI to hang after exiting a command in line mode
          while (!cc3_uart_has_data (0)) {
            if (fgetc (stdin) == '\r')
              free (img.pix);
            return;
          }
          for (int j = 0; j < lm_width; j++) {
            if (lm[j] == 0xAA) {
              if (!quiet)
                putchar (0xAB);
            }
            else {
              if (!quiet)
                putchar (lm[j]);
            }
          }
        }
      }
      // keep this check here if you don't want the CMUcam2 GUI to hang after exiting a command in line mode
      while (!cc3_uart_has_data (0)) {
        if (fgetc (stdin) == '\r')
          free (img.pix);
        return;
      }
      cc3_track_color_scanline_finish (t_pkt);
      if (line_mode) {
        if (!quiet)
          putchar (0xAA);
        if (!quiet)
          putchar (0xAA);
      }
      if (auto_led) {
        if (t_pkt->int_density > 2)
          cc3_led_set_on (0);
        else
          cc3_led_set_off (0);
      }

      if( t_pkt->int_density>5) {
       if(servo_settings->x_control ) 
	      	{
		int8_t t_step;
		t_step=0;
		if(t_pkt->centroid_x>x_mid+servo_settings->pan_range_far) t_step=servo_settings->pan_step;
		else if(t_pkt->centroid_x>x_mid+servo_settings->pan_range_near) t_step=(servo_settings->pan_step/2);

		if(t_pkt->centroid_x<x_mid-servo_settings->pan_range_far) t_step=-servo_settings->pan_step;
		else if(t_pkt->centroid_x<x_mid-servo_settings->pan_range_near) t_step=-(servo_settings->pan_step/2);

		
		#ifdef SERVO_REVERSE_DIRECTION
			servo_settings->x-=t_step;	
		#else
			servo_settings->x+=t_step;	
		#endif
		
		if(servo_settings->x>SERVO_MAX) servo_settings->x=SERVO_MAX;
		if(servo_settings->x<SERVO_MIN) servo_settings->x=SERVO_MIN;
		cc3_gpio_set_servo_position (0, servo_settings->x);
		}
	if( servo_settings->y_control )
	{
	if(t_pkt->centroid_y>y_mid+servo_settings->tilt_range_far) servo_settings->y+=servo_settings->tilt_step;
		else if(t_pkt->centroid_y>y_mid+servo_settings->tilt_range_near) servo_settings->y+=servo_settings->tilt_step/2;

		if(t_pkt->centroid_y<y_mid-servo_settings->tilt_range_far) servo_settings->y-=servo_settings->tilt_step;
		else if(t_pkt->centroid_y<y_mid-servo_settings->tilt_range_near) servo_settings->y-=servo_settings->tilt_step/2;

		if(servo_settings->y>SERVO_MAX) servo_settings->y=SERVO_MAX;
		if(servo_settings->y<SERVO_MIN) servo_settings->y=SERVO_MIN;
		cc3_gpio_set_servo_position (1, servo_settings->y);
	}	
      }
      
      if(!quiet) cmucam2_write_t_packet (t_pkt,servo_settings);

    } 
    else return 0;
    while (!cc3_uart_has_data (0))
    {
      if(fgetc(stdin)=='\r' )
      	break;
    }
  } while (!poll_mode);
  free (img.pix);
  return 1;
}


void cmucam2_write_t_packet (cc3_track_pkt_t * pkt, cmucam2_servo_t *servo_settings)
{
static bool empty_cnt=0;

  if (pkt->centroid_x > 255)
    pkt->centroid_x = 255;
  if (pkt->centroid_y > 255)
    pkt->centroid_y = 255;
  if (pkt->x0 > 255)
    pkt->x0 = 255;
  if (pkt->x1 > 255)
    pkt->x1 = 255;
  if (pkt->y1 > 255)
    pkt->y1 = 255;
  if (pkt->y0 > 255)
    pkt->y0 = 255;
  if (pkt->num_pixels > 255)
    pkt->num_pixels = 255;
  if (pkt->int_density > 255)
    pkt->int_density = 255;

  if (pkt->num_pixels == 0)
  {
    if(packet_filter_flag==0)
     {
	printf( "T" );
        if((t_pkt_mask & 0x01) !=0) printf( " 0");
        if((t_pkt_mask & 0x02) !=0) printf( " 0");
        if((t_pkt_mask & 0x04) !=0) printf( " 0");
        if((t_pkt_mask & 0x08) !=0) printf( " 0");
        if((t_pkt_mask & 0x10) !=0) printf( " 0");
        if((t_pkt_mask & 0x20) !=0) printf( " 0");
        if((t_pkt_mask & 0x40) !=0) printf( " 0");
        if((t_pkt_mask & 0x80) !=0) printf( " 0");
     }
    if(packet_filter_flag==1 && empty_cnt==0)
    {
	printf( "T" );
        if((t_pkt_mask & 0x01) !=0) printf( " 0");
        if((t_pkt_mask & 0x02) !=0) printf( " 0");
        if((t_pkt_mask & 0x04) !=0) printf( " 0");
        if((t_pkt_mask & 0x08) !=0) printf( " 0");
        if((t_pkt_mask & 0x10) !=0) printf( " 0");
        if((t_pkt_mask & 0x20) !=0) printf( " 0");
        if((t_pkt_mask & 0x40) !=0) printf( " 0");
        if((t_pkt_mask & 0x80) !=0) printf( " 0");
    }
  }
  else
  {
    empty_cnt=0;
    printf( "T" );
    if((t_pkt_mask & 0x01) !=0) printf( " %d",pkt->centroid_x );
    if((t_pkt_mask & 0x02) !=0) printf( " %d",pkt->centroid_y );
    if((t_pkt_mask & 0x04) !=0) printf( " %d",pkt->x0);
    if((t_pkt_mask & 0x08) !=0) printf( " %d",pkt->y0);
    if((t_pkt_mask & 0x10) !=0) printf( " %d",pkt->x1);
    if((t_pkt_mask & 0x20) !=0) printf( " %d",pkt->y1);
    if((t_pkt_mask & 0x40) !=0) printf( " %d",pkt->num_pixels);
    if((t_pkt_mask & 0x80) !=0) printf( " %d",pkt->int_density);
    //printf ("T %d %d %d %d %d %d %d %d", pkt->centroid_x, pkt->centroid_y,
      //      pkt->x0, pkt->y0, pkt->x1, pkt->y1, pkt->num_pixels,
        //    pkt->int_density);
  }
  if(servo_settings->x_report) printf( " %d",servo_settings->x );
  if(servo_settings->y_report) printf( " %d",servo_settings->y );
  if(packet_filter_flag==0) printf( "\r" );
  if(packet_filter_flag==1)
    {
	if(empty_cnt==0) printf ("\r");
  	if (pkt->num_pixels == 0) empty_cnt=1;
    }
}

void cmucam2_write_h_packet (cc3_histogram_pkt_t * pkt)
{
  uint32_t i;
  uint32_t total_pix;

  total_pix = cc3_g_pixbuf_frame.width * cc3_g_pixbuf_frame.height;
  printf ("H");
  for (i = 0; i < pkt->bins; i++) {
    pkt->hist[i] = (pkt->hist[i] * 256) / total_pix;
    if (pkt->hist[i] > 255)
      pkt->hist[i] = 255;
    printf (" %d", pkt->hist[i]);
  }
  printf ("\r");
}

void cmucam2_write_s_packet (cc3_color_info_pkt_t * pkt)
{

  printf( "S" );
  if((s_pkt_mask & 0x01) !=0) printf( " %d",pkt->mean.channel[0]);
  if((s_pkt_mask & 0x02) !=0) printf( " %d",pkt->mean.channel[1]);
  if((s_pkt_mask & 0x04) !=0) printf( " %d",pkt->mean.channel[2]);
  if((s_pkt_mask & 0x08) !=0) printf( " %d",pkt->deviation.channel[0]);
  if((s_pkt_mask & 0x10) !=0) printf( " %d",pkt->deviation.channel[1]);
  if((s_pkt_mask & 0x20) !=0) printf( " %d",pkt->deviation.channel[2]);
  printf( "\r" );

}

void print_ACK ()
{
  printf ("ACK\r");
}

void print_NCK ()
{
  printf ("NCK\r");
}



//int32_t cmucam2_get_command(cmucam2_command_t *cmd, int32_t *arg_list)
int32_t cmucam2_get_command (int32_t * cmd, int32_t * arg_list)
{
  char line_buf[MAX_LINE];
  char c;
  char *token;
  int32_t fail, length, argc;
  uint32_t i;

  fail = 0;
  length = 0;
  *cmd = 0;
  c = 0;
  while (c != '\r') {
    c = fgetc (stdin);
    if (length < (MAX_LINE - 1)) {
      line_buf[length] = c;
      length++;
    }
    else
      fail = 1;
  }
  // wait until a return and then fail
  if (fail == 1)
    return -1;
  line_buf[length] = '\0';

  if (line_buf[0] == '\r' || line_buf[0] == '\n') {
    *cmd = RETURN;
    return 0;
  }

  token = strtok (line_buf, " \r\n");

  if (token == NULL)
    return -1;
  for (i = 0; i < strlen (token); i++)
    token[i] = toupper (token[i]);
  fail = 1;
  for (i = 0; i < CMUCAM2_CMD_END; i++) {
    if (strcmp (token, cmucam2_cmds[i]) == 0) {
      fail = 0;
      *cmd = i;
      break;
    }

  }
  if (fail == 1)
    return -1;
  argc = 0;
  while (true) {
    /* extract string from string sequence */
    token = strtok (NULL, " \r\n");
    /* check if there is nothing else to extract */
    if (token == NULL) {
      // printf("Tokenizing complete\n");
      return argc;
    }
    for (i = 0; i < strlen (token); i++) {
      if (!isdigit (token[i]))
        return -1;
    }
    arg_list[argc] = atoi (token);
    argc++;
  }

  return -1;
}
