#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_color_track.h>
#include <cc3_color_info.h>
#include <cc3_histogram.h>
#include <cc3_frame_diff.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <cc3_jpg.h>
#include <cc3_math.h>
#include <cc3_hsv.h>

// Uncomment line below to reverse servo direction for auto-servo and demo mode 
#define SERVO_REVERSE_DIRECTION_PAN
#define SERVO_REVERSE_DIRECTION_TILT

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

#define DEFAULT_COLOR 0
#define HSV_COLOR     1

static const unsigned int MAX_ARGS = 10;
static const unsigned int MAX_LINE = 128;

static const char VERSION_BANNER[] = "CMUcam2 v1.00 c6";

typedef struct {
  uint8_t pan_step, tilt_step;
  uint8_t pan_range_near, tilt_range_near;
  uint8_t pan_range_far, tilt_range_far;
  int16_t x;
  int16_t y;
  bool y_control;
  bool x_control;
  bool y_report;
  bool x_report;
} cmucam2_servo_t;


typedef enum {
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
  GET_BUTTON,
  FRAME_DIFF_CHANNEL,
  LOAD_FRAME,
  RAW_MODE,
  COLOR_SPACE,
  HIRES_DIFF,

  RETURN,                       // Must be second to last
  CMUCAM2_CMDS_COUNT            // Must be last entry so array sizes are correct
} cmucam2_command_t;

static const char cmucam2_cmds[CMUCAM2_CMDS_COUNT][3] = {
  [RETURN] = "",

  /* Buffer Commands */
  [BUF_MODE] = "BM",
  [READ_FRAME] = "RF",

  /* Camera Module Commands */
  [CAMERA_REG] = "CR",
  [CAMERA_POWER] = "CP",
  //  CT camera type

  /* Data Rate Commands */
  //  DM delay mode
  [POLL_MODE] = "PM",
  //  PS packet skip
  [RAW_MODE] = "RM",
  [PACKET_FILTER] = "PF",
  [OUTPUT_MASK] = "OM",

  /* Servo Commands */
  [SET_SERVO] = "SV",
  [GET_SERVO] = "GS",
  [SERVO_OUTPUT] = "SO",
  [SERVO_MASK] = "SM",
  [SERVO_PARAMETERS] = "SP",

  /* Image Windowing Commands */
  [SEND_FRAME] = "SF",
  [DOWN_SAMPLE] = "DS",
  [VIRTUAL_WINDOW] = "VW",
  //  FS frame stream
  [HI_RES] = "HR",
  [GET_WINDOW] = "GW",
  //  PD pixel difference

  /* Auxiliary I/O Commands */
  [GET_INPUT] = "GI",
  [SET_INPUT] = "SI",           // new for cmucam3
  [GET_BUTTON] = "GB",
  [LED_0] = "L0",
  //  L1 LED control

  /* Color Tracking Commands */
  [TRACK_COLOR] = "TC",
  [TRACK_INVERT] = "TI",
  [TRACK_WINDOW] = "TW",
  [NOISE_FILTER] = "NF",
  [LINE_MODE] = "LM",
  [GET_TRACK] = "GT",
  [SET_TRACK] = "ST",

  /* Histogram Commands */
  [GET_HISTOGRAM] = "GH",
  [CONF_HISTOGRAM] = "HC",
  //  HT histogram track

  /* Frame Differencing Commands */
  [FRAME_DIFF] = "FD",
  [LOAD_FRAME] = "LF",
  [FRAME_DIFF_CHANNEL] = "DC",
  [HIRES_DIFF] = "HD",
  //  MD mask difference
  //  UD upload difference

  /* Color Statistics Commands */
  [GET_MEAN] = "GM",

  /* System Level Commands */
  //  SD sleep deeply
  //  SL sleep
  [RESET] = "RS",
  [GET_VERSION] = "GV",
  
  [COLOR_SPACE] = "CS",

  /* CMUcam3 New Commands */
  [SEND_JPEG] = "SJ",
};


static void cmucam2_load_frame (cc3_frame_diff_pkt_t * pkt, bool buf_mode, uint8_t sw_color_space);
static void cmucam2_get_histogram (cc3_histogram_pkt_t * h_pkt,
                                   bool poll_mode, bool buf_mode, uint8_t sw_color_space, bool quiet);
static void cmucam2_get_mean (cc3_color_info_pkt_t * t_pkt, bool poll_mode,
                              bool line_mode, bool buf_mode, uint8_t sw_color_space, bool quiet);
static void cmucam2_write_s_packet (cc3_color_info_pkt_t * pkt);
static void cmucam2_track_color (cc3_track_pkt_t * t_pkt,
                                 bool poll_mode,
                                 int8_t line_mode, bool auto_led,
                                 cmucam2_servo_t * servo_settings,
                                 bool buf_mode, uint8_t sw_color_space, bool quiet);
