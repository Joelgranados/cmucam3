#include <stdio.h>

#include "cc3.h"
#include "LPC2100.h"
#include "serial.h"
#include "interrupt.h"

unsigned long red_pixel, green_pixel, green_pixel2, blue_pixel;
unsigned int camera_type;

void camera_setup ()
{
    REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG; // | 
    // 0x50;
    REG (GPIO_IODIR) = DEFAULT_PORT_DIR;
    // REG(GPIO_IOSET)=CAM_BUF_ENABLE;
    // REG (GPIO_IOCLR) = CAM_BUF_ENABLE; // Change for AL440B
    REG (GPIO_IOCLR) = BUF_RESET;
    camera_reset ();
    fifo_reset ();

    camera_type = OV6620;
}

void camera_reset ()
{
    // Reset the Camera 
    REG (GPIO_IOCLR) = CAM_RESET;
    delay ();
    REG (GPIO_IOSET) = CAM_RESET;
    delay ();
    REG (GPIO_IOCLR) = CAM_RESET;
    delay ();

}

int _cc3_camera_set_reg (int reg, int val)
{
    unsigned int data[3];
    int to;
    data[0] = camera_type;
    data[1] = reg;
    data[2] = val;
    to = 0;
    while (i2c_send (3, data)) {
        to++;
        if (to > 3)
            return 0;
    }
    delay_us_4 (1);
    return 1;
}

void image_send_direct (int size_x, int size_y)
{
    int x, y;
    fifo_load_frame ();

    putchar (1);
    putchar (size_x);
    putchar (size_y);
    for (y = 0; y < size_y; y++) {
        putchar (2);
        for (x = 0; x < size_x; x++) {
            fifo_read_pixel ();
            putchar (red_pixel);
            putchar (green_pixel);
            putchar (blue_pixel);
        }
    }
    putchar (3);
}

void image_fifo_to_mem (unsigned char *img, int size_x, int size_y)
{
    int x, y;

    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            int index;
            fifo_read_pixel ();
            index = (size_x * y * 3) + (x * 3);
            img[index] = red_pixel;
            img[index + 1] = green_pixel;
            img[index + 2] = blue_pixel;
        }
    }

}

void image_send_uart (unsigned char *img, int size_x, int size_y)
{
    int y, x;

    putchar (1);                   // start of frame
    for (y = 0; y < size_y; y++) {
        putchar (2);               // indicates a new row
        for (x = 0; x < size_x; x++) {
            int index;
            index = (size_x * y * 3) + (x * 3);
            putchar (img[index]);  // Sends Red value
            putchar (img[index + 1]);      // Sends Green Value
            putchar (img[index + 2]);      // Sends Blue Value
        }
    }
    putchar (3);                   // end of frame

}

void InitialiseI2C (void)
{

    REG (I2C_I2CONCLR) = 0xFF;

    // Set pinouts as scl and sda
    REG (PCB_PINSEL0) = 0x50;

    REG (I2C_I2CONSET) = 0x40;  // I2C master 

    REG (I2C_I2CONSET) = 0x64;

    REG (I2C_I2DAT) = 0x42;

    REG (I2C_I2CONCLR) = 0x08;

    REG (I2C_I2CONCLR) = 0x20;
}

void delay (void)
{
    int x;
    for (x = 0; x < 10000; x++);
}

/*
 * void FRAME_READ_INC() { //while(REG(GPIO_IOPIN)&0x8000);
 * REG(GPIO_IOSET)=0x800; REG(GPIO_IOCLR)=0x800; }
 */



void fifo_load_frame ()
{

    unsigned int x, i;
    // REG(GPIO_IOCLR)=CAM_IE; 
    // while(frame_done!=1);
    fifo_read_reset ();
    fifo_write_reset ();
    while (!(REG (GPIO_IOPIN) & CAM_VSYNC));    // while(CAM_VSYNC);
    while (REG (GPIO_IOPIN) & CAM_VSYNC);       // while(!CAM_VSYNC);

    REG (GPIO_IOSET) = BUF_WEE;

    // wait for vsync to finish
    while (!(REG (GPIO_IOPIN) & CAM_VSYNC));    // while(CAM_VSYNC);

    enable_ext_interrupt ();

    for (i = 0; i < 3; i++) {
        while (!(REG (GPIO_IOPIN) & CAM_HREF));
        while (REG (GPIO_IOPIN) & CAM_HREF);
    }

    putchar ('E');
    // delay();
    // REG(GPIO_IOCLR)=BUF_WEE; //BUF_WEE=0
}

void fifo_reset ()
{
    REG (GPIO_IOCLR) = BUF_RESET;
    delay_us_4 (1);
    REG (GPIO_IOSET) = BUF_RESET;

    REG (GPIO_IOCLR) = BUF_WEE;
    REG (GPIO_IOCLR) = BUF_WRST;
    REG (GPIO_IOCLR) = BUF_RRST;
    REG (GPIO_IOCLR) = BUF_RCK;
    REG (GPIO_IOSET) = BUF_RCK;
    REG (GPIO_IOCLR) = BUF_RCK;
    delay_us_4 (1);
    REG (GPIO_IOSET) = BUF_WRST;
    REG (GPIO_IOSET) = BUF_RRST;

}

