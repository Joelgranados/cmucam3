#include <cc3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "spoonBot.h"


int main (void)
{
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
    
    cc3_camera_init ();

    printf ("Starting up...\n");
    cc3_led_set_state (0, true);


    cc3_gpio_set_mode(0, CC3_GPIO_MODE_SERVO);
    cc3_gpio_set_mode(1, CC3_GPIO_MODE_SERVO);
    cc3_gpio_set_mode(2, CC3_GPIO_MODE_SERVO);
    cc3_gpio_set_mode(3, CC3_GPIO_MODE_OUTPUT);


     //printf ("Get calibration...\n");
     //spoonBot_get_calibration();
     spoonBot_calibrate(75,69,210,1,-1,1 );   // These numbers came from the calibration
     printf( "SpoonBot!\n" );
     spoonBot_stop();
     spoonBot_wait(200);
     printf( "SpoonBot Down\n" );
     spoonBot_spoon_pos(-150);
     spoonBot_wait(200);
     printf( "SpoonBot Up\n" );
     spoonBot_spoon_pos(0);
     spoonBot_wait(200);
     printf( "SpoonBot Right\n" );
     spoonBot_right(50);
     spoonBot_wait(100);
     spoonBot_stop();
     printf( "SpoonBot Left\n" );
     spoonBot_left(50);
     spoonBot_wait(100);
     spoonBot_stop();
     printf( "SpoonBot Done\n" );

while(1);

    return 0;
}

