// Values that seem to work well
//#define COLOR_THRESH   25
//#define MIN_BLOB_SIZE  15

#define WIDTH	88
#define HEIGHT	72

// Constants for connected component blob reduce
#define SELECTED	255
#define NOT_SELECTED	0
#define MARKED		2
#define FINAL_SELECTED	1


void polly( uint8_t color_thresh, uint8_t min_blob_size );
void connected_component_reduce (cc3_image_t * img, int min_blob_size);
void generate_histogram (cc3_image_t * img, uint8_t * hist);
void matrix_to_pgm (cc3_image_t * img);
void convert_histogram_to_ppm (cc3_image_t * img, uint8_t * hist);
int count (cc3_image_t * img, int x, int y, int steps);
int reduce (cc3_image_t * img, int x, int y, int steps, int remove);
void write_raw_fifo_ppm();