static void cmucam2_frame_diff (cc3_frame_diff_pkt_t * pkt,
                                bool poll_mode, bool line_mode, bool buf_mode,
                                bool auto_led, uint8_t sw_color_space, bool quiet);
static int32_t cmucam2_get_command (cmucam2_command_t * cmd,
                                    uint32_t arg_list[]);
static int32_t cmucam2_get_command_raw (cmucam2_command_t * cmd,
                                        uint32_t arg_list[]);
static void print_ACK (void);
static void print_NCK (void);
static void print_prompt (void);
static void print_cr (void);
static void cmucam2_write_t_packet (cc3_track_pkt_t * pkt,
                                    cmucam2_servo_t * servo_settings);
static void cmucam2_write_h_packet (cc3_histogram_pkt_t * pkt);
static void cmucam2_send_image_direct (bool auto_led,uint8_t sw_color_space);

static void raw_print (uint8_t val);

static bool packet_filter_flag;
static uint8_t t_pkt_mask;
static uint8_t s_pkt_mask;

static bool raw_mode_output;
static bool raw_mode_no_confirmations;
static bool raw_mode_input;

int main (void)
{
  cmucam2_command_t command;
  int32_t val, n;
  uint32_t arg_list[MAX_ARGS];
  uint32_t start_time;
  uint8_t sw_color_space;
  bool error, poll_mode, auto_led, demo_mode, buf_mode;
  int8_t line_mode;
  cc3_track_pkt_t t_pkt;
  cc3_color_info_pkt_t s_pkt;
  cc3_histogram_pkt_t h_pkt;
  cc3_frame_diff_pkt_t fd_pkt;
  cmucam2_servo_t servo_settings;

  //cc3_filesystem_init ();

  cc3_uart_init (0,
                 SERIAL_BAUD_RATE,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_BINARY);
  val = setvbuf (stdout, NULL, _IONBF, 0);

  if (!cc3_camera_init ()) {
    cc3_led_set_state (0, true);
    exit (1);
  }

  servo_settings.x_control = false;
  servo_settings.y_control = false;
  servo_settings.x_report = false;
  servo_settings.y_report = false;
  demo_mode = false;

  // Keep this memory in the bank for frame differencing
  fd_pkt.previous_template = malloc (16 * 16 * sizeof (uint32_t));
  if (fd_pkt.previous_template == NULL)
    printf ("Malloc FD startup error!\r");
  start_time = cc3_timer_get_current_ms ();

  do {
    if (cc3_button_get_state () == 1) {
      // Demo Mode flag
      demo_mode = true;
      servo_settings.x_control = true;
      servo_settings.y_control = true;
      servo_settings.x_report = true;
      servo_settings.y_report = true;
      // Debounce Switch
      cc3_led_set_state (0, false);
      cc3_timer_wait_ms (500);
      break;
    }

  } while (cc3_timer_get_current_ms () < (start_time + 2000));


cmucam2_start:
  sw_color_space=DEFAULT_COLOR;
  auto_led = true;
  poll_mode = false;
  line_mode = 0;
  buf_mode = false;
  packet_filter_flag = false;
  t_pkt_mask = 0xFF;
  s_pkt_mask = 0xFF;
  h_pkt.bins = 28;
  fd_pkt.coi = 1;
  fd_pkt.template_width = 8;
  fd_pkt.template_height = 8;
  t_pkt.track_invert = false;
  t_pkt.noise_filter = 0;

  // set to 0 since cmucam2 appears to initialize to this
  t_pkt.lower_bound.channel[0] = 0;
  t_pkt.upper_bound.channel[0] = 0;
  t_pkt.lower_bound.channel[1] = 0;
  t_pkt.upper_bound.channel[1] = 0;
  t_pkt.lower_bound.channel[2] = 0;
  t_pkt.upper_bound.channel[2] = 0;

  raw_mode_output = false;
  raw_mode_no_confirmations = false;
  raw_mode_input = false;

  servo_settings.x = SERVO_MID;
  servo_settings.y = SERVO_MID;
  servo_settings.pan_range_far = 32;
  servo_settings.pan_range_near = 20;
  servo_settings.pan_step = 20;
  servo_settings.tilt_range_far = 32;
  servo_settings.tilt_range_near = 20;
  servo_settings.tilt_step = 20;


  cc3_camera_set_power_state (true);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);

  printf ("%s\r", VERSION_BANNER);

  cc3_gpio_set_mode (0, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (2, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (3, CC3_GPIO_MODE_SERVO);

  cc3_gpio_set_servo_position (0, SERVO_MID);
  cc3_gpio_set_servo_position (1, SERVO_MID);
  cc3_gpio_set_servo_position (2, SERVO_MID);
  cc3_gpio_set_servo_position (3, SERVO_MID);

  cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 1);

  if (demo_mode) {
    cc3_led_set_state (0, true);
    cc3_timer_wait_ms (5000);
    cc3_camera_set_auto_exposure (false);
    cc3_camera_set_auto_white_balance (false);
    // Wait for second button press as target lock
    while (1) {
      cc3_led_set_state (0, true);
      cc3_timer_wait_ms (100);
      cc3_led_set_state (0, false);
      cc3_timer_wait_ms (100);
      if (cc3_button_get_state () == 1)
        break;
    }

  }
  while (true) {
    cc3_channel_t old_coi;

    print_prompt ();
    error = false;
    if (demo_mode == true) {
      n = 0;
      command = TRACK_WINDOW;
    }
    else if (raw_mode_input) {
      n = cmucam2_get_command_raw (&command, arg_list);
    }
    else {
      n = cmucam2_get_command (&command, arg_list);
    }
    if (n != -1) {
      switch (command) {

      case RESET:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        print_cr ();
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
        if (n != 2 || arg_list[0] > 1) {
          error = true;
          break;
        }
        if (arg_list[0] == 0)
          t_pkt_mask = arg_list[1];
        if (arg_list[0] == 1)
          s_pkt_mask = arg_list[1];
        print_ACK ();
        break;


      case GET_VERSION:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        // no different in raw mode
        printf ("%s", VERSION_BANNER);
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

      case BUF_MODE:
        if (n != 1 || arg_list[0] > 1) {
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
        if (n != 1 || arg_list[0] > 1) {
          error = true;
          break;
        }

        print_ACK ();
        if (arg_list[0] == 1)
          packet_filter_flag = true;
        else
          packet_filter_flag = false;
        break;

      case POLL_MODE:
        if (n != 1 || arg_list[0] > 1) {
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

        print_ACK ();
        servo_settings.pan_range_far = arg_list[0];
        servo_settings.pan_range_near = arg_list[1];
        servo_settings.pan_step = arg_list[2];
        servo_settings.tilt_range_far = arg_list[3];
        servo_settings.tilt_range_near = arg_list[4];
        servo_settings.tilt_step = arg_list[5];
        break;

      case SERVO_MASK:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        servo_settings.x_control = !!(arg_list[0] & 0x1);
        servo_settings.y_control = !!(arg_list[0] & 0x2);
        servo_settings.x_report = !!(arg_list[0] & 0x4);
        servo_settings.y_report = !!(arg_list[0] & 0x8);
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
        cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 1);
        break;



      case LOAD_FRAME:
        if (n != 0) {
          error = true;
          break;
        }
        print_ACK ();
        fd_pkt.total_x = cc3_g_pixbuf_frame.width;
        fd_pkt.total_y = cc3_g_pixbuf_frame.height;
        fd_pkt.load_frame = 1;  // load a new frame
        cmucam2_load_frame (&fd_pkt, buf_mode, sw_color_space);
        break;

      case HIRES_DIFF:
        if (n != 1 || arg_list[0] > 1) {
          error = true;
          break;
        }
        print_ACK ();
        if (arg_list[0] == 0) {
          fd_pkt.template_width = 8;
          fd_pkt.template_height = 8;
        }
        else {
          fd_pkt.template_width = 16;
          fd_pkt.template_height = 16;
        }
        break;

      case FRAME_DIFF:
        if (n != 1) {
          error = true;
          break;
        }
        print_ACK ();
        fd_pkt.threshold = arg_list[0];
        fd_pkt.load_frame = 0;
        fd_pkt.total_x = cc3_g_pixbuf_frame.width;
        fd_pkt.total_y = cc3_g_pixbuf_frame.height;
        cmucam2_frame_diff (&fd_pkt, poll_mode, line_mode, buf_mode, auto_led, sw_color_space,
                            0);
        break;

      case FRAME_DIFF_CHANNEL:
        if (n != 1 || arg_list[0] > 2) {
          error = true;
          break;
        }
        print_ACK ();
        fd_pkt.coi = arg_list[0];
        break;


      case CONF_HISTOGRAM:
        if (n != 1 || arg_list[0] < 1) {
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

      case COLOR_SPACE:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        sw_color_space= arg_list[0];
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
            line_mode = 1;
          else if (arg_list[1] == 2)
		  line_mode=2;
	  else
            line_mode = 0;
        }
        break;


      case SEND_JPEG:
        if (n != 0 && n != 1) {
          error = true;
          break;
        }

        print_ACK ();
        // ignore raw mode
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
          cc3_pixbuf_frame_set_coi (arg_list[0]);
        }
        else if (n > 1) {
          error = true;
          break;
        }

        print_ACK ();
        cmucam2_send_image_direct (auto_led,sw_color_space);
        cc3_pixbuf_frame_set_coi (old_coi);
        break;

      case RAW_MODE:
        if (n != 1) {
          error = true;
          break;
        }

        raw_mode_output = arg_list[0] & 1;
        raw_mode_no_confirmations = arg_list[0] & 2;
        raw_mode_input = arg_list[0] & 4;
        print_ACK ();           // last because ACK may be supressed
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

      case VIRTUAL_WINDOW:
        if (n != 4) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_pixbuf_frame_set_roi (arg_list[0] * 2,
                                  arg_list[1], arg_list[2] * 2, arg_list[3]);
        break;

      case GET_TRACK:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        if (raw_mode_output) {
          putchar (255);
          raw_print (t_pkt.lower_bound.channel[0]);
          raw_print (t_pkt.lower_bound.channel[1]);
          raw_print (t_pkt.lower_bound.channel[2]);
          raw_print (t_pkt.upper_bound.channel[0]);
          raw_print (t_pkt.upper_bound.channel[1]);
          raw_print (t_pkt.upper_bound.channel[2]);
        }
        else {
          printf ("%d %d %d %d %d %d\r", t_pkt.lower_bound.channel[0],
                  t_pkt.lower_bound.channel[1], t_pkt.lower_bound.channel[2],
                  t_pkt.upper_bound.channel[0], t_pkt.upper_bound.channel[1],
                  t_pkt.upper_bound.channel[2]);
        }
        break;

      case GET_WINDOW:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        if (raw_mode_output) {
          putchar (255);
          raw_print (cc3_g_pixbuf_frame.x0 / 2);
          raw_print (cc3_g_pixbuf_frame.y0);
          raw_print (cc3_g_pixbuf_frame.x1 / 2);
          raw_print (cc3_g_pixbuf_frame.y1);
        }
        else {
          printf ("%d %d %d %d\r", cc3_g_pixbuf_frame.x0 / 2,
                  cc3_g_pixbuf_frame.y0, cc3_g_pixbuf_frame.x1 / 2,
                  cc3_g_pixbuf_frame.y1);
        }
        break;

      case DOWN_SAMPLE:
        if (n != 2) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST,
                                        arg_list[0] * 2, arg_list[1]);
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
        cmucam2_track_color (&t_pkt, poll_mode, line_mode, auto_led,
                             &servo_settings, buf_mode, sw_color_space, 0);
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
          cc3_pixbuf_frame_set_roi (x0, y0, x1, y1);
          // call get mean
          cmucam2_get_mean (&s_pkt, 1, line_mode, buf_mode,sw_color_space, 1);
          // set window back to full size
          x0 = 0;
          x1 = cc3_g_pixbuf_frame.raw_width;
          y0 = 0;
          y1 = cc3_g_pixbuf_frame.raw_height;
          cc3_pixbuf_frame_set_roi (x0, y0, x1, y1);
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
          cmucam2_track_color (&t_pkt, poll_mode, line_mode, auto_led,
                               &servo_settings, buf_mode,sw_color_space, 0);
        }
        demo_mode = false;
        break;


      case GET_MEAN:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        cmucam2_get_mean (&s_pkt, poll_mode, line_mode, buf_mode,sw_color_space, 0);
        break;


      case GET_HISTOGRAM:
        if (n != 1 || arg_list[0] > 2) {
          error = true;
          break;
        }

        print_ACK ();
        h_pkt.channel = arg_list[0];
        cmucam2_get_histogram (&h_pkt, poll_mode, buf_mode,sw_color_space, 0);
        break;


      case SET_SERVO:
        if (n != 2 || arg_list[0] > 4) {
          error = true;
          break;
        }

        print_ACK ();
        cc3_gpio_set_mode (arg_list[0], CC3_GPIO_MODE_SERVO);
        cc3_gpio_set_servo_position (arg_list[0], arg_list[1]);
        if (arg_list[0] == 0)
          servo_settings.x = arg_list[1];
        if (arg_list[0] == 1)
          servo_settings.y = arg_list[1];
        break;

      case GET_SERVO:
        if (n != 1) {
          error = true;
          break;
        }

        print_ACK ();

        {
          uint8_t servo = cc3_gpio_get_servo_position (arg_list[0]);
          if (raw_mode_output) {
            putchar (255);
            raw_print (servo);
          }
          else {
            printf ("%d\r", servo);
          }
          break;
        }

      case GET_INPUT:
        if (n != 0) {
          error = true;
          break;
        }

        print_ACK ();
        {
          uint8_t input =
            (cc3_gpio_get_value (arg_list[0])) |
            (cc3_gpio_get_value (arg_list[1]) << 1) |
            (cc3_gpio_get_value (arg_list[2]) << 2) |
            (cc3_gpio_get_value (arg_list[3]) << 3);
          if (raw_mode_output) {
            putchar (255);
            raw_print (input);
          }
          else {
            printf ("%d\r", input);
          }
        }
        break;


      case SET_INPUT:
        if (n != 1) {
          error = true;
          break;
        }
        print_ACK ();
        cc3_gpio_set_mode (arg_list[0], CC3_GPIO_MODE_INPUT);

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
            putchar (255);
            raw_print (button);
          }
          else {
            printf ("%d\r", button);
          }
        }
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


