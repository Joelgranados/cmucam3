#include "spoonBot.h"
#include <cc3.h>
#include <time.h>
#include <stdio.h>
#include "inttypes.h"


uint32_t SBOT_LEFT_MID, SBOT_RIGHT_MID;
uint32_t SBOT_SPOON_MID, SBOT_SPOON_DOWN, SBOT_SPOON_UP;
int8_t SBOT_LEFT_DIR, SBOT_RIGHT_DIR, SBOT_SPOON_DIR;

void spoonBot_wait (uint32_t delay)
{
  uint32_t t;
  t = clock ();
  while (clock () < t + delay);

}

void spoonBot_spoon_pos (int position)
{

  if (SBOT_SPOON_DIR < 0) {
    if (SBOT_SPOON_MID - position < SBOT_SPOON_DOWN)
      cc3_gpio_set_servo_position (2, SBOT_SPOON_DOWN);
    else
      cc3_gpio_set_servo_position (2, SBOT_SPOON_MID - position);
  }
  else {
    if (SBOT_SPOON_MID + position > SBOT_SPOON_UP)
      cc3_gpio_set_servo_position (2, SBOT_SPOON_UP);
    else
      cc3_gpio_set_servo_position (2, SBOT_SPOON_MID + position);
  }
}

void spoonBot_left (int speed)
{
  if (SBOT_LEFT_DIR > 0)
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID - speed);
  else
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID + speed);
  if (SBOT_RIGHT_DIR > 0)
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID + speed);
  else
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID - speed);
}

void spoonBot_right (int speed)
{
  if (SBOT_LEFT_DIR > 0)
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID + speed);
  else
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID - speed);
  if (SBOT_RIGHT_DIR > 0)
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID - speed);
  else
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID + speed);

}

void spoonBot_drive (int speed)
{
  if (SBOT_LEFT_DIR > 0)
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID + speed);
  else
    cc3_gpio_set_servo_position (0, SBOT_LEFT_MID - speed);
  if (SBOT_RIGHT_DIR > 0)
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID + speed);
  else
    cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID - speed);

}

void spoonBot_stop ()
{
  cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID);
  cc3_gpio_set_servo_position (0, SBOT_LEFT_MID);
  //cc3_gpio_set_servo_position(2,SBOT_SPOON_MID);
}

void spoonBot_calibrate (spoonBot_calibrate_t cal)
{
  SBOT_LEFT_MID = cal.left_mid;
  SBOT_RIGHT_MID = cal.right_mid;
  SBOT_SPOON_MID = cal.spoon_mid;
  SBOT_SPOON_DOWN = cal.spoon_down;
  SBOT_SPOON_UP = cal.spoon_up;
  SBOT_SPOON_DIR = cal.spoon_dir;
  SBOT_RIGHT_DIR = cal.right_dir;
  SBOT_LEFT_DIR = cal.left_dir;
  cc3_gpio_set_servo_position (0, SBOT_LEFT_MID);
  cc3_gpio_set_servo_position (1, SBOT_RIGHT_MID);
  cc3_gpio_set_servo_position (2, SBOT_SPOON_MID);
}


// This function allows you to enter servo positions to test out the calibration.
// Entering 300 as a servo position will exit.
void spoonBot_get_calibration ()
{
  uint32_t left, right, spoon, val;

  cc3_gpio_set_mode (0, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_OUTPUT);
  cc3_gpio_set_mode (2, CC3_GPIO_MODE_OUTPUT);
  val = 0;
  printf ("Enter Servo Position, or 300 to move to the next servo\r\n");
  while (val != 300) {
    printf ("Left Wheel: ");
    scanf ("%d", &val);
    if (val != 300)
      left = val;
    cc3_gpio_set_servo_position (0, left);
  }
  cc3_gpio_set_mode (0, CC3_GPIO_MODE_OUTPUT);
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_SERVO);
  val = 0;
  while (val != 300) {
    printf ("Right Wheel: ");
    scanf ("%d", &val);
    if (val != 300)
      right = val;
    cc3_gpio_set_servo_position (1, right);
  }
  val = 0;
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_OUTPUT);
  cc3_gpio_set_mode (2, CC3_GPIO_MODE_SERVO);
  while (val != 300) {
    printf ("Spoon: ");
    scanf ("%d", &val);
    if (val != 300)
      spoon = val;
    cc3_gpio_set_servo_position (2, spoon);
  }
  printf ("left = %d, right = %d, spoon = %d\n", left, right, spoon);
}

/*
void spoonBot_laser_on()
{
REG(GPIO_IOSET)=_CC3_SERVO_3;
}

void spoonBot_laser_off()
{
REG(GPIO_IOCLR)=_CC3_SERVO_3;
}

void spoonBot_laser_pulse(int duration)
{
uint32_t start;
start=clock();
REG(GPIO_IOSET)=_CC3_SERVO_3;
while(clock()<start+duration);
REG(GPIO_IOCLR)=_CC3_SERVO_3;
}
*/
