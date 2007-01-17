#ifndef _LREG_H_
#define _LREG_H_

#include <stdint.h>


typedef struct {
   double r_sqr;  // correlation squared
   double a;  // intercept 
   double b;  // slope 
   double stddevPoints;  
   double bError;  
} reg_data_t;



double lreg_mean( uint8_t data[], uint32_t size );
void lreg(uint8_t  x_data[], uint8_t  y_data[], uint8_t size,reg_data_t *reg_out );
uint32_t isqrt (uint32_t n);

#endif