void cmucam2_send_image_direct (bool auto_led, uint8_t sw_color_space)
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
        cc3_led_set_state (0, true);
      else
        cc3_led_set_state (0, false);
    }
    cc3_pixbuf_read_rows (row, 1);
    if(sw_color_space==HSV_COLOR && num_channels==CC3_CHANNEL_ALL )  cc3_rgb2hsv_row(row,size_x);
    for (x = 0; x < size_x * num_channels; x++) {
      uint8_t p = row[x];

      // avoid confusion from FIFO corruptions
      if (p < 16) {
        p = 16;
      }
      else if (p > 240) {
        p = 240;
      }
      putchar (p);
    }
  }
  putchar (3);

  cc3_led_set_state (0, false);
  free (row);
}



void cmucam2_get_histogram (cc3_histogram_pkt_t * h_pkt, bool poll_mode,
                            bool buf_mode, uint8_t sw_color_space, bool quiet)
{
  cc3_image_t img;
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (3 * img.width);
  h_pkt->hist = malloc (h_pkt->bins * sizeof (uint32_t));
  if (img.pix == NULL || h_pkt->hist == NULL) {
    printf ("INTERNAL ERROR\r");
    return;
  }
  do {
    if (!buf_mode)
      cc3_pixbuf_load ();
    else
      cc3_pixbuf_rewind ();
    if (cc3_histogram_scanline_start (h_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
        if(sw_color_space==HSV_COLOR  && img.channels==CC3_CHANNEL_ALL)  cc3_rgb2hsv_row(img.pix,img.width);
        cc3_histogram_scanline (&img, h_pkt);
      }
      cc3_histogram_scanline_finish (h_pkt);
      while (!cc3_uart_has_data (0)) {
        if (getchar () == '\r') {
          free (img.pix);
          free (h_pkt->hist);
          return;
        }
      }
      if (!quiet)
        cmucam2_write_h_packet (h_pkt);
    }
    if (!cc3_uart_has_data (0)) {
      if (getchar () == '\r')
        break;
    }
  } while (!poll_mode);

  free (img.pix);
  free (h_pkt->hist);


}