void fifo_write_reset ()
{
    REG (GPIO_IOCLR) = BUF_WEE;
    REG (GPIO_IOCLR) = BUF_WRST;

    delay_us_4 (1);

    REG (GPIO_IOSET) = BUF_WRST;
}

void fifo_read_reset ()
{
    REG (GPIO_IOCLR) = BUF_RRST;
    REG (GPIO_IOCLR) = BUF_RCK;
    REG (GPIO_IOSET) = BUF_RCK;
    REG (GPIO_IOCLR) = BUF_RCK;
    REG (GPIO_IOSET) = BUF_RRST;
}


void fifo_read_pixel ()
{
    // if(hiresMode && !frame_dump) frame_skip_pixel();
    green_pixel = REG (GPIO_IOPIN);
    FIFO_READ_INC ();
    red_pixel = REG (GPIO_IOPIN);
    FIFO_READ_INC ();
    green_pixel2 = REG (GPIO_IOPIN);
    FIFO_READ_INC ();
    blue_pixel = REG (GPIO_IOPIN);
    FIFO_READ_INC ();

    red_pixel >>= 24;
    green_pixel >>= 24;
    blue_pixel >>= 24;


    // Help combat bad contacts in the bus? and because if you take them
    // away the compiler breaks
    // If you remove this, the frame rate goes up on the ov6620 while the
    // ov7620 could have problems
    /*
     * if(red_pixel<16) red_pixel=16; if(red_pixel>240) red_pixel=240;
     * if(green_pixel<16) green_pixel=16; if(green_pixel>240)
     * green_pixel=240; if(blue_pixel<16) blue_pixel=16;
     * if(blue_pixel>240) blue_pixel=240; 
     */
    // if(CAM_VSYNC && !buffer_mode )frame_write_reset();

}

void fifo_skip_pixel ()
{
    FIFO_READ_INC ();
    FIFO_READ_INC ();
    FIFO_READ_INC ();
    FIFO_READ_INC ();
}



void delay_i2c ()
{
    int x;
    // make about 4 us
    for (x = 0; x < 1000; x++);
}

void delay_us_4 (int cnt)
{
    int i, x;
    for (i = 0; i < cnt; i++)
        for (x = 0; x < 10; x++);
}

void set_cam_ddr_i2c_idle ()
{
    // DDR(I2C_PORT,I2C_PORT_DDR_IDLE);
    REG (GPIO_IODIR) = I2C_PORT_DDR_IDLE;
    delay_i2c ();

}

void set_cam_ddr_i2c_write ()
{
    // DDR(I2C_PORT,I2C_PORT_DDR_WRITE);
    REG (GPIO_IODIR) = I2C_PORT_DDR_WRITE;
    delay_i2c ();
}


void set_cam_ddr (volatile unsigned long val)
{
    // DDR(I2C_PORT,val);
    REG (GPIO_IODIR) = val;
    delay_i2c ();
}


unsigned int i2c_send (unsigned int num, unsigned int *buffer)
{
    unsigned int ack, i, k;
    unsigned int data;

    // Send Start Bit
    // I2C_SDA=0; // needed because values can be reset by read-modify
    // cycle
    REG (GPIO_IOCLR) = 0x00800000;
    set_cam_ddr (I2C_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
    // I2C_SCL=0; // needed because values can be reset by read-modify
    // cycle
    REG (GPIO_IOCLR) = 0x00400000;
    set_cam_ddr_i2c_write ();   // SDA=0 SCL=0

    // Send the Byte 
    for (k = 0; k != num; k++) {
        data = buffer[k];       // To avoid shifting array problems 
        for (i = 0; !(i & 8); i++)      // Write data 
        {
            if (data & 0x80) {
                set_cam_ddr (I2C_PORT_DDR_READ_SDA);    // SDA=1 SCL=0
                set_cam_ddr_i2c_idle ();        // SDA=1 SCL=1
            }
            else {
                set_cam_ddr_i2c_write ();       // SDA=0 SCL=0
                set_cam_ddr (I2C_PORT_DDR_READ_SCL);    // SDA=0 SCL=1

            }
            while (!(REG (GPIO_IOPIN) & 0x00400000));
            // while(!I2C_SCL);


            if (data & 0x08) {
                set_cam_ddr (I2C_PORT_DDR_READ_SDA);    // SDA=1 SCL=0

            }
            else {
                set_cam_ddr_i2c_write ();       // SDA=0 SCL=0
            }

            data <<= 1;
        }                       // END OF 8 BIT FOR LOOP

        // Check ACK <*************************************
        set_cam_ddr (I2C_PORT_DDR_READ_SDA);    // SDA=1 SCL=0

        set_cam_ddr_i2c_idle ();        // SDA=1 SCL=1
        ack = 0;

        // if(I2C_SDA) // sample SDA
        if (REG (GPIO_IOPIN) & 0x00800000) {
            ack |= 1;
            break;
        }

        set_cam_ddr_i2c_write ();       // SDA=0 SCL=0

    }

    // Send Stop Bit 
    set_cam_ddr (I2C_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
    set_cam_ddr_i2c_idle ();    // SDA=1 SCL=1

    return ack;

}
