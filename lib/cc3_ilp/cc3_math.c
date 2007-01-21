#include "cc3_math.h"


#define iter1(N) \
    try = root + (1 << (N)); \
    if (n >= try << (N))   \
    {   n -= try << (N);   \
        root |= 2 << (N); \
    }

uint32_t cc3_isqrt (uint32_t n)
{
    uint32_t root = 0, try;
    iter1 (15);    iter1 (14);    iter1 (13);    iter1 (12);
    iter1 (11);    iter1 (10);    iter1 ( 9);    iter1 ( 8);
    iter1 ( 7);    iter1 ( 6);    iter1 ( 5);    iter1 ( 4);
    iter1 ( 3);    iter1 ( 2);    iter1 ( 1);    iter1 ( 0);
    return root >> 1;
}



double cc3_mean( uint8_t data[], uint32_t size ) 
{

  double mean = 0.0;
  uint32_t sum = 0;
  uint32_t i;

  for (i = 0; i < size; i++) {
    sum = sum + data[i];
  }

  mean = sum / size;
  return mean;

} 




/**

  Calculate the linear regression coefficients, a and b, along with
  the standard regression error (e.g., the error of b), the standard
  deviation of the points and the correlation.

  A regression line is described by the equation y' = a + bx.  The
  coefficients a and b are returned in a lineInfo object, along
  with the other values.

  Formally, linear regression of a set of {x,y} points is described in
  terms of independent and dependent variables.  The array x contains
  the independent variable, which is exactly known.  The array y
  contains the dependent variable, which is "measured".  The y values
  are assumed to be random values, dependent on the x values.

 */

void cc3_linear_reg(uint8_t x_data[], uint8_t y_data[], uint8_t size,cc3_linear_reg_data_t *reg_out )
{


  if (size > 0) {



    double muX = cc3_mean( x_data,size );
    double muY = cc3_mean( y_data,size );


    //     N-1
    //     ---
    //     \   (Xi - meanX)(Yi - meanY)
    //     /__ 
    //     i=0
    // b = -----------------------------
    //     N-1
    //     ---             2
    //     \   (Xi - meanX)
    //     /__ 
    //     i=0
    //

    double SSxy = 0;
    double SSxx = 0;
    double SSyy = 0;
    double Sx = 0;
    double Sy = 0;
    double Sxy = 0;
    double SSy = 0;
    double SSx = 0;

    for (uint8_t i = 0; i < size; i++) {
      double subX = (x_data[i] - muX);
      double subY = (y_data[i] - muY);
      Sx = Sx + x_data[i];
      Sy = Sy + y_data[i];
      Sxy = Sxy + (x_data[i] * y_data[i]);
      SSx = SSx + (x_data[i] * x_data[i]);
      SSy = SSy + (y_data[i] * y_data[i]);
      SSyy = SSyy + subY * subY;
      SSxy = SSxy + subX * subY;
      SSxx = SSxx + subX * subX;

    }

    

    // slope
    double m = SSxy / SSxx;
    // intercept
    double b = muY - m * muX;

    double r2Numerator = (size * Sxy) - (Sx * Sy);
    double r2Denominator = ((size*SSx) - (Sx * Sx))*((size*SSy) - (Sy * Sy));
    double r2 = (r2Numerator * r2Numerator) / r2Denominator;

    reg_out->r_sqr = r2; 
    reg_out->b = b; 
    reg_out->m = m; 
  } // if N > 0

} // lineInfo


