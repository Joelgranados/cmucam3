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
} polly_config_t;

typedef struct {
	uint8_t max_depth;
	uint8_t min_blob_size;
	uint8_t connectivity;
} ccr_config_t;

ccr_config_t g_cc_conf;

int polly( polly_config_t config);
void connected_component_reduce (cc3_image_t * img, ccr_config_t config);
void generate_polly_histogram (cc3_image_t * img, uint8_t * hist);
void matrix_to_pgm (cc3_image_t * img);
void convert_histogram_to_ppm (cc3_image_t * img, uint8_t * hist);
int count (cc3_image_t * img, int x, int y, int steps);
int reduce (cc3_image_t * img, int x, int y, int steps, int remove);
void write_raw_fifo_ppm();



