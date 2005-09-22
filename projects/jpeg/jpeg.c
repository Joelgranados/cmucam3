//#include <math.h>
//#include "tables.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "serial.h"

#define IMG_X   176
#define IMG_Y   288
//#define IMG_X   320
//#define IMG_Y	480
#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS INT32 shift_temp;
#define RIGHT_SHIFT(x,shft) \
    ((shift_temp = (x)) < 0 ? \
    (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
    (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif

#define JPEG_INTERNALS
#define EXCLUDE_DECODER
#define cmu_cam_board_buffer_size  120
char frame_done;
//unsigned char img[IMG_Y][IMG_X][3];	// Store like this for best pointer stride
unsigned char img[8][IMG_X][3];                   // 8 lines of an image read from the FIFO

// TODO: NO GLOBAL INITIALIZATION ON GNU ARM!  Fix.
//const int cmu_cam_board_buffer_size = 120;        // WARNING: We really only have 119 bytes of buffer because of the way send_network_packet(const int frame_number) was written
const int txs_debug = 0;

unsigned char outbuf[cmu_cam_board_buffer_size];  // Our output buffer for ENCODER
unsigned char network_packet[80];                 // This is what we will store data to send over the network.
unsigned char dec_outbuf[1000];                   // Our output buffer for DECODER

int last_network_packet_start_bit;
int last_network_packet_start_byte;
int amount_of_data_in_network_packet_from_boundary_edge_bits;


#ifndef EXCLUDE_DECODER
// Decoder only 
int dec_bit_pos;
int dec_byte_pos;                                 // For the decoder ONLY
int dec_last_DC_value;
int dec_last_DC_value_from_first_column;
int dec_last_chr_DC_value;
int dec_last_chr_DC_value_from_first_column;
int valid_dec_bit_pos;                            // This is so we know how much data is in the buffer!  Don't read invalid data!
int valid_dec_byte_pos;
float img_Y_left1[8][8]; 
float img_Y_left2[8][8]; 
float img_Y_left3[8][8]; 
float img_Y_right1[8][8]; 
float img_Y_right2[8][8]; 
float img_Y_right3[8][8]; 
float img_Cr_1[8][8]; 
float img_Cr_2[8][8]; 
float img_Cr_3[8][8]; 
float img_Cb_1[8][8]; 
float img_Cb_2[8][8]; 
float img_Cb_3[8][8]; 
int blocks_found_in_network_packet; 
int block_position; 
#endif 


uint8_t img_Y[8][88*2];
uint8_t img_Cr[8][88];
uint8_t img_Cb[8][88];

void load_line()
{
uint8_t i,j;

		
for(i=0; i<8; i++ )
	for(j=0; j<88; j++ )
	{
	   cc3_pixbuf_read ();
           img_Y[i][2*j]= cc3_g_current_pixel.channel[CC3_Y];
           img_Y[i][2*j+1]= cc3_g_current_pixel.channel[CC3_Y2];
           img_Cr[i][j]= cc3_g_current_pixel.channel[CC3_CR];
           img_Cb[i][j]= cc3_g_current_pixel.channel[CC3_CB];
	}
}


int bit_pos;
int byte_pos;
int last_DC_value;
int last_DC_value_from_first_column;
int chr_last_DC_value;
int chr_last_DC_value_from_first_column;

int total_stuffed;                                // GUI ONLY


#ifndef EXCLUDE_DECODER
coeff_table lum_AC_table(162,"AC luminance (VLC)");
coeff_table lum_DC_table(14,"DC luminance");
coeff_table chr_AC_table(162,"AC chrominance (VLC)");
coeff_table chr_DC_table(14,"DC chrominance");
#endif

// TODO: GUI ONLY.  GLOBALS WILL BREAK GNUARM!!!
//       REMOVE ALL OF THESE DECLARATIONS BELOW AND PUT INTO THE MAIN FUNCTION FOR THE CAMERA
const int luminance_quant_bit_shift_table[8][8] =
  {
    4,       3,        3,       4,       5,       5,      6,        6,
    3,       3,        3,       4,       4,       6,      6,        6,
    4,       3,        4,       4,       5,       6,      6,        6,
    4,       4,        4,       5,       6,       6,      6,        6,
    4,       4,        5,       6,       6,       7,      7,        6,
    4,       5,        6,       6,       6,       7,      7,        6,
    6,       6,        6,       6,       7,       7,      7,        7,
    6,       7,        7,       7,       7,       7,      7,        6
  };

    const int chrominance_quant_bit_shift_table[8][8] =
    {
        4,      4,       4,       6,      7,      7,     7,       7,
        4,      4,       4,       6,      7,      7,     7,       7,
        4,      4,       6,       7,      7,      7,     7,       7,
        6,      6,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7
    };

    const float DCT_coeffs[8][8] =
    {
        0.3536,    0.4904,    0.4619,    0.4157,    0.3536,    0.2778,    0.1913,    0.0975,
        0.3536,    0.4157,    0.1913,   -0.0975,   -0.3536,   -0.4904,   -0.4619,   -0.2778,
        0.3536,    0.2778,   -0.1913,   -0.4904,   -0.3536,    0.0975,    0.4619,    0.4157,
        0.3536,    0.0975,   -0.4619,   -0.2778,    0.3536,    0.4157,   -0.1913,   -0.4904,
        0.3536,   -0.0975,   -0.4619,    0.2778,    0.3536,   -0.4157,   -0.1913,    0.4904,
        0.3536,   -0.2778,   -0.1913,    0.4904,   -0.3536,   -0.0975,    0.4619,   -0.4157,
        0.3536,   -0.4157,    0.1913,    0.0975,   -0.3536,    0.4904,   -0.4619,    0.2778,
        0.3536,   -0.4904,    0.4619,   -0.4157,    0.3536,   -0.2778,    0.1913,   -0.0975
    };

    const float DCT_coeffs_trans[8][8] =
    {
        0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,
        0.4904,    0.4157,    0.2778,    0.0975,   -0.0975,   -0.2778,   -0.4157,   -0.4904,
        0.4619,    0.1913,   -0.1913,   -0.4619,   -0.4619,   -0.1913,    0.1913,    0.4619,
        0.4157,   -0.0975,   -0.4904,   -0.2778,    0.2778,    0.4904,    0.0975,   -0.4157,
        0.3536,   -0.3536,   -0.3536,    0.3536,    0.3536,   -0.3536,   -0.3536,    0.3536,
        0.2778,   -0.4904,    0.0975,    0.4157,   -0.4157,   -0.0975,    0.4904,   -0.2778,
        0.1913,   -0.4619,    0.4619,   -0.1913,   -0.1913,    0.4619,   -0.4619,    0.1913,
        0.0975,   -0.2778,    0.4157,   -0.4904,    0.4904,   -0.4157,    0.2778,   -0.0975
	}; 

    float input[8][8] =                     // Let's try some real data from my "Kell Bell" image
    {
        125,  120,  110,  111,  120,  124,  131,  149,
        126,  121,  115,  117,  120,  120,  125,  142,
        126,  123,  116,  121,  126,  126,  127,  140,
        127,  129,  126,  127,  133,  139,  139,  144,
        129,  138,  133,  139,  146,  148,  164,  152,
        152,  141,  137,  139,  146,  152,  156,  150,
        141,  132,  132,  134,  143,  146,  156,  150,
        119,  120,  131,  137,  146,  157,  155,  151
    };

    float input2[8][8] =
    {
        240,  234,  232,  228,  225,  215,  202,  181,
        238,  235,  232,  233,  223,  209,  197,  165,
        236,  232,  232,  235,  213,  209,  190,  155,
        232,  233,  234,  224,  212,  199,  177,  146,
        229,  229,  232,  222,  208,  195,  168,  136,
        227,  225,  220,  218,  202,  185,  152,  125,
        224,  218,  213,  211,  191,  173,  141,  113,
        227,  215,  214,  203,  186,  159,  128,  100
    };

inline void stuff_bits(const int num_bits_huffman, const unsigned int huff_value)
{
    static unsigned int temp = 0;                 // Be careful with how we handle this so the static doesn't haunt us.
    static unsigned int bits_to_go = 0;
    static unsigned int iter = 0;

    // GUI ONLY
    //total_stuffed += num_bits_huffman;
    //printf("\tStuffing %02d bits \n\ttotal = %04d\n\tvalue = %d, \n\tstarting at <bit_pos, byte_pos> = <%d,%d>\n\n", num_bits_huffman, total_stuffed, huff_value, bit_pos, byte_pos);
    // END GUI ONLY

    // let's handle the huffman code (size) first
    if(num_bits_huffman <= bit_pos + 1)           // The size will not cross a byte boundary
    {
        outbuf[byte_pos] = outbuf[byte_pos] | (huff_value << (bit_pos - num_bits_huffman + 1));
        bit_pos = bit_pos - num_bits_huffman;
        if(bit_pos == -1)                         // This will happen when the huffman fills the entire block up.
        {
            bit_pos = 7;
            byte_pos++;
        }
    }
    else                                          // The size WILL cross a byte boundary.  rats.
    {                                             // We only have bit_pos + 1 bits that we can fit into here.
                                                  // Let's grab only the highest bits that we can stuff into the current byte
        temp = huff_value >> (num_bits_huffman - bit_pos - 1);
        outbuf[byte_pos] = outbuf[byte_pos] | temp;
                                                  // WARNING: Order here is critical.
        bits_to_go = num_bits_huffman - bit_pos - 1;
        byte_pos++;

        // Notice the setting of bit_pos is implicit here to be 7
        // We do not need it for this next while loop.  We will need it afterwards though.  We do not change bit_pos so that we still have it for a reference...
        iter = 1;                                 // Logically should be zero.  But it is factored into the calculation below (saves 2 cycles)
        while(bits_to_go > 8)                     // Take whole bytes at a time.
        {
            outbuf[byte_pos] = (huff_value & ( 255 << (num_bits_huffman - 8 *(iter) - (bit_pos + 1)))) >> ((num_bits_huffman - 8 *(iter) - (bit_pos + 1)));
            byte_pos++;
            iter++;
            bits_to_go -= 8;
        }
        // Finish the last few bits (if any)
        temp = (1 << bits_to_go) - 1;
        temp = huff_value & temp;
                                                  // We don't have to 'AND' it because these are the first elements in the byte!
        outbuf[byte_pos] = temp << (8 - bits_to_go);
        bit_pos = 7 - bits_to_go;                 // Now we fix it.  Thank you for your services while you were around bit_pos.

    }

    // We have gotten ourselves within 100 bytes of the end!  Move before we get in trouble!
    //const int flush_buf_limit = cmu_cam_board_buffer_size - 100;

    //if(byte_pos >= flush_buf_limit)               // TODO: How often should we send a chunk of data?
    //{
    //    memmove(&outbuf[0], &outbuf[flush_buf_limit], byte_pos-flush_buf_limit+1);
    //    memset(&outbuf[byte_pos-flush_buf_limit+1], 0, flush_buf_limit);
    //    byte_pos -= flush_buf_limit;
    //    printf("WATCH OUT! BYTES HAVE BEEN SENT OVER THE NETWORK!\n\r");
    //}
}


inline void add_bits_to_stream(const int num_bits_huffman, const unsigned int huff_value, const int num_bits_data, const unsigned int data_value, const unsigned int sign_bit)
{                                                 // WARNING: THE SIGN BIT IS NOT INCLUDED IN THE NUMBER OF BITS
    stuff_bits(num_bits_huffman, huff_value);
    stuff_bits(num_bits_data,    data_value);
    stuff_bits(            1,      sign_bit);
}


inline void add_VLC_to_stream(const int bits, const unsigned int code_word, const unsigned int sign)
{
    stuff_bits(bits, code_word);
    stuff_bits(   1,      sign);
    //    printf("Adding the VLC <length, code, sign> = <%d,%d,%d>\n\r", bits, code_word, sign);
}


inline void add_DC_coefficient_to_stream(const int DC_coeff_value, const int is_in_first_column, const int is_luminance)
{
    if(is_in_first_column)
        printf("Adding first row DC coefficient:  %d\n", DC_coeff_value);
    else
        printf("Adding DC coefficient: %d \n", DC_coeff_value);

    // This is the table we will use for the DC coefficients
    // size ->  Huffman code  --> bits to follow  --> abs values --> examples
    //    0 ->  00                   none				0 			 0 = 00
    //    1 ->  010                     s				1			 1 = 010 0		-1 = 010 1
    //    2 ->  011                    Xs				2:3			 3 = 011 10    -2 = 011 01		-3 = 011 11
    //    3 ->  100                   XXs               4:7
    //    4 ->  101                  XXXs               8:15
    //    5 ->  110                 XXXXs               16:31
    //    6 ->  1110               XXXXXs               32:63
    //    7 ->  11110             XXXXXXs               64:127
    //    8 ->  111110           XXXXXXXs               128:255
    //    9 ->  1111110         XXXXXXXXs               256:511
    //    10 -> 11111110       XXXXXXXXXs               512:1023
    //    11 -> 111111110     XXXXXXXXXXs               1024:2047
    //    XX -> 1*(XX-3)0         (XX-1)s 	            2^(size-1):2^(size)-1
    static int diff;
    static int sign_bit = 0;
    static int chr_diff;
    static int chr_sign_bit = 0;

    if(is_luminance)
    {
        if(is_in_first_column)
        {
            printf("We are using the first column special case.  This value is %d\n", last_DC_value_from_first_column);
                                                  // This will be our differential DC_coefficient.  The first will be diffed with zero.
            diff = DC_coeff_value - last_DC_value_from_first_column;
            last_DC_value = DC_coeff_value;       // Keep it updated for the next iteration
            last_DC_value_from_first_column = DC_coeff_value;
        }
        else
        {
            diff = DC_coeff_value - last_DC_value;// This will be our differential DC_coefficient.  The first will be diffed with zero.
            last_DC_value = DC_coeff_value;
        }

        // TODO: Can we improve this with a hash map?
        if(diff < 0)
        {
            sign_bit = 1;
            diff = -diff;
        }
        else
            sign_bit = 0;

        //printf("Our luminace diff = %d\n\r", diff);

        //inline void add_bits_to_stream(int num_bits_huffman, int value, int num_bits_data, int value, int sign_bit)
        if(diff == 0)                             // size = 0  [0]
                                                  // WARNING: THE ZERO CASE IS SPECIAL BECAUSE IT DOES NOT HAVE A SIGN BIT
            add_bits_to_stream(1,         0,    0,           0,     0);
                                                  //          WE WILL INCORPORATE THE LAST ZERO IN THE HUFFMAN AS THE SIGN
        else if(diff < 2)                         // size = 1  [1]
            add_bits_to_stream(3,         2,    0,           0,     sign_bit);
        else if(diff < 4)                         // size = 2  [2,3]
            add_bits_to_stream(3,         3,    1,  diff &   1,         sign_bit);
        else if(diff < 8)                         // size = 3  [4,7]
            add_bits_to_stream(3,         4,    2,  diff &   3,     sign_bit);
        else if(diff < 16)                        // size = 4  [8,15]
            add_bits_to_stream(3,         5,    3,  diff &   7,     sign_bit);
        else if(diff < 32)
            add_bits_to_stream(3,         6,    4,  diff &  15,     sign_bit);
        else if(diff < 64)
            add_bits_to_stream(4,        14,    5,  diff &  31,     sign_bit);
        else if(diff < 128)
            add_bits_to_stream(5,        30,    6,  diff &  63,     sign_bit);
        else if(diff < 256)
            add_bits_to_stream(6,        62,    7,  diff & 127,     sign_bit);
        else if(diff < 512)
            add_bits_to_stream(7,       126,    8,  diff & 255,     sign_bit);
        else if(diff <1024)
            add_bits_to_stream(8,       254,    9,  diff & 511,     sign_bit);
        else if(diff <2048)
            add_bits_to_stream(9,       510,   10,  diff &1023,     sign_bit);
        else if(diff <4096)
            add_bits_to_stream(10,     1022,   11,  diff &2047,     sign_bit);
        else if(diff <8192)
            add_bits_to_stream(11,     2046,   12,  diff &4095,     sign_bit);
        else
        {
            printf("ERROR: The code should never be here! Impossible.  Something is wrong with the code!");
            //exit(0);
        }
    }
    else                                          // OK, we are dealing with the chrominance and will use the chrominance table for DC coefficients
    {
        if(is_in_first_column)
        {
                                                  // This will be our differential DC_coefficient.  The first will be diffed with zero.
            printf("We are using the first column special case for the chrominance.  This value is %d\n", chr_last_DC_value_from_first_column);
            chr_diff = DC_coeff_value - chr_last_DC_value_from_first_column;
            chr_last_DC_value = DC_coeff_value;   // Keep it updated for the next iteration
            chr_last_DC_value_from_first_column = DC_coeff_value;
        }
        else
        {
                                                  // This will be our differential DC_coefficient.  The first will be diffed with zero.
            chr_diff = DC_coeff_value - chr_last_DC_value;
            chr_last_DC_value = DC_coeff_value;
        }

        // TODO: Can we improve this with a hash map?
        if(chr_diff < 0)
        {
            chr_sign_bit = 1;
            chr_diff = -chr_diff;
        }
        else
            chr_sign_bit = 0;
        //printf("Our chrominance diff = %d\n\r", chr_diff);
        //inline void add_bits_to_stream(int num_bits_huffman, int value, int num_bits_data, int value, int sign_bit)
        if(chr_diff == 0)                         // size = 0  [0]
                                                  // WARNING: THE ZERO CASE IS SPECIAL BECAUSE IT DOES NOT HAVE A SIGN BIT
            add_bits_to_stream(1,         0,    0,           0,     0);
                                                  //          WE WILL INCORPORATE THE LAST ZERO IN THE HUFFMAN AS THE SIGN
        else if(chr_diff < 2)                     // size = 1  [1]
            add_bits_to_stream(2,         1,    0,           0,     chr_sign_bit);
        else if(chr_diff < 4)                     // size = 2  [2,3]
            add_bits_to_stream(2,         2,    1,  chr_diff &   1,     chr_sign_bit);
        else if(chr_diff < 8)                     // size = 3  [4,7]
            add_bits_to_stream(3,         6,    2,  chr_diff &   3,     chr_sign_bit);
        else if(chr_diff < 16)                    // size = 4  [8,15]
            add_bits_to_stream(4,        14,    3,  chr_diff &   7,     chr_sign_bit);
        else if(chr_diff < 32)
            add_bits_to_stream(5,        30,    4,  chr_diff &  15,     chr_sign_bit);
        else if(chr_diff < 64)
            add_bits_to_stream(6,        62,    5,  chr_diff &  31,     chr_sign_bit);
        else if(chr_diff < 128)
            add_bits_to_stream(7,       126,    6,  chr_diff &  63,     chr_sign_bit);
        else if(chr_diff < 256)
            add_bits_to_stream(8,       254,    7,  chr_diff & 127,     chr_sign_bit);
        else if(chr_diff < 512)
            add_bits_to_stream(9,       510,    8,  chr_diff & 255,     chr_sign_bit);
        else if(chr_diff <1024)
            add_bits_to_stream(10,     1022,    9,  chr_diff & 511,     chr_sign_bit);
        else if(chr_diff <2048)
            add_bits_to_stream(11,     2046,   10,  chr_diff &1023,     chr_sign_bit);
        else if(chr_diff <4096)
            add_bits_to_stream(12,     4094,   11,  chr_diff &2047,     chr_sign_bit);
        else if(chr_diff <8192)
            add_bits_to_stream(13,     8190,   12,  chr_diff &4095,     chr_sign_bit);
        else
        {
            printf("ERROR: The code should never be here! Impossible.  Something is wrong with the code!");
            //exit(0);
        }
    }
}


                                                  // This will add the VLC components for the AC coefficient encoding
inline void add_RL_entry(const int run, const int raw_level, const int is_luminance)
{
    /* OK, here is our table.  Remember, the length is important because we can have strings that start with '0'.  In which case, the leading digit can not be ascertained from
       the decimal number.  Here is the massive table:
         Run / Level / Bits / Decimal number
         0 0 4 10
         0 1 2 0
         0 2 2 1
         0 3 3 4
         0 4 4 11
         0 5 5 26
         0 6 7 120
         0 7 8 248
    0 8 10 1014
    0 9 16 65410
    0 A 16 65411
    1 1 4 12
    1 2 5 27
    1 3 7 121
    1 4 9 502
    1 5 11 2038
    1 6 16 65412
    1 7 16 65413
    1 8 16 65414
    1 9 16 65415
    1 A 16 65416
    2 1 5 28
    2 2 8 249
    2 3 10 1015
    2 4 12 4084
    2 5 16 65417
    2 6 16 65418
    2 7 16 65419
    2 8 16 65420
    2 9 16 65421
    2 A 16 65422
    3 1 6 58
    3 2 9 503
    3 3 12 4085
    3 4 16 65423
    3 5 16 65424
    3 6 16 65425
    3 7 16 65426
    3 8 16 65427
    3 9 16 65428
    3 A 16 65429
    4 1 6 59
    4 2 10 1016
    4 3 16 65430
    4 4 16 65431
    4 5 16 65432
    4 6 16 65433
    4 7 16 65434
    4 8 16 65435
    4 9 16 65436
    4 A 16 65437
    5 1 7 123
    5 2 12 4086
    5 3 16 65438
    5 4 16 65439
    5 5 16 65440
    5 6 16 65441
    5 7 16 65442
    5 8 16 65443
    5 9 16 65444
    5 A 16 65445
    6 1 7 122
    6 2 11 2039
    6 3 16 65446
    6 4 16 65447
    6 5 16 65448
    6 6 16 65449
    6 7 16 65450
    6 8 16 65451
    6 9 16 65452
    6 A 16 65453
    7 1 8 250
    7 2 12 4087
    7 3 16 65454
    7 4 16 65455
    7 5 16 65456
    7 6 16 65457
    7 7 16 65458
    7 8 16 65459
    7 9 16 65460
    7 A 16 65461
    8 1 9 504
    8 2 15 32704
    8 3 16 65462
    8 4 16 65463
    8 5 16 65464
    8 6 16 65465
    8 7 16 65466
    8 8 16 65467
    8 9 16 65468
    8 A 16 65469
    9 1 9 505
    9 2 16 65470
    9 3 16 65471
    9 4 16 65472
    9 5 16 65473
    9 6 16 65474
    9 7 16 65475
    9 8 16 65476
    9 9 16 65477
    9 A 16 65478
    A 1 9 506
    A 2 16 65479
    A 3 16 65480
    A 4 16 65481
    A 5 16 65482
    A 6 16 65483
    A 7 16 65484
    A 8 16 65485
    A 9 16 65486
    A A 16 65487
    B 1 10 1017
    B 2 16 65488
    B 3 16 65489
    B 4 16 65490
    B 5 16 65491
    B 6 16 65492
    B 7 16 65493
    B 8 16 65494
    B 9 16 65495
    B A 16 65496
    C 1 10 1018
    C 2 16 65497
    C 3 16 65498
    C 4 16 65499
    C 5 16 65500
    C 6 16 65501
    C 7 16 65502
    C 8 16 65503
    C 9 16 65504
    C A 16 65505
    D 1 11 2040
    D 2 16 65506
    D 3 16 65507
    D 4 16 65508
    D 5 16 65509
    D 6 16 65510
    D 7 16 65511
    D 8 16 65512
    D 9 16 65513
    D A 16 65514
    E 1 16 65515
    E 2 16 65516
    E 3 16 65517
    E 4 16 65518
    E 5 16 65519
    E 6 16 65520
    E 7 16 65521
    E 8 16 65522
    E 9 16 65523
    E A 16 65524
    F 0 11 2041
    F 1 16 65525
    F 2 16 65526
    F 3 16 65527
    F 4 16 65528
    F 5 16 65529
    F 6 16 65530
    F 7 16 65531
    F 8 16 65532
    F 9 16 65533
    F A 16 65534
    F A 16 65534
    */

    int sign = 0;
    int level = 0;

    if(raw_level < 0)
    {
        level = -raw_level;
        sign = 1;
    }
    else
    {
        level = raw_level;
        sign = 0;
    }
   
    if(level > 10)
    {
        level = 10;
        printf("WARNING: We needed to clip one of the levels (1)\n");
    } //printf("Adding the entry with <run, level, raw_level> = <%d,%d,%d>\n\r",run, level, raw_level);

    if(is_luminance)
    {
        switch(run)
        {
            case 0:
                // 		0 0 4 10
                //		0 1 2 0
                //		0 2 2 1
                //		0 3 3 4
                //		0 4 4 11
                //		0 5 5 26
                //		0 6 7 120
                //		0 7 8 248
                //		0 8 10 1014
                //		0 9 16 65410
                //		0 A 16 65411
                switch(level)                     // run = 0
                {
                    case 1:
                        add_VLC_to_stream(2,0,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(2,1,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(3,4,sign);
                        break;
                    case 0:                       // This is the EOB statement.  It is out of sequence because it is less probable than cases 1,2 and 3
                                                  // This is also a strange guy, because there is no sign bit.
                        //add_VLC_to_stream(4,10,sign);  // Really, it should be this without the sign bit.  But we can't do it like this without massive confusion.
                        add_VLC_to_stream(3,5,0); // Let's trick the bit writer.  Same effect.
                        break;
                    case 4:
                        add_VLC_to_stream(4,11,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(5,26,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(7,120,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(8,248,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(10,1014,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65410,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65411,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 1:
                switch(level)                     // run = 1
                {
                    case 1:
                        add_VLC_to_stream(4,12,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(5,27,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(7,121,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(9,502,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(11,2038,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65412,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65413,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65414,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65415,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65416,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 2:
                switch(level)                     // run = 2
                {
                    case 1:
                        add_VLC_to_stream(5,28,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(8,249,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(10,1015,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(12,4084,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65417,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65418,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65419,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65420,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65421,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65422,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 3:
                switch(level)                     // run = 3
                {
                    case 1:
                        add_VLC_to_stream(6,58,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(9,503,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(12,4085,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65423,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65424,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65425,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65426,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65427,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65428,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65429,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 4:
                switch(level)                     // run = 4
                {
                    case 1:
                        add_VLC_to_stream(6,59,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(10,1016,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65430,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65431,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65432,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65433,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65434,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65435,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65436,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65437,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 5:
                switch(level)                     // run = 5
                {
                    case 1:
                        add_VLC_to_stream(7,123,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(12,4086,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65438,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65439,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65440,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65441,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65442,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65443,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65444,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65445,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 6:
                switch(level)                     // run = 6
                {
                    case 1:
                        add_VLC_to_stream(7,122,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(11,2039,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65446,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65447,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65448,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65449,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65450,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65451,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65452,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65453,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 7:
                switch(level)                     // run = 7
                {
                    case 1:
                        add_VLC_to_stream(8,250,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(12,4087,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65454,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65455,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65456,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65457,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65458,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65459,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65460,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65461,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 8:
                switch(level)                     // run = 8
                {
                    case 1:
                        add_VLC_to_stream(9,504,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(15,32704,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65462,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65463,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65464,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65465,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65466,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65467,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65468,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65469,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 9:
                switch(level)                     // run = 9
                {
                    case 1:
                        add_VLC_to_stream(9,505,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65470,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65471,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65472,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65473,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65474,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65475,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65476,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65477,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65478,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 0xA:
                switch(level)                     // run = A
                {
                    case 1:
                        add_VLC_to_stream(9,506,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65479,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65480,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65481,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65482,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65483,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65484,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65485,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65486,sign);
                        break;
                    case 10:
                        add_VLC_to_stream(16,65487,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 0xB:
                switch(level)                     // run = B
                {
                    case 1:
                        add_VLC_to_stream(10,1017,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65488,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65489,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65490,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65491,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65492,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65493,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65494,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65495,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65496,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 0xC:
                switch(level)                     // run = C
                {
                    case 1:
                        add_VLC_to_stream(10,1018,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65497,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65498,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65499,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65500,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65501,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65502,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65503,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65504,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65505,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 0xD:
                switch(level)                     // run = D
                {
                    case 1:
                        add_VLC_to_stream(11,2040,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65506,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65507,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65508,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65509,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65510,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65511,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65512,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65513,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65514,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            case 0xE:
                switch(level)                     // run = E
                {
                    case 1:
                        add_VLC_to_stream(16,65515,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65516,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65517,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65518,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65519,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65520,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65521,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65522,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65523,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65524,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;
            case 0xF:                             // The ZRL may be more common than some of the more lengthy runs.  Not sure, but let's put it here to see if we can save a few 'if' statements
                switch(level)                     // run = F
                {
                    case 0:
                        //add_VLC_to_stream(11,2041,sign);
                        add_VLC_to_stream(10,1020,1);// The hacked verzion to protect the sign bit.
                        break;
                    case 1:
                        add_VLC_to_stream(16,65525,sign);
                        break;
                    case 2:
                        add_VLC_to_stream(16,65526,sign);
                        break;
                    case 3:
                        add_VLC_to_stream(16,65527,sign);
                        break;
                    case 4:
                        add_VLC_to_stream(16,65528,sign);
                        break;
                    case 5:
                        add_VLC_to_stream(16,65529,sign);
                        break;
                    case 6:
                        add_VLC_to_stream(16,65530,sign);
                        break;
                    case 7:
                        add_VLC_to_stream(16,65531,sign);
                        break;
                    case 8:
                        add_VLC_to_stream(16,65532,sign);
                        break;
                    case 9:
                        add_VLC_to_stream(16,65533,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65534,sign);
                        break;
                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen! r = %d  l = %d \n\r", run, level);

                }
                break;

            default:
                printf("WE CAN NOT HANDLE THIS IMAGE.  WILL TRY AGAIN LATER (RLE overflow)\n\r");
        }

    }
    else                                          // OK, we are dealing with the chrominance
    {
        switch(run)
        {
            case 0:
                switch(level)                     // run = 0
                {
                    case 0x0:
                        //add_VLC_to_stream(2,0,sign);
                        add_VLC_to_stream(1,0,0); // There is no sign when the value is zero!
                        break;
                    case 0x1:
                        add_VLC_to_stream(2,1,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(3,4,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(4,10,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(5,24,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(5,25,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(6,56,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(7,120,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(9,500,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(10,1014,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(12,4084,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 1:
                switch(level)                     // run = 1
                {
                    case 0x1:
                        add_VLC_to_stream(4,11,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(6,57,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(8,246,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(9,501,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(11,2038,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(12,4085,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65416,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65417,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65418,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65419,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 2:
                switch(level)                     // run = 2
                {
                    case 0x1:
                        add_VLC_to_stream(5,26,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(8,247,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(10,1015,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(12,4086,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(15,32706,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65420,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65421,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65422,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65423,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65424,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 3:
                switch(level)                     // run = 3
                {
                    case 0x1:
                        add_VLC_to_stream(5,27,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(8,248,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(10,1016,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(12,4087,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65425,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65426,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65427,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65428,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65429,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65430,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 4:
                switch(level)                     // run = 4
                {
                    case 0x1:
                        add_VLC_to_stream(6,58,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(9,502,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65431,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65432,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65433,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65434,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65435,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65436,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65437,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65438,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 5:
                switch(level)                     // run = 5
                {
                    case 0x1:
                        add_VLC_to_stream(6,59,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(10,1017,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65439,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65440,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65441,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65442,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65443,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65444,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65445,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65446,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 6:
                switch(level)                     // run = 6
                {
                    case 0x1:
                        add_VLC_to_stream(7,121,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(11,2039,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65447,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65448,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65449,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65450,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65451,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65452,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65453,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65454,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 7:
                switch(level)                     // run = 7
                {
                    case 0x1:
                        add_VLC_to_stream(7,122,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(11,2040,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65455,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65456,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65457,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65458,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65459,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65460,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65461,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65462,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 8:
                switch(level)                     // run = 8
                {
                    case 0x1:
                        add_VLC_to_stream(8,249,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65463,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65464,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65465,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65466,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65467,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65468,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65469,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65470,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65471,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 9:
                switch(level)                     // run = 9
                {
                    case 0x1:
                        add_VLC_to_stream(9,503,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65472,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65473,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65474,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65475,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65476,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65477,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65478,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65479,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65480,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xA:
                switch(level)                     // run = A
                {
                    case 0x1:
                        add_VLC_to_stream(9,504,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65481,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65482,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65483,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65484,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65485,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65486,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65487,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65488,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65489,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xB:
                switch(level)                     // run = B
                {
                    case 0x1:
                        add_VLC_to_stream(9,505,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65490,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65491,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65492,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65493,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65494,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65495,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65496,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65497,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65498,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xC:
                switch(level)                     // run = C
                {
                    case 0x1:
                        add_VLC_to_stream(9,506,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65499,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65500,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65501,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65502,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65503,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65504,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65505,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65506,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65507,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xD:
                switch(level)                     // run = D
                {
                    case 0x1:
                        add_VLC_to_stream(11,2041,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65508,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65509,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65510,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65511,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65512,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65513,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65514,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65515,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65516,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xE:
                switch(level)                     // run = E
                {
                    case 0x1:
                        add_VLC_to_stream(14,16352,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65517,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65518,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65519,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65520,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65521,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65522,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65523,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65524,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65525,sign);
                        break;

                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            case 0xF:
                switch(level)                     // run = F
                {
                    case 0x0:
                        //add_VLC_to_stream(10,1018,sign);
                        //add_VLC_to_stream(10,1018,sign);
                        //add_VLC_to_stream(10,1018,sign);
                        add_VLC_to_stream(9,509,0);
                        break;
                    case 0x1:
                        add_VLC_to_stream(15,32707,sign);
                        break;
                    case 0x2:
                        add_VLC_to_stream(16,65526,sign);
                        break;
                    case 0x3:
                        add_VLC_to_stream(16,65527,sign);
                        break;
                    case 0x4:
                        add_VLC_to_stream(16,65528,sign);
                        break;
                    case 0x5:
                        add_VLC_to_stream(16,65529,sign);
                        break;
                    case 0x6:
                        add_VLC_to_stream(16,65530,sign);
                        break;
                    case 0x7:
                        add_VLC_to_stream(16,65531,sign);
                        break;
                    case 0x8:
                        add_VLC_to_stream(16,65532,sign);
                        break;
                    case 0x9:
                        add_VLC_to_stream(16,65533,sign);
                        break;
                    case 0xA:
                        add_VLC_to_stream(16,65534,sign);
                        break;
                    default:
                        printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");

                }
                break;
            default:
                printf("ERROR! We should never reach this state with the VLC table we have chosen!\n\r");
        }
        //            case 0:
        //add_VLC_to_stream(10,1018,sign);
        //add_VLC_to_stream(10,1018,sign);
        //        add_VLC_to_stream(9,509,0);
        //        break;

    }
}


                                                  // Zig-zag pattern and find RLE entries
inline void add_AC_coefficients_to_stream(const int AC_coeffs[8][8], const int is_luminance)
{
    // We will use Run-Level Encoding (RLE) for the AC coefficients.  One way to do this is do the zig-zag pattern in a 'for' loop.
    // This code will eliminate the for loop be having a single counter which will iterate over all of the pattern.  The 'for'
    // loop would also have a few book keeping variables that we can save on.  Not sure how much faster this will make it.  Here is
    // the zig-zag pattern:

    static int run;
    run = 0;

    //if(AC_coeffs[0][0] != 0)   // coefficient #0  // IGNORE THE DC COEFFICIENT

    if(AC_coeffs[0][1] != 0)                      // coefficient #1
    {
        add_RL_entry(run, AC_coeffs[0][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        //		if(run == 15)          // We can't have a run of 16 after only 1 coefficient!!!
        //		{
        //			add_RL_entry(15, 0);
        //			run = 0;
        //		}
    }

    if(AC_coeffs[1][0] != 0)                      // coefficient #2
    {
        add_RL_entry(run, AC_coeffs[1][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[2][0] != 0)                      // coefficient #3
    {
        add_RL_entry(run, AC_coeffs[2][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[1][1] != 0)                      // coefficient #4
    {
        add_RL_entry(run, AC_coeffs[1][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[0][2] != 0)                      // coefficient #5
    {
        add_RL_entry(run, AC_coeffs[0][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[0][3] != 0)                      // coefficient #6
    {
        add_RL_entry(run, AC_coeffs[0][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[1][2] != 0)                      // coefficient #7
    {
        add_RL_entry(run, AC_coeffs[1][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[2][1] != 0)                      // coefficient #8
    {
        add_RL_entry(run, AC_coeffs[2][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[3][0] != 0)                      // coefficient #9
    {
        add_RL_entry(run, AC_coeffs[3][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[4][0] != 0)                      // coefficient #10
    {
        add_RL_entry(run, AC_coeffs[4][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[3][1] != 0)                      // coefficient #11
    {
        add_RL_entry(run, AC_coeffs[3][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[2][2] != 0)                      // coefficient #12
    {
        add_RL_entry(run, AC_coeffs[2][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[1][3] != 0)                      // coefficient #13
    {
        add_RL_entry(run, AC_coeffs[1][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[0][4] != 0)                      // coefficient #14
    {
        add_RL_entry(run, AC_coeffs[0][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
    }

    if(AC_coeffs[0][5] != 0)                      // coefficient #15
    {
        add_RL_entry(run, AC_coeffs[0][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[1][4] != 0)                      // coefficient #16
    {
        add_RL_entry(run, AC_coeffs[1][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[2][3] != 0)                      // coefficient #17
    {
        add_RL_entry(run, AC_coeffs[2][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][2] != 0)                      // coefficient #18
    {
        add_RL_entry(run, AC_coeffs[3][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][1] != 0)                      // coefficient #19
    {
        add_RL_entry(run, AC_coeffs[4][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][0] != 0)                      // coefficient #20
    {
        add_RL_entry(run, AC_coeffs[5][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][0] != 0)                      // coefficient #21
    {
        add_RL_entry(run, AC_coeffs[6][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][1] != 0)                      // coefficient #22
    {
        add_RL_entry(run, AC_coeffs[5][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][2] != 0)                      // coefficient #23
    {
        add_RL_entry(run, AC_coeffs[4][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][3] != 0)                      // coefficient #24
    {
        add_RL_entry(run, AC_coeffs[3][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[2][4] != 0)                      // coefficient #25
    {
        add_RL_entry(run, AC_coeffs[2][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[1][5] != 0)                      // coefficient #26
    {
        add_RL_entry(run, AC_coeffs[1][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[0][6] != 0)                      // coefficient #27
    {
        add_RL_entry(run, AC_coeffs[0][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[0][7] != 0)                      // coefficient #28
    {
        add_RL_entry(run, AC_coeffs[0][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[1][6] != 0)                      // coefficient #29
    {
        add_RL_entry(run, AC_coeffs[1][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[2][5] != 0)                      // coefficient #30
    {
        add_RL_entry(run, AC_coeffs[2][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][4] != 0)                      // coefficient #31
    {
        add_RL_entry(run, AC_coeffs[3][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][3] != 0)                      // coefficient #32
    {
        add_RL_entry(run, AC_coeffs[4][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][2] != 0)                      // coefficient #33
    {
        add_RL_entry(run, AC_coeffs[5][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][1] != 0)                      // coefficient #34
    {
        add_RL_entry(run, AC_coeffs[6][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][0] != 0)                      // coefficient #35
    {
        add_RL_entry(run, AC_coeffs[7][0], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][1] != 0)                      // coefficient #36
    {
        add_RL_entry(run, AC_coeffs[7][1], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][2] != 0)                      // coefficient #37
    {
        add_RL_entry(run, AC_coeffs[6][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][3] != 0)                      // coefficient #38
    {
        add_RL_entry(run, AC_coeffs[5][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][4] != 0)                      // coefficient #39
    {
        add_RL_entry(run, AC_coeffs[4][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][5] != 0)                      // coefficient #40
    {
        add_RL_entry(run, AC_coeffs[3][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[2][6] != 0)                      // coefficient #41
    {
        add_RL_entry(run, AC_coeffs[2][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[1][7] != 0)                      // coefficient #42
    {
        add_RL_entry(run, AC_coeffs[1][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[2][7] != 0)                      // coefficient #43
    {
        add_RL_entry(run, AC_coeffs[2][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][6] != 0)                      // coefficient #44
    {
        add_RL_entry(run, AC_coeffs[3][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][5] != 0)                      // coefficient #45
    {
        add_RL_entry(run, AC_coeffs[4][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][4] != 0)                      // coefficient #46
    {
        add_RL_entry(run, AC_coeffs[5][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][3] != 0)                      // coefficient #47
    {
        add_RL_entry(run, AC_coeffs[6][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][2] != 0)                      // coefficient #48
    {
        add_RL_entry(run, AC_coeffs[7][2], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][3] != 0)                      // coefficient #49
    {
        add_RL_entry(run, AC_coeffs[7][3], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][4] != 0)                      // coefficient #50
    {
        add_RL_entry(run, AC_coeffs[6][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][5] != 0)                      // coefficient #51
    {
        add_RL_entry(run, AC_coeffs[5][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][6] != 0)                      // coefficient #52
    {
        add_RL_entry(run, AC_coeffs[4][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[3][7] != 0)                      // coefficient #53
    {
        add_RL_entry(run, AC_coeffs[3][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[4][7] != 0)                      // coefficient #54
    {
        add_RL_entry(run, AC_coeffs[4][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][6] != 0)                      // coefficient #55
    {
        add_RL_entry(run, AC_coeffs[5][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][5] != 0)                      // coefficient #56
    {
        add_RL_entry(run, AC_coeffs[6][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][4] != 0)                      // coefficient #57
    {
        add_RL_entry(run, AC_coeffs[7][4], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][5] != 0)                      // coefficient #58
    {
        add_RL_entry(run, AC_coeffs[7][5], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][6] != 0)                      // coefficient #59
    {
        add_RL_entry(run, AC_coeffs[6][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[5][7] != 0)                      // coefficient #60
    {
        add_RL_entry(run, AC_coeffs[5][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[6][7] != 0)                      // coefficient #61
    {
        add_RL_entry(run, AC_coeffs[6][7], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][6] != 0)                      // coefficient #62
    {
        add_RL_entry(run, AC_coeffs[7][6], is_luminance);
        run = 0;
    }
    else
    {
        run = run + 1;
        if(run == 15)
        {
            add_RL_entry(15, 0, is_luminance);
            run = 0;
        }
    }

    if(AC_coeffs[7][7] != 0)                      // coefficient #63
    {
        add_RL_entry(run, AC_coeffs[7][7], is_luminance);
        //		run = 0;               // No need for this anymore.  Let's not waste an assignment
    }
    else
    {
        //		run = run + 1;
        add_RL_entry(0, 0, is_luminance);         // Run 0, Level 0 == EOB
        //		if(run == 15)
        //		{
        //			add_RL_entry(15, 0, is_luminance);
        //			run = 0;
        //		}
    }

}


inline void  const mult_two_mats( float answer[8][8], const float left[8][8], const float  right[8][8])
{
    //	const int print_results = 0;
    static int iii = 0;
    static int jjj = 0;
    static int row_mark = 0;
    memset(&answer[0][0], 0, sizeof(float)*64);   // TODO: Why is this darn memset is needed.
    /*
        memset(&answer[0][0], 0, sizeof(float)*64);
        for(iii = 0; iii < 8; iii++)
        {
            for(jjj = 0; jjj < 8; jjj++)
            {
                for(row_mark = 0; row_mark < 8; row_mark++)
                {
                    answer[iii][jjj] += left[iii][row_mark] * right[row_mark][jjj];
                }

    //			if(print_results == 1)
    //				printf("%d\t",(int)answer[iii][jjj]);

    }
    //		if(print_results == 1)
    //			printf("\r\n");
    }
    */
    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[0][0] = left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0] + left[0][3]*right[3][0] + left[0][4]*right[4][0] + left[0][5]*right[5][0] + left[0][6]*right[6][0] + left[0][7]*right[7][0];
    answer[0][1] = left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1] + left[0][3]*right[3][1] + left[0][4]*right[4][1] + left[0][5]*right[5][1] + left[0][6]*right[6][1] + left[0][7]*right[7][1];
    answer[0][2] = left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2] + left[0][3]*right[3][2] + left[0][4]*right[4][2] + left[0][5]*right[5][2] + left[0][6]*right[6][2] + left[0][7]*right[7][2];
    answer[0][3] = left[0][0]*right[0][3] + left[0][1]*right[1][3] + left[0][2]*right[2][3] + left[0][3]*right[3][3] + left[0][4]*right[4][3] + left[0][5]*right[5][3] + left[0][6]*right[6][3] + left[0][7]*right[7][3];
    answer[0][4] = left[0][0]*right[0][4] + left[0][1]*right[1][4] + left[0][2]*right[2][4] + left[0][3]*right[3][4] + left[0][4]*right[4][4] + left[0][5]*right[5][4] + left[0][6]*right[6][4] + left[0][7]*right[7][4];
    answer[0][5] = left[0][0]*right[0][5] + left[0][1]*right[1][5] + left[0][2]*right[2][5] + left[0][3]*right[3][5] + left[0][4]*right[4][5] + left[0][5]*right[5][5] + left[0][6]*right[6][5] + left[0][7]*right[7][5];
    answer[0][6] = left[0][0]*right[0][6] + left[0][1]*right[1][6] + left[0][2]*right[2][6] + left[0][3]*right[3][6] + left[0][4]*right[4][6] + left[0][5]*right[5][6] + left[0][6]*right[6][6] + left[0][7]*right[7][6];
    answer[0][7] = left[0][0]*right[0][7] + left[0][1]*right[1][7] + left[0][2]*right[2][7] + left[0][3]*right[3][7] + left[0][4]*right[4][7] + left[0][5]*right[5][7] + left[0][6]*right[6][7] + left[0][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[1][0] = left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0] + left[1][3]*right[3][0] + left[1][4]*right[4][0] + left[1][5]*right[5][0] + left[1][6]*right[6][0] + left[1][7]*right[7][0];
    answer[1][1] = left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1] + left[1][3]*right[3][1] + left[1][4]*right[4][1] + left[1][5]*right[5][1] + left[1][6]*right[6][1] + left[1][7]*right[7][1];
    answer[1][2] = left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2] + left[1][3]*right[3][2] + left[1][4]*right[4][2] + left[1][5]*right[5][2] + left[1][6]*right[6][2] + left[1][7]*right[7][2];
    answer[1][3] = left[1][0]*right[0][3] + left[1][1]*right[1][3] + left[1][2]*right[2][3] + left[1][3]*right[3][3] + left[1][4]*right[4][3] + left[1][5]*right[5][3] + left[1][6]*right[6][3] + left[1][7]*right[7][3];
    answer[1][4] = left[1][0]*right[0][4] + left[1][1]*right[1][4] + left[1][2]*right[2][4] + left[1][3]*right[3][4] + left[1][4]*right[4][4] + left[1][5]*right[5][4] + left[1][6]*right[6][4] + left[1][7]*right[7][4];
    answer[1][5] = left[1][0]*right[0][5] + left[1][1]*right[1][5] + left[1][2]*right[2][5] + left[1][3]*right[3][5] + left[1][4]*right[4][5] + left[1][5]*right[5][5] + left[1][6]*right[6][5] + left[1][7]*right[7][5];
    answer[1][6] = left[1][0]*right[0][6] + left[1][1]*right[1][6] + left[1][2]*right[2][6] + left[1][3]*right[3][6] + left[1][4]*right[4][6] + left[1][5]*right[5][6] + left[1][6]*right[6][6] + left[1][7]*right[7][6];
    answer[1][7] = left[1][0]*right[0][7] + left[1][1]*right[1][7] + left[1][2]*right[2][7] + left[1][3]*right[3][7] + left[1][4]*right[4][7] + left[1][5]*right[5][7] + left[1][6]*right[6][7] + left[1][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[2][0] = left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0] + left[2][3]*right[3][0] + left[2][4]*right[4][0] + left[2][5]*right[5][0] + left[2][6]*right[6][0] + left[2][7]*right[7][0];
    answer[2][1] = left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1] + left[2][3]*right[3][1] + left[2][4]*right[4][1] + left[2][5]*right[5][1] + left[2][6]*right[6][1] + left[2][7]*right[7][1];
    answer[2][2] = left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2] + left[2][3]*right[3][2] + left[2][4]*right[4][2] + left[2][5]*right[5][2] + left[2][6]*right[6][2] + left[2][7]*right[7][2];
    answer[2][3] = left[2][0]*right[0][3] + left[2][1]*right[1][3] + left[2][2]*right[2][3] + left[2][3]*right[3][3] + left[2][4]*right[4][3] + left[2][5]*right[5][3] + left[2][6]*right[6][3] + left[2][7]*right[7][3];
    answer[2][4] = left[2][0]*right[0][4] + left[2][1]*right[1][4] + left[2][2]*right[2][4] + left[2][3]*right[3][4] + left[2][4]*right[4][4] + left[2][5]*right[5][4] + left[2][6]*right[6][4] + left[2][7]*right[7][4];
    answer[2][5] = left[2][0]*right[0][5] + left[2][1]*right[1][5] + left[2][2]*right[2][5] + left[2][3]*right[3][5] + left[2][4]*right[4][5] + left[2][5]*right[5][5] + left[2][6]*right[6][5] + left[2][7]*right[7][5];
    answer[2][6] = left[2][0]*right[0][6] + left[2][1]*right[1][6] + left[2][2]*right[2][6] + left[2][3]*right[3][6] + left[2][4]*right[4][6] + left[2][5]*right[5][6] + left[2][6]*right[6][6] + left[2][7]*right[7][6];
    answer[2][7] = left[2][0]*right[0][7] + left[2][1]*right[1][7] + left[2][2]*right[2][7] + left[2][3]*right[3][7] + left[2][4]*right[4][7] + left[2][5]*right[5][7] + left[2][6]*right[6][7] + left[2][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[3][0] = left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0] + left[3][3]*right[3][0] + left[3][4]*right[4][0] + left[3][5]*right[5][0] + left[3][6]*right[6][0] + left[3][7]*right[7][0];
    answer[3][1] = left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1] + left[3][3]*right[3][1] + left[3][4]*right[4][1] + left[3][5]*right[5][1] + left[3][6]*right[6][1] + left[3][7]*right[7][1];
    answer[3][2] = left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2] + left[3][3]*right[3][2] + left[3][4]*right[4][2] + left[3][5]*right[5][2] + left[3][6]*right[6][2] + left[3][7]*right[7][2];
    answer[3][3] = left[3][0]*right[0][3] + left[3][1]*right[1][3] + left[3][2]*right[2][3] + left[3][3]*right[3][3] + left[3][4]*right[4][3] + left[3][5]*right[5][3] + left[3][6]*right[6][3] + left[3][7]*right[7][3];
    answer[3][4] = left[3][0]*right[0][4] + left[3][1]*right[1][4] + left[3][2]*right[2][4] + left[3][3]*right[3][4] + left[3][4]*right[4][4] + left[3][5]*right[5][4] + left[3][6]*right[6][4] + left[3][7]*right[7][4];
    answer[3][5] = left[3][0]*right[0][5] + left[3][1]*right[1][5] + left[3][2]*right[2][5] + left[3][3]*right[3][5] + left[3][4]*right[4][5] + left[3][5]*right[5][5] + left[3][6]*right[6][5] + left[3][7]*right[7][5];
    answer[3][6] = left[3][0]*right[0][6] + left[3][1]*right[1][6] + left[3][2]*right[2][6] + left[3][3]*right[3][6] + left[3][4]*right[4][6] + left[3][5]*right[5][6] + left[3][6]*right[6][6] + left[3][7]*right[7][6];
    answer[3][7] = left[3][0]*right[0][7] + left[3][1]*right[1][7] + left[3][2]*right[2][7] + left[3][3]*right[3][7] + left[3][4]*right[4][7] + left[3][5]*right[5][7] + left[3][6]*right[6][7] + left[3][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[4][0] = left[4][0]*right[0][0] + left[4][1]*right[1][0] + left[4][2]*right[2][0] + left[4][3]*right[3][0] + left[4][4]*right[4][0] + left[4][5]*right[5][0] + left[4][6]*right[6][0] + left[4][7]*right[7][0];
    answer[4][1] = left[4][0]*right[0][1] + left[4][1]*right[1][1] + left[4][2]*right[2][1] + left[4][3]*right[3][1] + left[4][4]*right[4][1] + left[4][5]*right[5][1] + left[4][6]*right[6][1] + left[4][7]*right[7][1];
    answer[4][2] = left[4][0]*right[0][2] + left[4][1]*right[1][2] + left[4][2]*right[2][2] + left[4][3]*right[3][2] + left[4][4]*right[4][2] + left[4][5]*right[5][2] + left[4][6]*right[6][2] + left[4][7]*right[7][2];
    answer[4][3] = left[4][0]*right[0][3] + left[4][1]*right[1][3] + left[4][2]*right[2][3] + left[4][3]*right[3][3] + left[4][4]*right[4][3] + left[4][5]*right[5][3] + left[4][6]*right[6][3] + left[4][7]*right[7][3];
    answer[4][4] = left[4][0]*right[0][4] + left[4][1]*right[1][4] + left[4][2]*right[2][4] + left[4][3]*right[3][4] + left[4][4]*right[4][4] + left[4][5]*right[5][4] + left[4][6]*right[6][4] + left[4][7]*right[7][4];
    answer[4][5] = left[4][0]*right[0][5] + left[4][1]*right[1][5] + left[4][2]*right[2][5] + left[4][3]*right[3][5] + left[4][4]*right[4][5] + left[4][5]*right[5][5] + left[4][6]*right[6][5] + left[4][7]*right[7][5];
    answer[4][6] = left[4][0]*right[0][6] + left[4][1]*right[1][6] + left[4][2]*right[2][6] + left[4][3]*right[3][6] + left[4][4]*right[4][6] + left[4][5]*right[5][6] + left[4][6]*right[6][6] + left[4][7]*right[7][6];
    answer[4][7] = left[4][0]*right[0][7] + left[4][1]*right[1][7] + left[4][2]*right[2][7] + left[4][3]*right[3][7] + left[4][4]*right[4][7] + left[4][5]*right[5][7] + left[4][6]*right[6][7] + left[4][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[5][0] = left[5][0]*right[0][0] + left[5][1]*right[1][0] + left[5][2]*right[2][0] + left[5][3]*right[3][0] + left[5][4]*right[4][0] + left[5][5]*right[5][0] + left[5][6]*right[6][0] + left[5][7]*right[7][0];
    answer[5][1] = left[5][0]*right[0][1] + left[5][1]*right[1][1] + left[5][2]*right[2][1] + left[5][3]*right[3][1] + left[5][4]*right[4][1] + left[5][5]*right[5][1] + left[5][6]*right[6][1] + left[5][7]*right[7][1];
    answer[5][2] = left[5][0]*right[0][2] + left[5][1]*right[1][2] + left[5][2]*right[2][2] + left[5][3]*right[3][2] + left[5][4]*right[4][2] + left[5][5]*right[5][2] + left[5][6]*right[6][2] + left[5][7]*right[7][2];
    answer[5][3] = left[5][0]*right[0][3] + left[5][1]*right[1][3] + left[5][2]*right[2][3] + left[5][3]*right[3][3] + left[5][4]*right[4][3] + left[5][5]*right[5][3] + left[5][6]*right[6][3] + left[5][7]*right[7][3];
    answer[5][4] = left[5][0]*right[0][4] + left[5][1]*right[1][4] + left[5][2]*right[2][4] + left[5][3]*right[3][4] + left[5][4]*right[4][4] + left[5][5]*right[5][4] + left[5][6]*right[6][4] + left[5][7]*right[7][4];
    answer[5][5] = left[5][0]*right[0][5] + left[5][1]*right[1][5] + left[5][2]*right[2][5] + left[5][3]*right[3][5] + left[5][4]*right[4][5] + left[5][5]*right[5][5] + left[5][6]*right[6][5] + left[5][7]*right[7][5];
    answer[5][6] = left[5][0]*right[0][6] + left[5][1]*right[1][6] + left[5][2]*right[2][6] + left[5][3]*right[3][6] + left[5][4]*right[4][6] + left[5][5]*right[5][6] + left[5][6]*right[6][6] + left[5][7]*right[7][6];
    answer[5][7] = left[5][0]*right[0][7] + left[5][1]*right[1][7] + left[5][2]*right[2][7] + left[5][3]*right[3][7] + left[5][4]*right[4][7] + left[5][5]*right[5][7] + left[5][6]*right[6][7] + left[5][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[6][0] = left[6][0]*right[0][0] + left[6][1]*right[1][0] + left[6][2]*right[2][0] + left[6][3]*right[3][0] + left[6][4]*right[4][0] + left[6][5]*right[5][0] + left[6][6]*right[6][0] + left[6][7]*right[7][0];
    answer[6][1] = left[6][0]*right[0][1] + left[6][1]*right[1][1] + left[6][2]*right[2][1] + left[6][3]*right[3][1] + left[6][4]*right[4][1] + left[6][5]*right[5][1] + left[6][6]*right[6][1] + left[6][7]*right[7][1];
    answer[6][2] = left[6][0]*right[0][2] + left[6][1]*right[1][2] + left[6][2]*right[2][2] + left[6][3]*right[3][2] + left[6][4]*right[4][2] + left[6][5]*right[5][2] + left[6][6]*right[6][2] + left[6][7]*right[7][2];
    answer[6][3] = left[6][0]*right[0][3] + left[6][1]*right[1][3] + left[6][2]*right[2][3] + left[6][3]*right[3][3] + left[6][4]*right[4][3] + left[6][5]*right[5][3] + left[6][6]*right[6][3] + left[6][7]*right[7][3];
    answer[6][4] = left[6][0]*right[0][4] + left[6][1]*right[1][4] + left[6][2]*right[2][4] + left[6][3]*right[3][4] + left[6][4]*right[4][4] + left[6][5]*right[5][4] + left[6][6]*right[6][4] + left[6][7]*right[7][4];
    answer[6][5] = left[6][0]*right[0][5] + left[6][1]*right[1][5] + left[6][2]*right[2][5] + left[6][3]*right[3][5] + left[6][4]*right[4][5] + left[6][5]*right[5][5] + left[6][6]*right[6][5] + left[6][7]*right[7][5];
    answer[6][6] = left[6][0]*right[0][6] + left[6][1]*right[1][6] + left[6][2]*right[2][6] + left[6][3]*right[3][6] + left[6][4]*right[4][6] + left[6][5]*right[5][6] + left[6][6]*right[6][6] + left[6][7]*right[7][6];
    answer[6][7] = left[6][0]*right[0][7] + left[6][1]*right[1][7] + left[6][2]*right[2][7] + left[6][3]*right[3][7] + left[6][4]*right[4][7] + left[6][5]*right[5][7] + left[6][6]*right[6][7] + left[6][7]*right[7][7];

    //     i  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j         i  R        R  j
    answer[7][0] = left[7][0]*right[0][0] + left[7][1]*right[1][0] + left[7][2]*right[2][0] + left[7][3]*right[3][0] + left[7][4]*right[4][0] + left[7][5]*right[5][0] + left[7][6]*right[6][0] + left[7][7]*right[7][0];
    answer[7][1] = left[7][0]*right[0][1] + left[7][1]*right[1][1] + left[7][2]*right[2][1] + left[7][3]*right[3][1] + left[7][4]*right[4][1] + left[7][5]*right[5][1] + left[7][6]*right[6][1] + left[7][7]*right[7][1];
    answer[7][2] = left[7][0]*right[0][2] + left[7][1]*right[1][2] + left[7][2]*right[2][2] + left[7][3]*right[3][2] + left[7][4]*right[4][2] + left[7][5]*right[5][2] + left[7][6]*right[6][2] + left[7][7]*right[7][2];
    answer[7][3] = left[7][0]*right[0][3] + left[7][1]*right[1][3] + left[7][2]*right[2][3] + left[7][3]*right[3][3] + left[7][4]*right[4][3] + left[7][5]*right[5][3] + left[7][6]*right[6][3] + left[7][7]*right[7][3];
    answer[7][4] = left[7][0]*right[0][4] + left[7][1]*right[1][4] + left[7][2]*right[2][4] + left[7][3]*right[3][4] + left[7][4]*right[4][4] + left[7][5]*right[5][4] + left[7][6]*right[6][4] + left[7][7]*right[7][4];
    answer[7][5] = left[7][0]*right[0][5] + left[7][1]*right[1][5] + left[7][2]*right[2][5] + left[7][3]*right[3][5] + left[7][4]*right[4][5] + left[7][5]*right[5][5] + left[7][6]*right[6][5] + left[7][7]*right[7][5];
    answer[7][6] = left[7][0]*right[0][6] + left[7][1]*right[1][6] + left[7][2]*right[2][6] + left[7][3]*right[3][6] + left[7][4]*right[4][6] + left[7][5]*right[5][6] + left[7][6]*right[6][6] + left[7][7]*right[7][6];
    answer[7][7] = left[7][0]*right[0][7] + left[7][1]*right[1][7] + left[7][2]*right[2][7] + left[7][3]*right[3][7] + left[7][4]*right[4][7] + left[7][5]*right[5][7] + left[7][6]*right[6][7] + left[7][7]*right[7][7];
}


inline int const quant_element( const int bits, const float input)
{
    //  Total, we have:
    //  1  if statement
    //  1  float adds
    //  0  float mults
    //  1  int adds
    // [1] float sign change
    // [1] int sign change
    //  2  bit shifts
    //  1  cast

    // We are sneaky here with the float to int conversion.  Nice.
    if(input < 0)                                 // OK, we have a negative number
    {
        return -( (((int)( -input + (1 << (bits-1))) ) )>> bits);
    }
    else
    {
        return ((((int)( input + (1 << (bits-1))) )) >> bits);
    }

}


inline  float const inv_quant_element( const int bits, const int input)
{

    // This is a trivial conversion, but the parens make it look complicated
    if(input < 0)
    {
        //printf("INV QUANT: input = %d, output = %f\n", input,(float)(input << bits));
        return ((float)(input << bits));
    }
    else
    {
        //printf("INV QUANT: input = %d, output = %f\n", input,(float)(- (-input << bits)));
        return ((float)(- (-input << bits)));     // Make positive, shift, then make negative again.  We don't want to goof with the sign bit... uck
    }

}


void print_matrix(const float target[8][8])
{
    int i = 0; int j = 0;

    printf("\n\rAnswer =\n\r");
    for(i=0;i <8; i++)
    {
        for(j=0; j < 8; j++)
            printf("%d\t", (int)target[i][j]);
        printf("\n\r");
    }
    printf("\n\r");
}


void print_image_matrix(const float target[8][8][3])
{
    int i = 0; int j = 0;

    printf("RGB image:\n");
    printf("\tR:\n\r");

    for(i=0;i <8; i++)
    {
        printf("\t");
        for(j=0; j < 8; j++)
            printf("%d\t", (int)target[i][j][1]);
        printf("\n\r");
    }
    printf("\tG:\n\r");
    for(i=0;i <8; i++)
    {
        printf("\t");
        for(j=0; j < 8; j++)
            printf("%d\t", (int)target[i][j][2]);
        printf("\n\r");
    }
    printf("\tB:\n\r");
    for(i=0;i <8; i++)
    {
        printf("\t");
        for(j=0; j < 8; j++)
            printf("%d\t", (int)target[i][j][3]);
        printf("\n\r");
    }

    printf("\n\r");
}


void print_int_matrix(const int target[8][8])
{
    int i = 0; int j = 0;

    //if(target[0][0] == 67) printf("JAMMIN\n");

    printf("\n\rAnswer =\n\r");
    for(i=0;i <8; i++)
    {
        for(j=0; j < 8; j++)
            printf("%d\t", target[i][j]);
        printf("\n\r");
    }
    printf("\n\r");
}


// ****************************************** Decoder section *************************************

void print_data_in_zig_zag_order(int data[8][8])
{

    bool move_up_diagonal = true;                 // true = upstroke, false = downstroke (regarding the zig-zag pattern

    int n_blocks = 0;
    int block_row = 0;
    int block_col = 0;

    while(n_blocks < 64)
    {
        //printf("<row,col> = <%u,%u>, val = %f\n", block_row, block_col, data[block_row][block_col]);
        printf("%d ",data[block_row][block_col]);

        if(move_up_diagonal)                      // Move to the next block
        {
            if(block_row == 0)                    // We have hit an upper boundary
            {
                if(block_col+1 == 8)              // We have hit the upper right corner
                    block_row++;
                else                              // We can move over one, do that.
                    block_col++;

                move_up_diagonal = false;
            }
            else if(block_col+1 == 8)             // We have reached the right edge without the zero row
            {
                block_row++;
                move_up_diagonal = false;
            }
            else                                  // normal case
            {
                block_row--;
                block_col++;
            }
        }
        else
        {
            if(block_col == 0)                    // We are going to step off the left side of the image
            {
                if(block_row+1 == 8)              // We have hit the lower left corner
                    block_col++;
                else                              // We have just hit the left side
                    block_row++;

                move_up_diagonal = true;
            }
            else if (block_row+1 == 8)            // We have hit the bottom edge without the zero column
            {
                block_col++;
                move_up_diagonal = true;
            }
            else                                  //	  if(block_row+1 == YHEIGHT/8)   // We have stepped off the bottom of the image
            {
                block_row++;
                block_col--;
            }
        }
        n_blocks++;
    }
    printf("\n");
}

#ifndef EXCLUDE_DECODER
inline bool add_bit_to_value(int & value, int & num_bits)
{
                                                  // Good we have the rest of this byte to ourselves.
    if(dec_byte_pos < valid_dec_byte_pos || (dec_byte_pos == valid_dec_byte_pos && dec_bit_pos > valid_dec_bit_pos))
    {
      if(txs_debug)
      //if(dec_byte_pos > 25 )
	//	exit(1); 
	//else
	printf("add_bit says: <dec_bit_pos,dec_byte_pos>:<valid_bit, valid_byte>:<num_bits> : <bit>  = <%d,%d>:<%d,%d>:<%d> : <%d> \n",dec_bit_pos, dec_byte_pos, valid_dec_bit_pos, valid_dec_byte_pos, num_bits + 1, ( ( dec_outbuf[dec_byte_pos] & (1 << dec_bit_pos)) >> dec_bit_pos));
        value = (value << 1) + ( ( dec_outbuf[dec_byte_pos] & (1 << dec_bit_pos)) >> dec_bit_pos);
        num_bits++;

        dec_bit_pos--;
        if(dec_bit_pos < 0)                       // We are going from one byte to the next
        {
            dec_bit_pos = 7;
            dec_byte_pos++;
        }
        //printf("new value = %d \n", value);

        return true;
    }
    else
    {
        printf("\nNot enough data!\n");
        return false;
    }

}
#endif

#ifndef EXCLUDE_DECODER
bool check_for_valid_VLC(int test_num, int RL[2], int bits, bool is_luminance)
{

    //    printf("Finding Value... (%d bits)\n", RLd[0]-1);

    int temp = 0;
    static int junk = 0;

    if(txs_debug)
    {
        printf("Checking value = %d with %d bits. NEXT to read is <dec_bit_pos, dec_byte_pos> = <%d,%d>\n", test_num, bits, dec_bit_pos, dec_byte_pos);
        printf("Checking value = %d with %d bits.\n", test_num, bits);
    }
    coeff_table * the_table;

    if(is_luminance)
        the_table = &(lum_AC_table);
    else
        the_table = &(chr_AC_table);

    if(false == the_table->find_match(bits, test_num, RL))
        return false;

    if(RL[1] != 0)                                // If the value is not equal to 0, get the sign bit
    {
        if(false == add_bit_to_value(temp, junk))
        {
            printf("Out of data!  Get more from the network!\n");
            return false;
        }

        if(temp == 1)
            RL[1] = -RL[1];
    }
    if(txs_debug)
    {
        printf("We found a match at value = %d, number of bits = %d", test_num, bits);
        printf("\tMatch has run = %d, level = %d, <dec_bit_pos,dec_byte_pos> = <%d,%d>\n", RL[0], RL[1], dec_bit_pos, dec_byte_pos);
    }
    return true;
}
#endif

#ifndef EXCLUDE_DECODER
bool find_next_DC(bool first_in_row, int & DC_level, bool is_luminance)
{
  //    if(first_in_row)
  //      printf("This coefficient is the first in the row.  Let's be smart\n");

    int temp = 0;
    int value = 0;
    int hypothesis = 0;
    int num_bits = 0;
    int RLd[2];
    DC_level = 0;                                 // Let's reset it in case the user gets sloppy.
    coeff_table * the_table;

    if(is_luminance)
        the_table = &(lum_DC_table);
    else
        the_table = &(chr_DC_table);

    while(1)
    {
                                                  // This is fine, we just don't have enough data.  Get more from the network
        if(false == add_bit_to_value(hypothesis, num_bits))
        {
            return -1;
        }

        //printf("Trying %d with %d bits\n", hypothesis, num_bits);

        if(the_table->find_match(num_bits, hypothesis, RLd) == true)
        {
            // printf("We found a match at run = %d, level = %d\n", RLd[0], RLd[1]);
            //printf("Now, we will extract %d bits to find the value\n", RLd[0]);

            for(int i = 0; i < RLd[0]-1; i++)
            {
                add_bit_to_value(value, RLd[1]);  // The second argument doesn't matter;
            }

            if(RLd[0] >= 1)                       // This adds the MSB back in.  We don't have a MSB if the values are -1,0,1
                value = value + (1 <<  (RLd[0] - 1));

            if(RLd[0] != 0)
            {
                add_bit_to_value(temp, RLd[1]);   // Get the sign bit
                if(temp == 1)
                    value = -value;               // TODO: Test to see this works!!
            }
            //printf("value = %d***\n", value);
            break;
        }
    }

    if(is_luminance)
    {
      if(!first_in_row)
      {
        DC_level = value + dec_last_DC_value;
        dec_last_DC_value = DC_level;             // TODO: test to see that the subsequent blocks work correctly!
      }
      else
      {
        DC_level = value + dec_last_DC_value_from_first_column;
        printf("(first row coeff) DC_level = %i, dec_last_DC = %i, dec_last_DC_from_first = %i\n",DC_level, dec_last_DC_value, dec_last_DC_value_from_first_column);

        dec_last_DC_value = DC_level;
        dec_last_DC_value_from_first_column = DC_level;
      }
    }
    else
    {
      if(!first_in_row)
      {
        DC_level = value + dec_last_chr_DC_value;
        dec_last_chr_DC_value = DC_level;             // TODO: test to see that the subsequent blocks work correctly!
      }
      else
      {
        DC_level = value + dec_last_chr_DC_value_from_first_column;
        printf("(first row coeff) DC_level = %i, dec_last_DC = %i, dec_last_DC_from_first = %i\n",DC_level, dec_last_DC_value, dec_last_DC_value_from_first_column);

        dec_last_chr_DC_value = DC_level;
        dec_last_chr_DC_value_from_first_column = DC_level;
      }

    }
    return true;
}
#endif

#ifndef EXCLUDE_DECODER
bool find_next_VLC(int RL[2], bool is_luminance)
{                                                 // -1 will designate the end of the block, otherwise, it will have the VLC codes in order
    int hypothesis = 0;
    int num_bits = 0;
    while(1)
    {
        // TODO: Check to see that we have enough data in the buffer

                                                  // This is fine, we just don't have enough data.  Get more from the network
        if(false == add_bit_to_value(hypothesis, num_bits))
        {
            return 0;
        }

        if(true == check_for_valid_VLC(hypothesis, RL, num_bits, is_luminance))
        {
            return 1;
        }
        else if(num_bits > 16)
        {
            printf("ERROR! We should never have a code this long!  Something wrong with the VLC lookup!\n");
            //exit(1);
        };
    }

}
#endif

void add_run_level_to_decoded_matrix(int answer[8][8], int run, int level)
{
    bool move_up_diagonal = true;                 // true = upstroke, false = downstroke (regarding the zig-zag pattern)

    int n_blocks = 0;                             // This whole function is somewhat sloppy and could be recoded for speed  I tossed it together quickly, so that's why it is so.
    int block_row = 0;
    int block_col = 0;

    int to_add = run + 1;                         // We are going to add this many entries.

    if(run == 0 && level == 0)                    // Yes, I know, there is a more intelligent way to do this.  But this is easy and doesn't require much thinking =)
    {
        to_add = 674;
    }
    else if( run == -1)                           // Another hack.  But efficient
        answer[0][0] = level;

    while(n_blocks < 64 && to_add > 0)            // This will search for the 9999999 entries
    {
        if(move_up_diagonal)                      // Move to the next block
        {
            if(block_row == 0)                    // We have hit an upper boundary
            {
                if(block_col+1 == 8)              // We have hit the upper right corner
                    block_row++;
                else                              // We can move over one, do that.
                    block_col++;

                move_up_diagonal = false;
            }
            else if(block_col+1 == 8)             // We have reached the right edge without the zero row
            {
                block_row++;
                move_up_diagonal = false;
            }
            else                                  // normal case
            {
                block_row--;
                block_col++;
            }
        }
        else
        {
            if(block_col == 0)                    // We are going to step off the left side of the image
            {
                if(block_row+1 == 8)              // We have hit the lower left corner
                    block_col++;
                else                              // We have just hit the left side
                    block_row++;

                move_up_diagonal = true;
            }
            else if (block_row+1 == 8)            // We have hit the bottom edge without the zero column
            {
                block_col++;
                move_up_diagonal = true;
            }
            else                                  // We have stepped off the bottom of the image
            {
                block_row++;
                block_col--;
            }
        }

        if(answer[block_row][block_col] > 9999998)
        {
            to_add--;
            if(run == 0 && level == 0)
                to_add = 63 - n_blocks;

            if(to_add == 0)
                answer[block_row][block_col] = level;
            else
                answer[block_row][block_col] = 0; // We still have more work to do.

        }

        n_blocks++;
    }
}


void initialize_decode_matrix(int target[8][8])
{
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            target[i][j] =  9999998 + 1;

}


bool compare_two_matricies(int a[8][8], int b[8][8])
{
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            if(a[i][j] != b[i][j])
                return false;

    return true;
}


// ******************************************* network sending data  *****************************
#ifndef EXCLUDE_DECODER
void extract_block_from_80_byte_buffer()
{
    // This function will:
    // 1) Grab a packet from the network
    // 2) Decode the information that it has
    // 3) Place the pixels on the screen

    static int DCT_received_decoded[8][8];
    static int frame_number = 0;
    static int max_last_bit_dec = 0;
    static int max_last_byte_dec = 0;

    static float DCT_result[8][8];   memset(&DCT_result[0][0], 0, sizeof(float)*64);
    static float DCT_temp_res[8][8]; memset(&DCT_temp_res[0][0], 0, sizeof(float)*64);
    static int   quant_result[8][8];

    static float recovered_pixels[8][8]; 

    static float recovered_Y_left[8][8];
    static float recovered_Y_right[8][8];
    static float recovered_Cr[8][8];
    static float recovered_Cb[8][8];

    blocks_found_in_network_packet = 0; 

    dec_byte_pos = 0;                             // We know that our data is aligned at the beginning of the block
    dec_bit_pos = 7;

    memcpy(dec_outbuf, network_packet, 80);       // This will be later replaced by a network packet.

    // Get the frame number
    frame_number = network_packet[79] + ((0x1F & network_packet[78]) << 8);

    // Get the number of the bit that is next in line.
    max_last_bit_dec = (network_packet[78] >> 5);

    // Get the number of the byte that is next in line.
    max_last_byte_dec = network_packet[77];

    printf("We have a received a block to decode.\nThe next free bit is: %d and this occurs in byte: %d\nThis frame number is: %d\n\n", max_last_bit_dec, max_last_byte_dec, frame_number);
    //printf("The 0th byte is %d\n", network_packet[0]); 
    //printf("The 1st byte is %d\n", network_packet[1]); 
    //printf("The 20th bytes is %d\n", network_packet[20]); 

    int RL[2] = {999999, 9999999};
    int DC_level = 0;
    bool to_continue = true;
    int is_luminance = 1; 

    //    for(int num_blocks = 0; num_blocks < IMG_Y/8; num_blocks++)
                                                  // TODO: This is to start, eventually this will be a while(more_data)
    for(int num_blocks = 0; num_blocks < 1; num_blocks++)
    {
      blocks_found_in_network_packet++; 
      for(int sequence_of_luminance_blocks = 0; sequence_of_luminance_blocks < 2; sequence_of_luminance_blocks++)  // There should be two Y blocks per chunk. 
      {
	memset(&DCT_result[0][0], 0, sizeof(float)*64);                  // Erase our decoded blocks
	memset(&DCT_temp_res[0][0], 0, sizeof(float)*64);

	RL[0] = 999999; // Not sure if this is required.... TODO
	RL[1] = 999999; 
        initialize_decode_matrix(DCT_received_decoded);
      
        // DC coefficient for the block
         find_next_DC(1, DC_level, is_luminance);
        add_run_level_to_decoded_matrix(DCT_received_decoded, -1, DC_level);

        // AC coefficients for the block
        to_continue = find_next_VLC(RL, is_luminance );
        add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

        while(to_continue )                       // Hmm... I'm not sure which order this evaluates
        {
            to_continue = find_next_VLC(RL, is_luminance);

            if(to_continue)
                add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

            if((RL[0] == 0 && RL[1] == 0))
                to_continue = false;
        }

	// Now, we have the DCT matrix, let's inverse quantize it. 
	for(int i = 0; i < 8; i++)
	  for(int j = 0; j < 8; j++)
            if(is_luminance)
	      recovered_pixels[i][j] = inv_quant_element(luminance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);
	    else
	      recovered_pixels[i][j] = inv_quant_element(chrominance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);
	
	// Finally, let's take the inverse DCT
	mult_two_mats(DCT_temp_res, DCT_coeffs, recovered_pixels);
	mult_two_mats(recovered_pixels, DCT_temp_res, DCT_coeffs_trans);

	if(sequence_of_luminance_blocks == 0)
	  memcpy(recovered_Y_left,  recovered_pixels, sizeof(float)*64); 
	else 
	  memcpy(recovered_Y_right, recovered_pixels, sizeof(float)*64); 

      }

      is_luminance = false; 
      DC_level = 0; 
      // Extract the Cr and Cb matricies!!!
     for(int sequence_of_chrominance_blocks = 0; sequence_of_chrominance_blocks < 2; sequence_of_chrominance_blocks++)  // There should be two chrominance matrices - one Cr the other Cb. 
     {
       //printf("TRYING TO DECODE A CHROMINANCE BLOCK!!!\n\n"); 
	memset(&DCT_result[0][0], 0, sizeof(float)*64);                  // Erase our decoded blocks
	memset(&DCT_temp_res[0][0], 0, sizeof(float)*64);

	RL[0] = 999999; // Not sure if this is required.... TODO
	RL[1] = 999999; 
        initialize_decode_matrix(DCT_received_decoded);

        // DC coefficient for the block
        find_next_DC(1, DC_level, is_luminance);
        add_run_level_to_decoded_matrix(DCT_received_decoded, -1, DC_level);

        // AC coefficients for the block
        to_continue = find_next_VLC(RL, is_luminance );
        add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

        while(to_continue )                       // Hmm... I'm not sure which order this evaluates
        {
            to_continue = find_next_VLC(RL, is_luminance);

            if(to_continue)
                add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

            if((RL[0] == 0 && RL[1] == 0))
                to_continue = false;
        }
	// If we wanted to, compare the matricies.  See the code below for how to do this. 
	print_int_matrix(DCT_received_decoded);

	// Now, we have the DCT matrix, let's inverse quantize it. 
	for(int i = 0; i < 8; i++)
	  for(int j = 0; j < 8; j++)
            if(is_luminance)
	      recovered_pixels[i][j] = inv_quant_element(luminance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);
	    else
	      recovered_pixels[i][j] = inv_quant_element(chrominance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);
	
	// Finally, let's take the inverse DCT
	mult_two_mats(DCT_temp_res, DCT_coeffs, recovered_pixels);
	mult_two_mats(recovered_pixels, DCT_temp_res, DCT_coeffs_trans);

	if(sequence_of_chrominance_blocks == 0) 
	  memcpy(recovered_Cr, recovered_pixels, sizeof(float)*64); 
	else
	  memcpy(recovered_Cb, recovered_pixels, sizeof(float)*64);
       }


     print_matrix(input);
     print_matrix(recovered_Y_left);
     print_matrix(recovered_Y_right);
     print_matrix(recovered_Cr);
     print_matrix(recovered_Cb);

     if(num_blocks == 0) 
     {
       memcpy(img_Y_left1, recovered_Y_left, sizeof(float)*64); 
       memcpy(img_Y_right1, recovered_Y_right, sizeof(float)*64); 
       memcpy(img_Cb_1, recovered_Cb, sizeof(float)*64); 
       memcpy(img_Cr_1, recovered_Cr, sizeof(float)*64); 
       block_position = frame_number; 
     }
     else if(num_blocks == 1)
     {
       memcpy(img_Y_left2, recovered_Y_left, sizeof(float)*64); 
       memcpy(img_Y_right2, recovered_Y_right, sizeof(float)*64); 
       memcpy(img_Cb_2, recovered_Cb, sizeof(float)*64); 
       memcpy(img_Cr_2, recovered_Cr, sizeof(float)*64); 
     }
     else if(num_blocks == 2)
     {
       memcpy(img_Y_left3, recovered_Y_left, sizeof(float)*64); 
       memcpy(img_Y_right3, recovered_Y_right, sizeof(float)*64); 
       memcpy(img_Cb_3, recovered_Cb, sizeof(float)*64); 
       memcpy(img_Cr_3, recovered_Cr, sizeof(float)*64); 
     }
     else
     {
       printf("ERROR! We should never have this many blocks in a packet!!\n"); 
     }
    }
}
#endif

void send_network_packet(const int frame_number)
{

int i,j;
char c;

    // This is the format for the packet:
    // bytes    use
    // 0-76     data (do not need to use all 77 bytes)
    // 77       length (equal to the highest byte used, even if not all of it is used)
    // 78       highest 3 bits = the number of the first bit that is not used.  e.g.  if we use 75 bytes + 3 bits, this number would be 4.  or, if we use 76 bytes, this number would be 7 (0-1=7)
    //          remaining 5 bits are the MSBs for the frame number
    // 79       LSBs for the frame number, where the frame number is the last block to be encoded

    // network_packet[80]
    // byte_pos
    // bit_pos
    // last_network_packet_start_bit:          the last starting bit will tell us how much we need to shift each block
    // last_network_packet_start_byte

                                                  // Clear our packet
    memset(network_packet, 0, sizeof(unsigned char)*80);

    // We want to take all of the blocks up until last_network_packet_start_byte
    memcpy(network_packet, outbuf, 77);           // Why think about this, take all 77!  Not much wasted here.
    network_packet[79] = frame_number % 256;      // The next two lines allow 13 bits for the frame number
                                                  // Don't allow this to be larger than 5 bits.
    network_packet[78] = (0x1F & (frame_number >> 8));
    network_packet[78] = network_packet[78] | (last_network_packet_start_bit << 5);
    network_packet[77] = last_network_packet_start_byte;

    // We will then have to shift all of the remaining blocks to <7,0>.  What an awful waste of resources, but there is no other way around this problem.
    // We could try to get smart and only copy the data we know exist, but who cares.  As long as we keep track of which data is valid by the above method, it will not matter
    for(int j = last_network_packet_start_byte; j < (cmu_cam_board_buffer_size -1); j++)
        outbuf[j - last_network_packet_start_byte] = ((outbuf[j] << (7 - last_network_packet_start_bit)) % 256) +
            (outbuf[j+1] >> (last_network_packet_start_bit + 1));

    // Fix the indicies and continue
    if(last_network_packet_start_bit > bit_pos)
    {
        byte_pos = byte_pos - last_network_packet_start_byte;
        bit_pos = 7 + bit_pos - last_network_packet_start_bit;
    }
    else if(last_network_packet_start_bit < bit_pos)
    {
        byte_pos = byte_pos - (1 + last_network_packet_start_byte);
        bit_pos  = 7 - (8 - (bit_pos - last_network_packet_start_bit));
    }
    else
    {
                                                  // Zero based means we don't have to compensate
        byte_pos = byte_pos - last_network_packet_start_byte;
        bit_pos = 7;
    }
  
    // TODO: SEND THE BITS OVER THE NETWORK AND DECODE
    putchar('*');
    fflush(stdout);
    c=uart0_getc();
    for(i=0; i<80; i++ )
	{
	putchar(network_packet[i]);
	}
fflush(stdout);

}



/*
void convert_YUV_block_to_RGB(const float Y_left[8][8],const float Y_right[8][8],const  float U[8][8],const float V[8][8], float RGB_left[8][8][3], float RGB_right[8][8][3])
{
    // There are four blocks interleaved.  The UV space is sampled horizontally, hence the composite block is 8x16
    //
    //  YL YR  <-- total of 8x16
    //   U  V  <-- total of 8x16

    float loc_r, loc_g, loc_b;

    for(int i = 0; i < 8; i++)                    // We will run over the left 8x8 block
        for(int j = 0; j < 8; j++)
    {
        convert_YUV_pixel_to_RGB(Y_left[i][j] , U[i][int(j/2)], V[i][int(j/2)], loc_r, loc_g, loc_b);

        RGB_left[i][j][1] = loc_r;                // Load our values into the matricies
        RGB_left[i][j][2] = loc_g;
        RGB_left[i][j][3] = loc_b;

    }

    for(int i = 0; i < 8; i++)                    // We will run over the right 8x8 block
        for(int j = 0; j < 8; j++)
    {
        convert_YUV_pixel_to_RGB(Y_right[i][j] , U[i][int(j/2)+4], V[i][int(j/2)+4], loc_r, loc_g, loc_b);

        RGB_right[i][j][1] = loc_r;               // Load our values into the matricies
        RGB_right[i][j][2] = loc_g;
        RGB_right[i][j][3] = loc_b;
    }

}
*/

// ******************************************* test ground section *******************************

void initialize_and_run_jpeg()
{
    const int start_bit = 7;


    last_network_packet_start_bit = start_bit;    // Start at the beginning
    last_network_packet_start_byte = 0;
    amount_of_data_in_network_packet_from_boundary_edge_bits = 0;

    bit_pos = start_bit;                          // Let's start at byte 0 and the left most bit
    byte_pos = 0;                                 //  76543210 76543210 76543210 76543210 ....

#ifndef EXCLUDE_DECODER
    valid_dec_bit_pos = 7;                        // This bit that it referes to will actually be invalid!!!!
    valid_dec_byte_pos = 1210;

    dec_bit_pos = start_bit;                      // this is for the decoder only and it represents the next bit not read yet.
    dec_byte_pos = 0;-1      0       0       0       0       0       0       0
    dec_last_DC_value = 0;
    dec_last_DC_value_from_first_column = 0;
    dec_last_chr_DC_value = 0;
    dec_last_chr_DC_value_from_first_column = 0;
#endif

    // Fix our globals
    frame_done = 1;
    last_DC_value = 0;
    last_DC_value_from_first_column = 0;
    chr_last_DC_value = 0;
    chr_last_DC_value_from_first_column = 0;
    memset(&outbuf[0], 0, sizeof(unsigned char)*cmu_cam_board_buffer_size);

    //      0        1        2        3    ....-1      0       0       0       0       0       0       0
    /*
    const float DCT_coeffs[8][8] =
    {
        0.3536,    0.4904,    0.4619,    0.4157,    0.3536,    0.2778,    0.1913,    0.0975,
        0.3536,    0.4157,    0.1913,   -0.0975,   -0.3536,   -0.4904,   -0.4619,   -0.2778,
        0.3536,    0.2778,   -0.1913,   -0.4904,   -0.3536,    0.0975,    0.4619,    0.4157,
        0.3536,    0.0975,   -0.4619,   -0.2778,    0.3536,    0.4157,   -0.1913,   -0.4904,
        0.3536,   -0.0975,   -0.4619,    0.2778,    0.3536,   -0.4157,   -0.1913,    0.4904,
        0.3536,   -0.2778,   -0.1913,    0.4904,   -0.3536,   -0.0975,    0.4619,   -0.4157,
        0.3536,   -0.4157,    0.1913,    0.0975,   -0.3536,    0.4904,   -0.4619,    0.2778,
        0.3536,   -0.4904,    0.4619,   -0.4157,    0.3536,   -0.2778,    0.1913,   -0.0975
    };

    const float DCT_coeffs_trans[8][8] =
    {
        0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,    0.3536,
        0.4904,    0.4157,    0.2778,    0.0975,   -0.0975,   -0.2778,   -0.4157,   -0.4904,
        0.4619,    0.1913,   -0.1913,   -0.4619,   -0.4619,   -0.1913,    0.1913,    0.4619,
        0.4157,   -0.0975,   -0.4904,   -0.2778,    0.2778,    0.4904,    0.0975,   -0.4157,
        0.3536,   -0.3536,   -0.3536,    0.3536,    0.3536,   -0.3536,   -0.3536,    0.3536,
        0.2778,   -0.4904,    0.0975,    0.4157,   -0.4157,   -0.0975,    0.4904,   -0.2778,
        0.1913,   -0.4619,    0.4619,   -0.1913,   -0.1913,    0.4619,   -0.4619,    0.1913,
        0.0975,   -0.2778,    0.4157,   -0.4904,    0.4904,   -0.4157,    0.2778,   -0.0975
	}; 

    const float input[8][8] =                     // Let's try some real data from my "Kell Bell" image
    {
        125,  120,  110,  111,  120,  124,  131,  149,
        126,  121,  115,  117,  120,  120,  125,  142,
        126,  123,  116,  121,  126,  126,  127,  140,
        127,  129,  126,  127,  133,  139,  139,  144,
        129,  138,  133,  139,  146,  148,  164,  152,
        152,  141,  137,  139,  146,  152,  156,  150,
        141,  132,  132,  134,  143,  146,  156,  150,
        119,  120,  131,  137,  146,  157,  155,  151
    };

    const float input2[8][8] =
    {
        240,  234,  232,  228,  225,  215,  202,  181,
        238,  235,  232,  233,  223,  209,  197,  165,
        236,  232,  232,  235,  213,  209,  190,  155,
        232,  233,  234,  224,  212,  199,  177,  146,
        229,  229,  232,  222,  208,  195,  168,  136,
        227,  225,  220,  218,  202,  185,  152,  125,
        224,  218,  213,  211,  191,  173,  141,  113,
        227,  215,  214,  203,  186,  159,  128,  100
    };
    */
    // DECLARATIONS

    // ******************************** Quantization table declarations *******************************
    //
    // Suggested Luminance table (remember, your eye is more sensitive to chrominance):
    // 16        11        10        16        24        40        51        61
    // 12        12        14        19        26        58        60        55
    // 14        13        16        24        40        57        69        56
    // 14        17        22        29        51        87        80        62
    // 18        22        37        56        68        109      103        77
    // 24        35        55        64        81        104      113        92
    // 49        64        78        87       103        121      120       101
    // 72        92        95        98       112        100      103        99
    //
    // Suggested Chrominance table (you eye is less sensitive to this)
    // 17        18        24        47        99        99        99        99
    // 18        21        26        66        99        99        99        99
    // 24        26        56        99        99        99        99        99
    // 47        66        99        99        99        99        99        99
    // 99        99        99        99        99        99        99        99
    // 99        99        99        99        99        99        99        99
    // 99        99        99        99        99        99        99        99
    // 99        99        99        99        99        99        99        99
    //
    // However, we will not use these tables.  Remember, we pay a huge price for floating point
    // calculations.  Let's see if we can get this to be mere bit shifts.  Let's try these tables:
    //
    // luminance:
    // 16,       8,        8,       16,       32,       32,      64,        64,
    //  8,       8,        8,       16,       16,       64,      64,        64,
    // 16,       8,       16,       16,       32,       64,      64,        64,
    // 16,      16,       16,       32,       64,       64,      64,        64,
    // 16,      16,       32,       64,       64,      128,     128,        64,
    // 16,      32,       64,       64,       64,      128,     128,        64,
    // 64,      64,       64,       64,      128,      128,     128,       128,
    // 64,     128,      128,      128,      128,      128,     128,        64
    //
    // chrominance:
    //  16,      16,       16,       64,      128,      128,     128,       128,
    //  16,      16,       16,       64,      128,      128,     128,       128,
    //  16,      16,       64,      128,      128,      128,     128,       128,
    //  64,      64,      128,      128,      128,      128,     128,       128,
    // 128,     128,      128,      128,      128,      128,     128,       128,
    // 128,     128,      128,      128,      128,      128,     128,       128,
    // 128,     128,      128,      128,      128,      128,     128,       128,
    // 128,     128,      128,      128,      128,      128,     128,       128,
    //

    // Of course, only the bit shift numbers matter.  Here are the corresponding powers of 2:
    /*    const int luminance_quant_bit_shift_table[8][8] =
    {
        4,       3,        3,       4,       5,       5,      6,        6,
        3,       3,        3,       4,       4,       6,      6,        6,
        4,       3,        4,       4,       5,       6,      6,        6,
        4,       4,        4,       5,       6,       6,      6,        6,
        4,       4,        5,       6,       6,       7,      7,        6,
        4,       5,        6,       6,       6,       7,      7,        6,
        6,       6,        6,       6,       7,       7,      7,        7,
        6,       7,        7,       7,       7,       7,      7,        6
    };

    const int chrominance_quant_bit_shift_table[8][8] =
    {
        4,      4,       4,       6,      7,      7,     7,       7,
        4,      4,       4,       6,      7,      7,     7,       7,
        4,      4,       6,       7,      7,      7,     7,       7,
        6,      6,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7,
        7,      7,       7,       7,      7,      7,     7,       7
    };
    */
    // Remeber, we can not trivially apply these tables in the case where we have negative values...  A
    // bit shift with a negative value will goof up the twos-complement and send us camping.

    // ***************************** Huffman / RLE / packing declarations *****************************
    // For the DC coefficient coding scheme, please see the function above "add_DC_coefficient_to_stream"

    printf("\r\nTHIS SHOULD BE ZERO: %d", last_DC_value);

    // ************************************************************************************************
    // ***************************** EXECUTE JPEG ON ONE IMAGE ****************************************
    // ************************************************************************************************

    float DCT_result[8][8];   memset(&DCT_result[0][0], 0, sizeof(float)*64);
    float DCT_temp_res[8][8]; memset(&DCT_temp_res[0][0], 0, sizeof(float)*64);
    int   quant_result[8][8];


    const int block_row = 2; // These are zero based!
    const int block_col = 4; 
    int block_number = block_row*IMG_Y/8 + block_col; 

    cc3_pixbuf_load();

    for(int row=0; row<11; row++ )
    {
    load_line();
    for( int col=0; col<18; col+=2 )
    {
    for(int ijkl = 0; ijkl < 1; ijkl++)           // We want to just do one block at a time.  One block is YYUV
    {
      // Y packets
      for(int is_right_Y_packet = 0; is_right_Y_packet < 2; is_right_Y_packet++)
      { 
        last_network_packet_start_bit = bit_pos;  // Before we add the block, record where we are
        last_network_packet_start_byte = byte_pos;

                                                  // TODO: get the data and put it here (where input currently is).
        for(int j=0; j<8; j++ )
	       for(int k=0; k<8; k++ )
	       	input[j][k]=img_Y[j][k+is_right_Y_packet*8+(8*col)];	

    	mult_two_mats(DCT_temp_res, DCT_coeffs_trans, input);
        mult_two_mats(DCT_result, DCT_temp_res, DCT_coeffs);

        // ************************************* Quantization ***************************
        int i = 0; int j = 0;   int q = 0;       // TODO: inline this for loop

        for(i = 0; i < 8; i++)
            for(j = 0; j < 8; j++)
	      if(1)
		quant_result[i][j] = quant_element(luminance_quant_bit_shift_table[i][j],DCT_result[i][j]);
	      else
		quant_result[i][j] = quant_element(chrominance_quant_bit_shift_table[i][j],DCT_result[i][j]);

        // ******************** Run-Level Encoding && Byte Packing **********************
	add_DC_coefficient_to_stream(quant_result[0][0],1,1);  // the first 1 is for the first row param
	add_AC_coefficients_to_stream(quant_result, 1);

	printf("AFTER quant OUR Y packet, we have:\n"); 
	print_int_matrix(quant_result); 
      }	
      
      // Cr,Cb packets
      for(int is_Cb_packet = 0; is_Cb_packet < 2; is_Cb_packet++)
      { 
        last_network_packet_start_bit = bit_pos;  // Before we add the block, record where we are
        last_network_packet_start_byte = byte_pos;

                                                  // TODO: get the data and put it here (where input currently is).
        for(int j=0; j<8; j++ )
	       for(int k=0; k<8; k++ )
	       {
		   if(is_Cb_packet==1)
	       		input[j][k]=img_Cb[j][k+8*(col/2)];
	       	    else	
		        input[j][k]=img_Cr[j][k+8*(col/2)];
		}	
        mult_two_mats(DCT_temp_res, DCT_coeffs_trans, input);
        mult_two_mats(DCT_result, DCT_temp_res, DCT_coeffs);

        // ************************************* Quantization ***************************
        int i = 0; int j = 0;   int q = 0;       // TODO: inline this for loop

        for(i = 0; i < 8; i++)
            for(j = 0; j < 8; j++)
	      if(0)                            // The zero means we are working with chrominance
		quant_result[i][j] = quant_element(luminance_quant_bit_shift_table[i][j],DCT_result[i][j]);
	      else
		quant_result[i][j] = quant_element(chrominance_quant_bit_shift_table[i][j],DCT_result[i][j]);

        // ******************** Run-Level Encoding && Byte Packing **********************
	add_DC_coefficient_to_stream(quant_result[0][0],1,0);  // The zero means we are wrokgint with chrominance
	add_AC_coefficients_to_stream(quant_result, 0);  // The zero means we are working with chrominance
      }	
      
      //if(byte_pos >= 77)                           // We need to send one of our 80 byte specials (2 bytes for header information)
      //  send_network_packet();
 
      // TODO: WE ARE FORCING THE LAST NETWORK PACKET START BITS & BYTES!!! FIX THIS
      last_network_packet_start_bit = bit_pos;  // Before we add the block, record where we are
      last_network_packet_start_byte = byte_pos;
      printf( "frame_number = %d\r\n",col+(row*18) );
      send_network_packet(col+(row*18));                        // TODO: This will not always be frame 1!!!
   }
    }
    }
    printf("\n\nDONE ENCODING\n\n\n"); 
    
    //TODO: send_final_packet();

    //print_data_in_zig_zag_order(quant_result);

#ifndef EXCLUDE_DECODER
    // ************************************* Decode the block ***************************
    // Set up our tables
    lum_AC_table.populate_table_with_file("teds_VLC_tables_for_custom_luminance.txt");
    lum_DC_table.populate_table_with_file("DC_table_luminance.txt");
    chr_AC_table.populate_table_with_file("teds_VLC_tables_for_custom_chrominance.txt");
    chr_DC_table.populate_table_with_file("DC_table_chrominance.txt");

    //lum_AC_table.print_table();
    //lum_DC_table.print_table();

    extract_block_from_80_byte_buffer();          // Normally, we would want to get this from the network.
#endif 
    // NOTE: EVERYTHING BELOW HAS BEEN COPIED/ENHANCED INSIDE OF THE extract_block_from_80_byte_buffer() FUNCTION
    /* 
    initialize_decode_matrix(DCT_received_decoded);

    //memcpy(dec_outbuf, outbuf, cmu_cam_board_buffer_size);              // When we have an infinite buffer, we will just copy what we received.

    printf("\n********************\nStarting the decode process.... \n");

    int RL[2] = {999999, 9999999};
    int DC_level = 0;
    bool to_continue = true;

    //    for(int num_blocks = 0; num_blocks < IMG_Y/8; num_blocks++)
    for(int num_blocks = 0; num_blocks < 1; num_blocks++)            // TODO: This is to start, eventually this will be a while(more_data)
    {
    initialize_decode_matrix(DCT_received_decoded);

    if(use_dc_coeffs_in_stream == 1)
    {
    // DC coefficient for the block
    printf("We are starting to look at <%d,%d>\n",dec_byte_pos, dec_bit_pos);
    find_next_DC(!(num_blocks % (IMG_Y/8)), DC_level, is_luminance_test);
    add_run_level_to_decoded_matrix(DCT_received_decoded, -1, DC_level);
    }

    if(use_ac_coeffs_in_stream == 1)
    {
    // AC coefficients for the block
    to_continue = find_next_VLC(RL, is_luminance_test );
    add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

    while(to_continue )                       // Hmm... I'm not sure which order this evaluates
    {
    to_continue = find_next_VLC(RL, is_luminance_test);

    if(to_continue)
    add_run_level_to_decoded_matrix(DCT_received_decoded, RL[0], RL[1]);

    if((RL[0] == 0 && RL[1] == 0))
    to_continue = false;

    }

    if(compare_two_matricies(DCT_received_decoded, quant_result))
    printf("Success!\n");
    else
    {
    printf("The matricies differ.  This should never happen, find the bug.\n");
    print_data_in_zig_zag_order(DCT_received_decoded);
    print_data_in_zig_zag_order(quant_result);
    }
    }
    }

    //print_int_matrix(DCT_received_decoded);

    // below here is not tested
    for(i = 0; i < 8; i++)
    for(j = 0; j < 8; j++)
    if(is_luminance_test)
    recovered_pixels[i][j] = inv_quant_element(luminance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);
    else
    recovered_pixels[i][j] = inv_quant_element(chrominance_quant_bit_shift_table[i][j],DCT_received_decoded[i][j]);

    mult_two_mats(DCT_temp_res, DCT_coeffs, recovered_pixels);
    mult_two_mats(recovered_pixels, DCT_temp_res, DCT_coeffs_trans);

    //print_matrix(input);
    //print_matrix(recovered_pixels);
    */

    printf("bit_pos = %d, byte_pos = %d\n", bit_pos, byte_pos); 
    printf("The bus stops here <%d bytes later>!\n\n", byte_pos+1);

}


#ifndef EXCLUDE_DECODER
// Helper functions for the decoder.  This is an awful way to do business.  
int decoder_get_blocks_found()
{
  return blocks_found_in_network_packet; 
}

void decoder_get_first_block(float dec_img_Y_left1[8][8], float dec_img_Y_right1[8][8], float dec_img_Cr_1[8][8], float dec_img_Cb_1[8][8])
{
  memcpy(dec_img_Y_left1, img_Y_left1, sizeof(float)*64); 
  memcpy(dec_img_Y_right1, img_Y_right1, sizeof(float)*64); 
  memcpy(dec_img_Cr_1, img_Cr_1, sizeof(float)*64); 
  memcpy(dec_img_Cb_1, img_Cb_1, sizeof(float)*64); 
}

void decoder_get_second_block(float dec_img_Y_left2[8][8], float dec_img_Y_right2[8][8], float dec_img_Cr_2[8][8], float dec_img_Cb_2[8][8])
{
  memcpy(dec_img_Y_left2, img_Y_left2, sizeof(float)*64); 
  memcpy(dec_img_Y_right2, img_Y_right2, sizeof(float)*64); 
  memcpy(dec_img_Cr_2, img_Cr_2, sizeof(float)*64); 
  memcpy(dec_img_Cb_2, img_Cb_2, sizeof(float)*64); 
}

void decoder_get_third_block(float dec_img_Y_left3[8][8], float dec_img_Y_right3[8][8], float dec_img_Cr_3[8][8], float dec_img_Cb_3[8][8])
{
  memcpy(dec_img_Y_left3, img_Y_left3, sizeof(float)*64); 
  memcpy(dec_img_Y_right3, img_Y_right3, sizeof(float)*64); 
  memcpy(dec_img_Cr_3, img_Cr_3, sizeof(float)*64); 
  memcpy(dec_img_Cb_3, img_Cb_3, sizeof(float)*64); 
}


int  decoder_get_block_position()
{
  return block_position; 
}
#endif
