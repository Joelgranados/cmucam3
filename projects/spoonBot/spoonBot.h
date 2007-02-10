#ifndef SPOONBOT_H
#define SPOONBOT_H

#include "inttypes.h"

void spoonBot_spoon_pos( int position);
void spoonBot_left( int speed);
void spoonBot_right( int speed);
void spoonBot_drive( int speed);
void spoonBot_stop(void);
void spoonBot_calibrate(uint32_t left_mid, uint32_t right_mid, uint32_t spoon_mid, int8_t left_dir, int8_t right_dir, int8_t spoon_dir);
void spoonBot_get_calibration(void);
void spoonBot_laser(int duration);
void spoonBot_laser_off(void);
void spoonBot_laser_on(void);
void spoonBot_wait(uint32_t delay);


#endif
