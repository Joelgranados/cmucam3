#ifndef _CC3_MATH_H_
#define _CC3_MATH_H_

#include <stdint.h>


typedef struct {
   double r_sqr;  // correlation squared
   double b;  // intercept 
   double m;  // slope 
} cc3_linear_reg_data_t;



double cc3_mean( uint8_t data[], uint32_t size );
void cc3_linear_reg(uint8_t  x_data[], uint8_t  y_data[], uint8_t size,cc3_linear_reg_data_t *reg_out );
uint32_t cc3_isqrt (uint32_t n);

#endif
