#define FILE_MAX_SIZE 16
//#define PNG_ENABLED
#define JPG_ENABLED

#include <cc3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef PNG_ENABLED
  #include "png.h"
#endif

#ifdef JPG_ENABLED
  #include "jpeglib.h"
  #include <cc3_jpg.h>
#endif

#include "cc3_debug.h"

typedef enum{
  TAKE_IMAGE,
  TAKE_JPG,
  TAKE_PNG,

  SEND_JPG,
  LIST_ROOT,

  RETURN, /* Received a carriage return */
  CMD_COUNT /* Number of commands */
} wsn_cmd_t;

static const char wsn_cmds[CMD_COUNT][3] = {
  [TAKE_IMAGE] = "TI",
  [TAKE_JPG] = "TJ",
  [TAKE_PNG] = "TP",
  [SEND_JPG] = "SJ",
  [RETURN] = ""
};

static void print_prompt ( void );
static void print_NCK ( void );
static void print_ACK ( void );
static int32_t wsn_get_command ( wsn_cmd_t* );
static int32_t wsn_get_next_filename ( char*, char* );

#ifdef PNG_ENABLED
static int32_t wsn_capture_png ( void );
#endif

#ifdef JPG_ENABLED
static int32_t wsn_capture_jpg ( void );
static int32_t wsn_capture_and_send_jpg ( void );
#endif

int main (void)
{
  cc3_debug_init();

  //FIXME: The baud rate is hard coded :(
  /* Initialize the UART */
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
      CC3_UART_BINMODE_BINARY);

  /* Make stdout unbuffered */
  setvbuf (stdout, NULL, _IONBF, 0);

  /* Initialize camera */
  if (!cc3_camera_init ()) {
    cc3_led_set_state (0, true);
    exit (1);
  }
  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);

  /* use MMC */
  cc3_filesystem_init();

  /* give time to settle */
  cc3_timer_wait_ms(1000);

  /* We wait for a command*/
  wsn_cmd_t command;
  int32_t cmd_res = 0;

  while (true)
  {
    cmd_res = wsn_get_command ( & command );

    if ( cmd_res == -1 )
      continue;

    switch ( command )
    {
      case TAKE_IMAGE:
      case TAKE_JPG:
#ifdef JPG_ENABLED
        if ( wsn_capture_jpg () == -1 )
          print_NCK ();
        else
          print_ACK();
#else
        print_NCK();
#endif
        break;

      case TAKE_PNG:
#ifdef PNG_ENABLED
        if ( wsn_capture_png () == -1 )
          print_NCK ();
        else
          print_ACK ();
#else
        print_NCK ();
#endif
        break;

      case SEND_JPG:
#ifdef JPG_ENABLED
        wsn_capture_and_send_jpg ();
        print_ACK ();
#else
        print_NCK ();
#endif
        break;

      case RETURN:
        break;

      default:
        break;
    }
    print_prompt ();
  }
  return 0;
}

int32_t wsn_get_command (wsn_cmd_t * cmd)
{
  char line_cmd[4] = { '\0', '\0', '\0', '\0' };

  /* get at least 2 chars from stdin */
  for ( int i = 0, c = 0 ; i < 3 && c != '\r' && c != EOF ; i ++ )
  {
    c = getchar ();
    if ( c == '\r' || c == '\n' || c == EOF )
      break;
    else
      line_cmd[i] = c;
  }

  /* Check for empty command */
  if ( strlen ( line_cmd ) == 0 )
  {
    *cmd = RETURN;
    return 0;
  }

  /* Normalize name of the command */
  for ( unsigned int i = 0 ; i < strlen (line_cmd) ; i++)
    line_cmd[i] = toupper (line_cmd[i]);

  bool fail = true;
  for ( int i = 0 ; i < CMD_COUNT ; i++)
    if ( strcmp (line_cmd, wsn_cmds[i]) == 0 )
    {
      fail = false;
      *cmd = i;
      break;
    }

  if (fail)
    return -1;

  return 0;
}

#ifdef JPG_ENABLED
int32_t wsn_capture_jpg ()
{
  char filename[FILE_MAX_SIZE];
  FILE *f;

  if ( wsn_get_next_filename ( filename, "jpg" ) == -1 )
    return -1;

  f = fopen(filename, "w");
  if ( f == NULL )
    return -1;

  /* initialize jpg */
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  cinfo.image_width = cc3_g_pixbuf_frame.width;
  cinfo.image_height = cc3_g_pixbuf_frame.height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 80, true);

  /* read jpg into file */
  uint8_t *row;
  row = cc3_malloc_rows(1);
  if( row == NULL )
    return -1;

  JSAMPROW row_pointer[1];
  row_pointer[0] = row;

  jpeg_stdio_dest(&cinfo, f); // output is file
  cc3_pixbuf_load(); // capture a frame to the FIFO

  jpeg_start_compress(&cinfo, TRUE); // read and compress
  while (cinfo.next_scanline < cinfo.image_height) {
    cc3_pixbuf_read_rows(row, 1);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* keep things tidy */
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  free(row);
  fclose(f);

  return 0;
}

int32_t wsn_capture_and_send_jpg ()
{
  cc3_jpeg_send_simple ();
  printf ( "END_JPG" ); /* Give a hint as to when it ends */
  return 0;
}
#endif

#ifdef PNG_ENABLED
int32_t wsn_capture_png ()
{
  /* Check if file exists, skip over existing ones */
  char filename[FILE_MAX_SIZE];
  FILE *f;

  if ( wsn_get_next_filename ( filename, "png" ) == -1 )
    return -1;

  f = fopen(filename, "w");
  if ( f == NULL )
    return -1;

  /* Capture PNG */
  cc3_pixbuf_load();
  png_structp png_ptr =
    png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    return -1;

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr,
                             (png_infopp)NULL);
    return -1;
  }

  // Initial settings.
  png_set_compression_mem_level(png_ptr, 5);
  png_set_compression_window_bits(png_ptr, 11);

  // more png
  uint32_t size_x, size_y;
  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;
  png_init_io(png_ptr, f);
  png_set_IHDR(png_ptr, info_ptr, size_x, size_y,
               8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // header
  png_write_info(png_ptr, info_ptr);

  // write row by row
  uint8_t *row = cc3_malloc_rows(1);
  for ( unsigned int y = 0 ; y < size_y ; y++ )
  {
    cc3_pixbuf_read_rows(row, 1);
    png_write_row(png_ptr, row);
  }
  free(row);

  // End
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  if ( fclose(f) == EOF )
    return -1; /* The file did not close properly */

  return 0;
}
#endif

int32_t wsn_get_next_filename ( char * buffer, char * ext )
{
  bool exists = true;
  FILE *f;
  int i = 0;
  do {
#ifdef VIRTUAL_CAM
    snprintf(buffer, FILE_MAX_SIZE, "img%.5d.%s", i, ext);
#else
    snprintf(buffer, FILE_MAX_SIZE, "c:/img%.5d.%s", i, ext);
#endif
    f = fopen(buffer, "r");
    if (f == NULL) /* file does not exists */
      exists = false;
    else
      if ( fclose(f) == EOF )
        return -1; /* file did not close properly */
    i++;
  } while(exists);
  return 0;
}

void print_prompt () { printf (":"); }
void print_NCK () { printf ("NCK\r"); }
void print_ACK () { printf ("ACK\r"); }
