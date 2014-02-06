
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <inttypes.h>
#include "ms8226dec.h"


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
            //printf("0x%02X\n",(uint8_t)(chr));
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

    return 0;
}
