#include <inttypes.h>
#include "ms8226dec.h"

const uint8_t digit_data[12] = {0x7D,0x05,0x5B,0x1F,0x27,0x3E,0x7E,0x15,0x7F,0x3F,0x68,0x00};
const uint8_t digit_chr [12] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'L', ' '};

char byte2char(uint8_t b)
{
    int i;
    for (i=0;i<12;i++) if (b==digit_data[i]) return digit_chr[i];
    return 'X';
}

int convert_ms8226_value(char chr, char *value_str)
{
    uint8_t d = chr&0x0F;
    uint8_t p = (chr&0xF0)>>4;

    static uint8_t cnt=0,error=0;
    static uint8_t dbuf[8];

    if (p==1)
    {
        cnt = 0;
        error = 0;
        dbuf[0] = 0;
    }

    cnt++;
    if (p!=cnt) error ++;
    else
    {
        if (p&0x01)
            dbuf[p>>1]|=d;
        else
            dbuf[p>>1]=(d<<4);
    }

    if ((p==14)&&(error==0))
    {
        if (dbuf[1]&0x80) *value_str++='-';   // -
        *value_str++=byte2char(dbuf[1]&0x7F); // 0..9
        if (dbuf[2]&0x80) *value_str++=',';   // ,
        *value_str++=byte2char(dbuf[2]&0x7F); // 0..9
        if (dbuf[3]&0x80) *value_str++=',';   // ,
        *value_str++=byte2char(dbuf[3]&0x7F); // 0..9
        if (dbuf[4]&0x80) *value_str++=',';   // ,
        *value_str++=byte2char(dbuf[4]&0x7F); // 0..9
        if (dbuf[5]&0xEA) // unK m M
        {
            *value_str++='e';
            if (dbuf[5]&0xC8) *value_str++='-'; // un  m
            if (dbuf[5]&0x80) *value_str++='6'; // u
            if (dbuf[5]&0x40) *value_str++='9'; //  n
            if (dbuf[5]&0x20) *value_str++='3'; //   K
            if (dbuf[5]&0x08) *value_str++='3'; //     m
            if (dbuf[5]&0x02) *value_str++='6'; //       M
        }
        //*value_str++='\n';
        *value_str++='\0';
        //sprintf(value_str,"data: %02X %02X %02X %02X %02X %02X %02X %02X\n",dbuf[0],dbuf[1],dbuf[2],dbuf[3],dbuf[4],dbuf[5],dbuf[6],dbuf[7]);
        return 1;
    }

    return 0;
}

