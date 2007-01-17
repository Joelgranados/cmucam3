#ifndef _CC3_CONNECTED_COMPONET_H_
#define _CC3_CONNECTED_COMPONENT_H_

#include <stdint.h>
#include "cc3_ilp.h" 

#define L4_CONNECTED    0
#define L8_CONNECTED    1


typedef struct {
        uint8_t max_depth;
        uint8_t min_blob_size;
        uint8_t connectivity;
} ccr_config_t;

ccr_config_t g_cc_conf;



int count (cc3_image_t * img, int x, int y, int steps);
int reduce (cc3_image_t * img, int x, int y, int steps, int remove);
void cc3_connected_component_reduce (cc3_image_t * img, ccr_config_t conf);


#endif
