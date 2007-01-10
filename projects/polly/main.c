#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>


#define COLOR_THRESH   25
#define MIN_BLOB_SIZE  15

#define WIDTH	88
#define HEIGHT	72

// Constants for connected component blob reduce
#define SELECTED	255
#define NOT_SELECTED	0
#define MARKED		2
#define FINAL_SELECTED	1



void connected_component_reduce (cc3_image_t * img, int min_blob_size);
void generate_histogram (cc3_image_t * img, uint8_t * hist);
void matrix_to_ppm (cc3_image_t * img);
void convert_histogram_to_ppm (cc3_image_t * img, uint8_t * hist);
int count (cc3_image_t * img, int x, int y, int steps);
int reduce (cc3_image_t * img, int x, int y, int steps, int remove);



/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t start_time, val;
  char c;
  cc3_pixel_t p;
  cc3_image_t polly_img;
  cc3_image_t img;
  uint8_t range[WIDTH];
  //uint8_t p_img[WIDTH][HEIGHT];
  uint8_t p_img[HEIGHT * WIDTH];


  // setup system    
  cc3_system_setup ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_set_colorspace (CC3_RGB);
  cc3_set_resolution (CC3_LOW_RES);
  cc3_set_auto_white_balance (true);
  cc3_set_auto_exposure (true);


  cc3_pixbuf_set_subsample (CC3_NEAREST, 2, 2);
  cc3_pixbuf_set_coi (CC3_RED);

  cc3_clr_led (0);
  cc3_clr_led (1);
  cc3_clr_led (2);

  // sample wait command in ms 
  cc3_wait_ms (1000);
  cc3_set_led (0);

  // setup an image structure 
  //img.channels=CC3_GREEN;
  img.channels = 1;
  img.width = cc3_g_current_frame.width;
  img.height = cc3_g_current_frame.height;      // image will hold just 1 row for scanline processing
  //img.pix = cc3_malloc_rows(1);
  img.pix = cc3_malloc_rows (cc3_g_current_frame.height);
  if (img.pix == NULL) {
    printf ("Not enough memory...\n");
    exit (0);
  }
  printf ("img size = %d, %d\n", cc3_g_current_frame.width,
          cc3_g_current_frame.height);

  // setup polly temporary image
  polly_img.width = WIDTH;
  polly_img.height = HEIGHT;
  polly_img.channels = 1;
  polly_img.pix = &p_img;

  while (1) {
    cc3_pixel_t mid_pix;
    cc3_pixel_t right_pix;
    cc3_pixel_t down_pix;

    p.channel[0] = 0;
    for (int y = 0; y < HEIGHT; y++)
      for (int x = 0; x < WIDTH; x++)
        cc3_set_pixel (&polly_img, x, y, &p);


    cc3_pixbuf_load ();
    cc3_pixbuf_read_rows (img.pix, cc3_g_current_frame.height);

    p.channel[0] = SELECTED;
    for (int y = 0; y < img.height - 1; y++) {
      for (int x = 0; x < img.width - 1; x++) {
        int m, r, d;
        cc3_get_pixel (&img, x, y, &mid_pix);
        cc3_get_pixel (&img, x + 1, y, &right_pix);
        cc3_get_pixel (&img, x, y + 1, &down_pix);
        m = mid_pix.channel[0];
        r = right_pix.channel[0];
        d = down_pix.channel[0];
        if (m < r - COLOR_THRESH || m > r + COLOR_THRESH)
          cc3_set_pixel (&polly_img, x, y, &p);
        //p_img[x][y]=SELECTED;              
        if (m < d - COLOR_THRESH || m > d + COLOR_THRESH)
          cc3_set_pixel (&polly_img, x, y, &p);
        //p_img[x][y]=SELECTED;              

      }
    }

    connected_component_reduce (&polly_img, MIN_BLOB_SIZE);
    generate_histogram (&polly_img, range);
    convert_histogram_to_ppm (&polly_img, range);
    matrix_to_ppm (&polly_img);


  }


  free (img.pix);               // don't forget to free!

  while (1);

  return 0;
}

int count (cc3_image_t * img, int x, int y, int steps)
{
  int size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  size = 0;
  cc3_get_pixel (img, x, y, &p);
  if (p.channel[0] == SELECTED)
    size = 1;
  steps--;
  if (steps == 0)
    return size;
  p.channel[0] = MARKED;
  cc3_set_pixel (img, x, y, &p);

  if (x > 1) {
    cc3_get_pixel (img, x - 1, y, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y, steps);
  }
  if (x < width) {
    cc3_get_pixel (img, x + 1, y, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y, steps);
  }
  if (y > 1) {
    cc3_get_pixel (img, x, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x, y - 1, steps);
  }
  if (y < height) {
    cc3_get_pixel (img, x, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x, y + 1, steps);
  }

  if (x > 1 && y > 1) {
    cc3_get_pixel (img, x - 1, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y - 1, steps);
  }
  if (x < width && y > 1) {
    cc3_get_pixel (img, x + 1, y - 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y - 1, steps);
  }
  if (x > 1 && y < height) {
    cc3_get_pixel (img, x - 1, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x - 1, y + 1, steps);
  }
  if (x < width && y < height) {
    cc3_get_pixel (img, x + 1, y + 1, &p);
    if (p.channel[0] == SELECTED)
      size += count (img, x + 1, y + 1, steps);
  }

  return size;
}

