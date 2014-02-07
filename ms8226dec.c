/**
 * ms8226 module .. convert data from ms8226 multimeter into readable form
 *
 * author: ondrejh.ck@email.cz
 * date: 7.2.2014
 *
 * interface functions:
 *
 *   uin8_t ms8226_read(char chr) .. stores data from multimeter into local buffer
 *      when all data read correctly (buffer filled) returns 1 otherwise returns 0
 *
 *   void ms8226_convert(char *value_str) .. converts internal buffer data into string
 *      containing value in 00,00e0/-0 form. decimal separator can be changed by changing
 *      define in header file
 *
 * usage example:
 *
 *   for (;;)
 *   {
 *     char chr;
 *     if (serial_getchar(&chr)) // if byte from serial port
 *     {
 *       if (ms8226_read(chr))           // put it into buffer and if all data read
 *       {
 *         ms8226_convert(str);          // convert buffer
 *         printf("%s\n",str);           // show result
 *         break;                        // break if don't want another values
 *       }
 *     }
 *   }
 *
 **/

#include <inttypes.h>
#include "ms8226dec.h"

// constants (lcd conversion table)
const uint8_t digit_data[12] = {0x7D,0x05,0x5B,0x1F,0x27,0x3E,0x7E,0x15,0x7F,0x3F,0x68,0x00};
const uint8_t digit_chr [12] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'L', ' '};

// internal buffer
uint8_t rawdata[8];

// use lcd conversion table (local function)
char byte2char(uint8_t b)
{
    int i;
    for (i=0;i<12;i++) if (b==digit_data[i]) return digit_chr[i];
    return 'X';
}

// put data into buffer, check if correct and return 1 if all data read (otherwise return 0)
int ms8226_read(char chr)
{
    uint8_t d = chr&0x0F;
    uint8_t p = (chr&0xF0)>>4;

    static uint8_t cnt=0,error=0;

    if (p==1)
    {
        cnt = 0;
        error = 0;
        rawdata[0] = 0;
    }

    cnt++;
    if (p!=cnt) error ++;
    else
    {
        if (p&0x01)
            rawdata[p>>1]|=d;
        else
            rawdata[p>>1]=(d<<4);
    }

    if ((p==14)&&(error==0))
    {
        return 1;
    }

    return 0;
}

// convert internal buffer into readable string containing value
void ms8226_convert(char *value_str)
{
    if (rawdata[1]&0x80) *value_str++='-';   // -
    *value_str++=byte2char(rawdata[1]&0x7F); // 0..9
    if (rawdata[2]&0x80) *value_str++=DEC_SEPARATOR;   // ,
    *value_str++=byte2char(rawdata[2]&0x7F); // 0..9
    if (rawdata[3]&0x80) *value_str++=DEC_SEPARATOR;   // ,
    *value_str++=byte2char(rawdata[3]&0x7F); // 0..9
    if (rawdata[4]&0x80) *value_str++=DEC_SEPARATOR;   // ,
    *value_str++=byte2char(rawdata[4]&0x7F); // 0..9
    if (rawdata[5]&0xEA) // unK m M
    {
        *value_str++='e';
        if (rawdata[5]&0xC8) *value_str++='-'; // un  m
        if (rawdata[5]&0x80) *value_str++='6'; // u
        if (rawdata[5]&0x40) *value_str++='9'; //  n
        if (rawdata[5]&0x20) *value_str++='3'; //   K
        if (rawdata[5]&0x08) *value_str++='3'; //     m
        if (rawdata[5]&0x02) *value_str++='6'; //       M
    }
    *value_str++='\0';
}
