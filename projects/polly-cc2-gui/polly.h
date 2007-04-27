#ifndef _POLLY_H_
#define _POLLY_H_

// Constants for connected component blob reduce
#define SELECTED	255
#define NOT_SELECTED	0
#define MARKED		2
#define FINAL_SELECTED	1

#define L4_CONNECTED	0
#define L8_CONNECTED	1

typedef struct {
	uint8_t color_thresh;
	uint8_t min_blob_size;
	uint8_t connectivity;
	uint8_t horizontal_edges;
	uint8_t vertical_edges;
	uint8_t blur;
	int8_t *histogram;
} polly_config_t;

typedef struct {
	uint8_t max_depth;
	uint8_t min_blob_size;
	uint8_t connectivity;
} ccr_config_t;


int polly( polly_config_t config);
void connected_component_reduce (cc3_image_t * img, ccr_config_t config);
void generate_polly_histogram (cc3_image_t * img, int8_t * hist);
void matrix_to_pgm (cc3_image_t * img);
void convert_histogram_to_ppm (cc3_image_t * img, int8_t * hist);
int count (cc3_image_t * img, int x, int y, int steps);
int reduce (cc3_image_t * img, int x, int y, int steps, int remove);
void write_raw_fifo_ppm();


#endif
