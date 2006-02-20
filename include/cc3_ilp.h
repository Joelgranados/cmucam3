#ifndef CC3_ILP_H
#define CC3_ILP_H

typedef struct {
    uint16_t width, height;
    uint8_t channels;
    void* pix;
} cc3_image_t;

typedef struct {
    uint16_t x0,y0,x1,y1;
    uint16_t centroid_x, centroid_y;
    uint32_t num_pixels;
    uint32_t int_density;
    uint8_t noise_filter;
    cc3_pixel_t *upper_bound;
    cc3_pixel_t *lower_bound;
} cc3_track_pkt_t;

void cc3_get_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *out_pix);
void cc3_set_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *in_pix);

uint8_t cc3_track_color(cc3_track_pkt_t *pkt);
uint8_t cc3_track_color_img(cc3_image_t *img, cc3_track_pkt_t *pkt);

#endif
