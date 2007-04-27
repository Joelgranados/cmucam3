/****************************
 * polly-cmucam-gui
 *
 * This is a modified version of the polly project that returns
 * histograms such that the CMUcam2 GUI can display them.  
 * With this firmware loaded, the "GH" get histogram command will 
 * return a downsampled polly histogram.
 ****************************/

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_img_writer.h>
#include "polly.h"

#define BINS 28
static const unsigned int MAX_LINE = 128;
static const unsigned int MAX_ARGS = 10;

typedef enum {
  GET_HISTOGRAM,
  GET_VERSION,
  RESET,
  RETURN,                       // Must be second to last
  CMUCAM2_CMDS_COUNT            // Must be last entry so array sizes are correct
} cmucam2_command_t;

static const char cmucam2_cmds[CMUCAM2_CMDS_COUNT][3] = {
  [RETURN] = "",
  /* Histogram Commands */
  [GET_HISTOGRAM] = "GH",
  [GET_VERSION] = "GV",
  [RESET] = "RS"
};

int32_t cmucam2_get_command (cmucam2_command_t * cmd, uint32_t * arg_list);

int main (void)
{
  uint32_t last_time, val, i, step;
  char c;
  uint8_t *x_axis, *h, cnt, conf;
  polly_config_t p_config;

  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (true);
  cc3_camera_set_auto_exposure (true);


  cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 2);
  cc3_pixbuf_frame_set_coi (CC3_CHANNEL_GREEN);

  cc3_led_set_state (0, false);
  cc3_led_set_state (1, false);
  cc3_led_set_state (2, false);

  // sample wait command in ms 
  cc3_timer_wait_ms (1000);

  // initialize pixbuf
  cc3_pixbuf_load ();

  x_axis = malloc (cc3_g_pixbuf_frame.width);
  h = malloc (cc3_g_pixbuf_frame.width);

  p_config.color_thresh = 10;
  p_config.min_blob_size = 20;
  p_config.connectivity = 1;
  p_config.horizontal_edges = 1;
  p_config.vertical_edges = 1;
  p_config.blur = 1;
  p_config.histogram = malloc (cc3_g_pixbuf_frame.width);

  printf ("CMUcam2 v1.00 c6\r");
  while (1) {
    uint32_t val, n, step, go;
    cmucam2_command_t command;
    uint32_t arg_list[MAX_ARGS];
    printf (":");
    // Parse out the GH command using the CMUcam2 command parser
    n = cmucam2_get_command (&command, arg_list);
    if (n != -1) {

      // If it is the get histogram command, then run Polly Algorithm
      if (command == GET_HISTOGRAM) {
        printf ("ACK\r");
        go = true;
        step = cc3_g_pixbuf_frame.width / BINS;
        step--;
	// Continue to load histograms until a '\r' is detected
        while (go) {
	  // Load a frame
          cc3_pixbuf_load ();
	  // Run polly on the frame
          polly (p_config);
          // p_config.histogram gets filled with the return data
          printf ("H");
          for (i = 0; i < BINS; i++) {
            // Downsample bins to 28 so that it is viewable in the CMUcam2 GUI
	    // Use MIN of multiple bins
            val = 255;
            for (uint32_t j = 0; j < step; j++)
              if (p_config.histogram[(i * step) + j] < val)
                val = p_config.histogram[(i * step) + j];
            printf (" %d", val);
          }
          printf ("\r");
          // Check for data in Uart
          while (!cc3_uart_has_data (0)) {
            if (getchar () == '\r') {
              go = false;
              break;
            }
          }
        }
      }
      else if (command == RETURN)
        printf ("ACK\r");
      else if (command == GET_VERSION)
        printf ("ACK\rCMUcam2 v1.00 c6\r");
      else if (command == RESET)
        printf ("ACK\rCMUcam2 v1.00 c6\r");
      else
        printf ("NCK\r");
    }
    else
      printf ("NCK\r");
  }

  return 0;
}


// CMUcam2 command parser
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
