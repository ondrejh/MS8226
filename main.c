
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <inttypes.h>

#define MAX_PORTNAME_LENGTH 31
#define MAX_BUFFER_LENGTH 31

int serial_getchar(HANDLE serial,char *c)
/** getchar from serial port
 * return 1 .. one char read
 *        0 .. none read
 *        -1 .. error occured */
{
    DWORD dwBytesRead = 0;
    if (!ReadFile(serial,c,1,&dwBytesRead,NULL))
    {
        // get char error
        return -1;
    }
    if (dwBytesRead==1) return 1;
    return 0;
}

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

int main()
{
    // serial port
    HANDLE hSerial;
    char PortName[MAX_PORTNAME_LENGTH+1];
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;

    strcpy(PortName,"COM3");

    /// open port
    hSerial = CreateFile(PortName,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
            //serial port does not exist. Inform user.
            printf("Serial port %s doesn't exist!\n",PortName);
            return -1;
        }
        //some other error occurred. Inform user.
        printf("Serial port %s openning error (other than it doesn't exist)!\n",PortName);
        return -1;
    }

    /// set port params
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        //error getting state
        printf("Get port state error (%s)!\n",PortName);
        return -1;
    }
    dcbSerialParams.BaudRate=CBR_2400;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if(!SetCommState(hSerial, &dcbSerialParams))
    {
        //error setting serial port state
        printf("Set port state error (%s)!\n",PortName);
        return -1;
    }

    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(hSerial, &timeouts))
    {
        //error occureed. Inform user
        printf("Set port timeouts error (%s)!\n",PortName);
        return -1;
    }

    char str[256] = "";
    for (;;)
    {
        char chr;
        if (serial_getchar(hSerial,&chr))
        {
            printf("0x%02X\n",(uint8_t)(chr));
            if (convert_ms8226_value(chr,str))
            {
                printf("%s\n",str);
                break;
            }
        }
    }

    // close port
    if (CloseHandle(hSerial)==0)
    {
        printf("Close handle error!\n");
        return -1;
    }

    printf("Hello world!\n");
    return 0;
}