int reduce (cc3_image_t * img, int x, int y, int steps, int remove)
{
  int size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;


  size = 0;
  cc3_get_pixel (img, x, y, &p);
  if (p.channel[0] == MARKED)
    size = 1;
  steps--;
  if (steps == 0)
    return size;

  if (remove)
    p.channel[0] = NOT_SELECTED;
  else
    p.channel[0] = FINAL_SELECTED;

  cc3_set_pixel (img, x, y, &p);

  if (x > 1) {
    cc3_get_pixel (img, x - 1, y, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y, steps, remove);
  }
  if (x < width) {
    cc3_get_pixel (img, x + 1, y, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y, steps, remove);
  }
  if (y > 1) {
    cc3_get_pixel (img, x, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x, y - 1, steps, remove);
  }
  if (y < height) {
    cc3_get_pixel (img, x, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x, y + 1, steps, remove);
  }

  if (x > 1 && y > 1) {
    cc3_get_pixel (img, x - 1, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y - 1, steps, remove);
  }
  if (x < width && y > 1) {
    cc3_get_pixel (img, x + 1, y - 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y - 1, steps, remove);
  }
  if (x > 1 && y < height) {
    cc3_get_pixel (img, x - 1, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x - 1, y + 1, steps, remove);
  }
  if (x < width && y < height) {
    cc3_get_pixel (img, x + 1, y + 1, &p);
    if (p.channel[0] == MARKED)
      size += reduce (img, x + 1, y + 1, steps, remove);
  }

  return size;
}



void connected_component_reduce (cc3_image_t * img, int min_blob_size)
{
  int x, y, size;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED) {
        size = count (img, x, y, min_blob_size);
        if (size < min_blob_size)
          reduce (img, x, y, min_blob_size, 1); // Delete marked
        else
          reduce (img, x, y, min_blob_size, 0); // Finalize marked
      }
    }


  // Translate for PPM
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == FINAL_SELECTED) {
        p.channel[0] = SELECTED;
        cc3_set_pixel (img, x, y, &p);
      }
    }
}


void generate_histogram (cc3_image_t * img, uint8_t * hist)
{
  int x, y;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  for (x = 0; x < width; x++)
    hist[x] = 0;


  for (x = width / 2; x < width; x++) {

    for (y = height - 1; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        //if(p_img[x][y]==SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  for (x = width / 2; x > 0; x--) {

    for (y = height - 1; y > 0; y--) {
      cc3_get_pixel (img, x, y, &p);
      if (p.channel[0] == SELECTED)
        //if(p_img[x][y]==SELECTED)
        break;
    }
    hist[x] = (height - 1 - y);
    if (y > height - 5)
      break;
  }

  // Downsample Histogram 
  for (x = 0; x < width - 5; x += 5) {
    int min;
    min = 100;
    for (int i = 0; i < 5; i++) {
      if (hist[x + i] < min)
        min = hist[x + i];
    }
    for (int i = 0; i < 5; i++)
      hist[x + i] = min;
  }





}

void convert_histogram_to_ppm (cc3_image_t * img, uint8_t * hist)
{
  int x, y;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  // Write the range image out    
  p.channel[0] = 0;
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++)
      cc3_set_pixel (img, x, y, &p);

  p.channel[0] = 255;
  for (x = 0; x < width; x++) {

    for (y = height - 1; y > height - 1 - (hist[x]); y--) {
      cc3_set_pixel (img, x, y, &p);
    }
  }
}



void matrix_to_ppm (cc3_image_t * img)
{
  static int cnt = 0;
  char str[32];
  FILE *fp;
  int width, height;
  cc3_pixel_t p;

  width = img->width;
  height = img->height;

  sprintf (str, "out_%d.pgm", cnt);
  cnt++;
  fp = fopen (str, "w");
  if (fp == NULL) {
    printf ("Can't open file...\n");
    exit (0);
  }
  fprintf (fp, "P5\n%d %d 255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      cc3_get_pixel (img, x, y, &p);
      fprintf (fp, "%c", p.channel[0]);
    }
  }
  fclose (fp);

}