void cmucam2_load_frame (cc3_frame_diff_pkt_t * pkt, bool buf_mode, uint8_t sw_color_space)
{
  cc3_image_t img;
  uint8_t old_coi;

  old_coi = cc3_g_pixbuf_frame.coi;
  cc3_pixbuf_frame_set_coi (pkt->coi);

  img.channels = 1;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (img.width);

  if (!buf_mode)
    cc3_pixbuf_load ();
  else
    cc3_pixbuf_rewind ();

  if (cc3_frame_diff_scanline_start (pkt) != 0) {
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      if(sw_color_space==HSV_COLOR  && img.channels==CC3_CHANNEL_ALL)  cc3_rgb2hsv_row(img.pix,img.width);
      cc3_frame_diff_scanline (&img, pkt);
    }
    cc3_frame_diff_scanline_finish (pkt);
  }
  else
    printf ("frame diff start error\r");

  cc3_pixbuf_frame_set_coi (old_coi);
  free (img.pix);


}

void cmucam2_frame_diff (cc3_frame_diff_pkt_t * pkt,
                         bool poll_mode, bool line_mode, bool buf_mode, 
                         bool auto_led, uint8_t sw_color_space, bool quiet)
{
  cc3_track_pkt_t t_pkt;
  cc3_image_t img;
  uint8_t old_coi;

  bool prev_packet_empty = true;

  old_coi = cc3_g_pixbuf_frame.coi;
  cc3_pixbuf_frame_set_coi (pkt->coi);
  img.channels = 1;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (img.width);
  pkt->current_template =
    malloc (pkt->template_width * pkt->template_height * sizeof (uint32_t));
  if (pkt->current_template == NULL)
    printf ("Malloc failed in frame diff\r");
  do {
    if (!buf_mode)
      cc3_pixbuf_load ();
    else
      cc3_pixbuf_rewind ();

    if (cc3_frame_diff_scanline_start (pkt) != 0) {

      while (cc3_pixbuf_read_rows (img.pix, 1)) {
	if(sw_color_space==HSV_COLOR && img.channels==CC3_CHANNEL_ALL)  cc3_rgb2hsv_row(img.pix,img.width);
        cc3_frame_diff_scanline (&img, pkt);
      }
      cc3_frame_diff_scanline_finish (pkt);

      while (!cc3_uart_has_data (0)) {
        if (getchar () == '\r') {
          cc3_pixbuf_frame_set_coi (old_coi);
          free (pkt->current_template);
          free (img.pix);
          return;
        }
      }
      if (!quiet) {
        t_pkt.x0 = pkt->x0 + 1;
        t_pkt.y0 = pkt->y0 + 1;
        t_pkt.x1 = pkt->x1 + 1;
        t_pkt.y1 = pkt->y1 + 1;
        t_pkt.num_pixels = pkt->num_pixels;
        t_pkt.centroid_x = pkt->centroid_x + 1;
        t_pkt.centroid_y = pkt->centroid_y + 1;
        t_pkt.int_density = pkt->int_density;
        if (auto_led) {
          if (t_pkt.num_pixels > 2)
            cc3_led_set_state (0, true);
          else
            cc3_led_set_state (0, false);
        }

        if (!(packet_filter_flag &&
              t_pkt.num_pixels == 0 && prev_packet_empty)) {
          cmucam2_write_t_packet (&t_pkt, NULL);
        }
        prev_packet_empty = t_pkt.num_pixels == 0;
      }
    }


    if (!cc3_uart_has_data (0)) {
      if (getchar () == '\r')
        break;
    }
  } while (!poll_mode);

  cc3_pixbuf_frame_set_coi (old_coi);
  free (pkt->current_template);
  free (img.pix);
}

