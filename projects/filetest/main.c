#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>

#include <string.h>


/* simple hello world, showing features and compiling*/
int main (void)
{
  // init filesystem driver
  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_TEXT);
  // Make it so that stdout and stdin are not buffered
  setvbuf (stdout, NULL, _IONBF, 0);
  setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (true);
  cc3_camera_set_auto_exposure (true);

  printf ("Hello World...\n");

  char filename[32];
  FILE *f;

  // first, test trunc
  f = fopen("c:/trunc.txt", "w");
  fprintf(f, "invalid\n");
  fclose(f);
  f = fopen("c:/trunc.txt", "w");
  fprintf(f, "valid!\n");
  fclose(f);

  // then append
  f = fopen("c:/append.txt", "w");
  fprintf(f, "start: %d\n", ftell(f));
  fclose(f);
  f = fopen("c:/append.txt", "a");
  fprintf(f, "fopen a: %d\n", ftell(f));
  fflush(f);
  fprintf(f, " a: %d\n", ftell(f));
  fflush(f);
  fprintf(f, " a: %d\n", ftell(f));
  fflush(f);
  fclose(f);

  // then append+
  f = fopen("c:/appendpl.txt", "w");
  fprintf(f, "start: %d\n", ftell(f));
  fclose(f);
  f = fopen("c:/appendpl.txt", "a+");
  fprintf(f, "fopen +: %d\n", ftell(f));
  fflush(f);
  fprintf(f, " +: %d\n", ftell(f));
  fflush(f);
  fprintf(f, " +: %d\n", ftell(f));
  fflush(f);
  fclose(f);

  while(1);


  // tests for directory limits
  for (int i = 0; i < 1000000; i++) {
    snprintf(filename, 32, "c:/aa/f%07d.txt", i);

    printf ("open %s\n", filename);
    f = fopen(filename, "a");
    if (f == NULL) {
      perror ("fopen");
      break;
    }

    printf("write filename and close\n");
    size_t size = fwrite(filename, strlen(filename), 1, f);
    if (size != 1) {
      perror("fwrite");
      break;
    }
    int result = fclose(f);
    if (result) {
      perror("fclose");
      break;
    }
  }


  while(1);
  return 0;
}
