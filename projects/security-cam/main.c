#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>
#include <jpeglib.h>
#include <cc3_frame_diff.h>

// How many pixels should change in a frame to be seen as motion
// Remember, the frame is downsampled to 16x16
#define NUM_PIX_CHANGE_THRESH     10

// How much the color of a single pixel should change to be detected 
#define PIX_CHANGE_THRESH	  20


static void capture_current_jpeg (FILE * f);
static void init_jpeg (void);
static void destroy_jpeg (void);

int main (void)
{
  uint32_t img_cnt;
  void *tmp;
  FILE *f;
  cc3_frame_diff_pkt_t fd_pkt;
  cc3_image_t img;
  uint32_t pixel_change_threshold;

  pixel_change_threshold = NUM_PIX_CHANGE_THRESH;




  cc3_uart_init (0,
                 CC3_UART_RATE_115200,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();

  cc3_filesystem_init ();

  //cc3_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_HIGH);
  // Set camera to low-res for faster frame differencing
  // cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  // cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 2);
  cc3_timer_wait_ms (1000);

  // init pixbuf with width and height
  cc3_pixbuf_load ();

  // init jpeg
  init_jpeg ();

  cc3_led_set_state (1, true);

  

  fd_pkt.coi = 1;
  cc3_pixbuf_frame_set_coi (fd_pkt.coi);
  fd_pkt.template_width = 16;
  fd_pkt.template_height = 16;
  fd_pkt.previous_template =
    malloc (fd_pkt.template_width * fd_pkt.template_height *
            sizeof (uint32_t));

  if (fd_pkt.previous_template == NULL)
    printf ("Malloc FD startup error!\r");

  fd_pkt.current_template =
    malloc (fd_pkt.template_width * fd_pkt.template_height *
            sizeof (uint32_t));

  if (fd_pkt.current_template == NULL)
    printf ("Malloc FD startup error!\r");

  fd_pkt.total_x = cc3_g_pixbuf_frame.width;
  fd_pkt.total_y = cc3_g_pixbuf_frame.height;
  fd_pkt.load_frame = 1;        // load a new frame
  fd_pkt.threshold = PIX_CHANGE_THRESH;

  img.channels = 1;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (img.width);


  img_cnt = 0;
  while (true) {
    char filename[32];


    // Wait some time for the change to go away and for the image to stabalize
    cc3_timer_wait_ms (2000);

    cc3_pixbuf_load ();
    fd_pkt.load_frame = 1;      // load a new frame
    if (cc3_frame_diff_scanline_start (&fd_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
        cc3_frame_diff_scanline (&img, &fd_pkt);
      }
      cc3_frame_diff_scanline_finish (&fd_pkt);
    }
    else
      printf ("frame diff start error\r");

    fd_pkt.load_frame = 0;      // load a new frame
    do {

      cc3_pixbuf_load ();
      if (cc3_frame_diff_scanline_start (&fd_pkt) != 0) {
        while (cc3_pixbuf_read_rows (img.pix, 1)) {
          cc3_frame_diff_scanline (&img, &fd_pkt);
        }
        cc3_frame_diff_scanline_finish (&fd_pkt);
      }
      else
        printf ("frame diff start error\r");

      // swap last frame template with current 
      tmp = fd_pkt.previous_template;
      fd_pkt.previous_template = fd_pkt.current_template;
      fd_pkt.current_template = tmp;
      printf ("diff from last frame: %d\r\n", fd_pkt.num_pixels);
    } while (fd_pkt.num_pixels < pixel_change_threshold);

    printf ("Changed detected, write jpg!\r\n");

    // Check if files exist, if they do then skip over them 
    do {
#ifdef VIRTUAL_CAM
      snprintf (filename, 16, "img%.5d.jpg", img_cnt);
#else
      snprintf (filename, 16, "c:/img%.5d.jpg", img_cnt);
#endif
      f = fopen (filename, "r");
      if (f != NULL) {
        printf ("%s already exists...\n", filename);
        img_cnt++;
        fclose (f);
      }
    } while (f != NULL);

    // print file that you are going to write to stderr
    fprintf (stderr, "<%s>\r\n", filename);
    f = fopen (filename, "w");
    if (f == NULL || img_cnt > 200) {
      cc3_led_set_state (3, true);
      printf ("Error: Can't open file\r\n");
      if (img_cnt > 200)
        printf ("Card full\r\n");
      while (1);
    }
    // Switch to full color for stored images
    cc3_pixbuf_frame_set_coi (CC3_CHANNEL_ALL);
    // Save the jpeg
    capture_current_jpeg (f);
    // Switch back to a single COI for the frame diff
    cc3_pixbuf_frame_set_coi (fd_pkt.coi);

    fclose (f);
    img_cnt++;
  }


  destroy_jpeg ();
  return 0;
}




static struct jpeg_compress_struct cinfo;
static struct jpeg_error_mgr jerr;
//static cc3_pixel_t *row;
uint8_t *row;

void init_jpeg (void)
{
  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_compress (&cinfo);

  // parameters for jpeg image
  cinfo.image_width = cc3_g_pixbuf_frame.width;
  cinfo.image_height = cc3_g_pixbuf_frame.height;
  printf ("image width=%d image height=%d\n", cinfo.image_width,
          cinfo.image_height);
  cinfo.input_components = 3;
  // cinfo.in_color_space = JCS_YCbCr;
  cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_defaults (&cinfo);
  jpeg_set_quality (&cinfo, 100, true);

  // allocate memory for 1 row
  row = cc3_malloc_rows (1);
  if (row == NULL)
    printf ("Out of memory!\n");
}

void capture_current_jpeg (FILE * f)
{
  JSAMPROW row_pointer[1];
  row_pointer[0] = row;

  // output is file
  jpeg_stdio_dest (&cinfo, f);

  // capture a frame to the FIFO
  //cc3_pixbuf_load();
  cc3_pixbuf_rewind ();

  // read and compress
  jpeg_start_compress (&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    cc3_pixbuf_read_rows (row, 1);
    jpeg_write_scanlines (&cinfo, row_pointer, 1);
  }

  // finish
  jpeg_finish_compress (&cinfo);
}



void destroy_jpeg (void)
{
  jpeg_destroy_compress (&cinfo);
  free (row);
}