void cmucam2_get_mean (cc3_color_info_pkt_t * s_pkt,
                       bool poll_mode, bool line_mode, bool buf_mode, uint8_t sw_color_space,
                       bool quiet)
{
  cc3_image_t img;
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (3 * img.width);
  do {
    if (!buf_mode)
      cc3_pixbuf_load ();
    else
      cc3_pixbuf_rewind ();
    if (cc3_color_info_scanline_start (s_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
	if(sw_color_space==HSV_COLOR && img.channels==CC3_CHANNEL_ALL)  cc3_rgb2hsv_row(img.pix,img.width);
        cc3_color_info_scanline (&img, s_pkt);
      }
      cc3_color_info_scanline_finish (s_pkt);
      while (!cc3_uart_has_data (0)) {
        if (getchar () == '\r') {
          free (img.pix);
          return;
        }
      }
      if (!quiet)
        cmucam2_write_s_packet (s_pkt);
    }
    if (!cc3_uart_has_data (0)) {
      if (getchar () == '\r')
        break;
    }
  } while (!poll_mode);

  free (img.pix);
}

void cmucam2_track_color (cc3_track_pkt_t * t_pkt,
                          bool poll_mode,
                          int8_t line_mode, bool auto_led,
                          cmucam2_servo_t * servo_settings, bool buf_mode, uint8_t sw_color_space,
                          bool quiet)
{
  cc3_image_t img;
  uint16_t x_mid, y_mid;
  int8_t t_step;

  bool prev_packet_empty = true;

  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = cc3_malloc_rows (1);
  if (img.pix == NULL) {
    return;
  }

  x_mid = cc3_g_pixbuf_frame.x0 + (cc3_g_pixbuf_frame.width / 2);
  y_mid = cc3_g_pixbuf_frame.y0 + (cc3_g_pixbuf_frame.height / 2);


  do {
    if (!buf_mode)
      cc3_pixbuf_load ();
    else
      cc3_pixbuf_rewind ();
    if (cc3_track_color_scanline_start (t_pkt) != 0) {
      uint8_t lm_width, lm_height;
      uint8_t *lm;
      lm_width = 0;
      lm_height = 0;
      if (line_mode==1) {
        // FIXME: This doesn't make sense
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

    if (line_mode==2) {
        // FIXME: This still doesn't make sense
        lm = &t_pkt->binary_scanline;
        lm_width = img.width / 8;
        if (img.width % 8 != 0)
          lm_width++;
        if (!quiet)
          putchar (0xFE);
        if (cc3_g_pixbuf_frame.height > 255)
          lm_height = 255;
        else
          lm_height = cc3_g_pixbuf_frame.height;
        //if (!quiet)
          //putchar (img.width);
        if (!quiet)
          putchar (lm_height);
      }

      
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
	if(sw_color_space==HSV_COLOR && img.channels==CC3_CHANNEL_ALL)  cc3_rgb2hsv_row(img.pix,img.width);
        cc3_track_color_scanline (&img, t_pkt);

        if (line_mode==1) {
          // keep this check here if you don't want the CMUcam2 GUI to hang after exiting a command in line mode
          while (!cc3_uart_has_data (0)) {
            if (getchar () == '\r') {
              free (img.pix);
              return;
            }
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


        if (line_mode==2) {
	  uint8_t min,max,p_count,conf;
	  uint32_t mean;
          // keep this check here if you don't want the CMUcam2 GUI to hang after exiting a command in line mode
          while (!cc3_uart_has_data (0)) {
            if (getchar () == '\r') {
              free (img.pix);
              return;
            }
          }
	  mean=0;
	  min=255;
	  max=0;
	  p_count=0;
          for (int j = 0; j < img.width; j++) {
	 	uint8_t block, offset;
        	block = j / 8;
        	offset = j % 8;
        	offset = 7 - offset;	
                if((lm[block] & (1<<offset))!=0) 
			{
			// bit detected
			if(j<min) min=j;
			if(j>max) max=j;
			p_count++;
			mean+=j;
			}
          }
	mean=mean/p_count;
	conf=((max-min)*100)/p_count;
	if (!quiet)
		printf( "%c%c%c%c%c",(uint8_t)mean,min,max,p_count,conf);
        }


	
      }
      // keep this check here if you don't want the CMUcam2 GUI to hang after exiting a command in line mode
      while (!cc3_uart_has_data (0)) {
        if (getchar () == '\r') {
          free (img.pix);
          return;
        }
      }
      cc3_track_color_scanline_finish (t_pkt);
      if (line_mode==1) {
        if (!quiet)
          putchar (0xAA);
        if (!quiet)
          putchar (0xAA);
      }

     if (line_mode==2) {
        if (!quiet)
          putchar (0xFD);
      }

      if (auto_led) {
        if (t_pkt->int_density > 2)
          cc3_led_set_state (0, true);
        else
          cc3_led_set_state (0, false);
      }

      if (t_pkt->int_density > 5 && servo_settings != NULL) {
        if (servo_settings->x_control) {
          t_step = 0;
          if (t_pkt->centroid_x > x_mid + servo_settings->pan_range_far)
            t_step = servo_settings->pan_step;
          else if (t_pkt->centroid_x > x_mid + servo_settings->pan_range_near)
            t_step = (servo_settings->pan_step / 2);

          if (t_pkt->centroid_x < x_mid - servo_settings->pan_range_far)
            t_step = -servo_settings->pan_step;
          else if (t_pkt->centroid_x < x_mid - servo_settings->pan_range_near)
            t_step = -(servo_settings->pan_step / 2);


#ifdef SERVO_REVERSE_DIRECTION_PAN
          servo_settings->x -= t_step;
#else
          servo_settings->x += t_step;
#endif
          t_step = 0;
          if (servo_settings->x > SERVO_MAX)
            servo_settings->x = SERVO_MAX;
          if (servo_settings->x < SERVO_MIN)
            servo_settings->x = SERVO_MIN;
          cc3_gpio_set_servo_position (0, servo_settings->x);
        }
        if (servo_settings->y_control) {
          if (t_pkt->centroid_y > y_mid + servo_settings->tilt_range_far)
            t_step = servo_settings->tilt_step;
          else if (t_pkt->centroid_y >
                   y_mid + servo_settings->tilt_range_near)
            t_step = servo_settings->tilt_step / 2;

          if (t_pkt->centroid_y < y_mid - servo_settings->tilt_range_far)
            t_step = -(servo_settings->tilt_step);
          else if (t_pkt->centroid_y <
                   y_mid - servo_settings->tilt_range_near)
            t_step = -(servo_settings->tilt_step / 2);

#ifdef SERVO_REVERSE_DIRECTION_TILT
          servo_settings->y -= t_step;
#else
          servo_settings->y += t_step;
#endif

          if (servo_settings->y > SERVO_MAX)
            servo_settings->y = SERVO_MAX;
          if (servo_settings->y < SERVO_MIN)
            servo_settings->y = SERVO_MIN;
          cc3_gpio_set_servo_position (1, servo_settings->y);
        }
      }

      if (!quiet) {
        if (!(packet_filter_flag &&
              t_pkt->num_pixels == 0 && prev_packet_empty)) {
          cmucam2_write_t_packet (t_pkt, servo_settings);
        }
      }
      prev_packet_empty = t_pkt->num_pixels == 0;
    }
    else
      return;

    while (!cc3_uart_has_data (0)) {
      if (getchar () == '\r')
        break;
    }
  } while (!poll_mode);
  free (img.pix);
  return;
}

void cmucam2_write_t_packet (cc3_track_pkt_t * pkt,
                             cmucam2_servo_t * servo_settings)
{

  // cap at 255
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

  // values to print
  uint8_t p[8];

  if (pkt->num_pixels == 0) {
    p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = p[6] = p[7] = 0;
  }
  else {
    p[0] = pkt->centroid_x;
    p[1] = pkt->centroid_y;
    p[2] = pkt->x0;
    p[3] = pkt->y0;
    p[4] = pkt->x1;
    p[5] = pkt->y1;
    p[6] = pkt->num_pixels;
    p[7] = pkt->int_density;
  }

  uint8_t mask = t_pkt_mask;
  if (raw_mode_output) {
    putchar (255);
  }
  printf ("T");

  // print out fields using mask
  for (int i = 0; i < 8; i++) {
    if (mask & 0x1) {
      if (raw_mode_output) {
        raw_print (p[i]);
      }
      else {
        printf (" %d", p[i]);
      }
    }
    mask >>= 1;
  }

  // print servo settings?
  if (servo_settings != NULL) {
    uint8_t sx = servo_settings->x;
    uint8_t sy = servo_settings->y;

    if (servo_settings->x_report) {
      if (raw_mode_output) {
        raw_print (sx);
      }
      else {
        printf (" %d", sx);
      }
    }
    if (servo_settings->y_report) {
      if (raw_mode_output) {
        raw_print (sy);
      }
      else {
        printf (" %d", sy);
      }
    }
  }

  print_cr ();
}

void cmucam2_write_h_packet (cc3_histogram_pkt_t * pkt)
{
  uint32_t i;
  uint32_t total_pix;

  total_pix = cc3_g_pixbuf_frame.width * cc3_g_pixbuf_frame.height;

  if (raw_mode_output) {
    putchar (255);
  }
  printf ("H");

  for (i = 0; i < pkt->bins; i++) {
    pkt->hist[i] = (pkt->hist[i] * 256) / total_pix;
    if (pkt->hist[i] > 255)
      pkt->hist[i] = 255;

    if (raw_mode_output) {
      raw_print (pkt->hist[i]);
    }
    else {
      printf (" %d", pkt->hist[i]);
    }
  }

  print_cr ();
}

void cmucam2_write_s_packet (cc3_color_info_pkt_t * pkt)
{
  uint8_t pkt2[6];
  uint8_t mask = s_pkt_mask;

  if (raw_mode_output) {
    putchar (255);
  }

  pkt2[0] = pkt->mean.channel[0];
  pkt2[1] = pkt->mean.channel[1];
  pkt2[2] = pkt->mean.channel[2];
  pkt2[3] = pkt->deviation.channel[0];
  pkt2[4] = pkt->deviation.channel[1];
  pkt2[5] = pkt->deviation.channel[2];

  printf ("S");

  for (int i = 0; i < 6; i++) {
    if (mask & 0x1) {
      if (raw_mode_output) {
        putchar (pkt2[i]);
      }
      else {
        printf (" %d", pkt2[i]);
      }
    }
    mask >>= 1;
  }

  print_cr ();
}

void print_ACK ()
{
  if (!raw_mode_no_confirmations)
    printf ("ACK\r");
}

void print_NCK ()
{
  if (!raw_mode_no_confirmations)
    printf ("NCK\r");
}

void print_prompt ()
{
  printf (":");
}

void print_cr ()
{
  if (!raw_mode_output) {
    printf ("\r");
  }
}

int32_t cmucam2_get_command (cmucam2_command_t * cmd, uint32_t * arg_list)
{
  char line_buf[MAX_LINE];
  int c;
  char *token;
  bool fail = false;
  uint32_t length, argc;
  uint32_t i;

  length = 0;
  c = 0;
  while (c != '\r') {
    c = getchar ();

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
  for (i = 0; i < CMUCAM2_CMDS_COUNT; i++) {
    if (strcmp (token, cmucam2_cmds[i]) == 0) {
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

int32_t cmucam2_get_command_raw (cmucam2_command_t * cmd, uint32_t * arg_list)
{
  bool fail;
  int c;
  unsigned int i;
  uint32_t argc;

  char cmd_str[3];
  cmd_str[2] = '\0';

  // read characters
  for (i = 0; i < 2; i++) {
    c = getchar ();
    if (c == EOF) {
      return -1;
    }

    cmd_str[i] = c;
  }

  // do lookup of command
  fail = true;
  for (i = 0; i < CMUCAM2_CMDS_COUNT; i++) {
    if (strcmp (cmd_str, cmucam2_cmds[i]) == 0) {
      fail = false;
      *cmd = i;
      break;
    }
  }
  if (fail) {
    return -1;
  }

  // read argc
  c = getchar ();
  if (c == EOF) {
    return -1;
  }
  argc = c;
  if (argc > MAX_ARGS) {
    return -1;
  }

  // read args
  for (i = 0; i < argc; i++) {
    c = getchar ();
    if (c == EOF) {
      return -1;
    }

    arg_list[i] = toupper (c);
  }

  // done
  return argc;
}


void raw_print (uint8_t val)
{
  if (val == 255) {
    putchar (254);              // avoid confusion
  }
  else {
    putchar (val);
  }
}
