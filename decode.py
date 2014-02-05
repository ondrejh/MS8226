

''' program receiving and decoding data from MS8226 multimeter
Serial protocol documentation thanks to http://rahmyzdhyfbr.tripod.com/

author: ondrejh.ck@email.cz
date: 5.2.2014
'''

''' 
Mastech MS8226 serial protocol

2400 8n1
14 Bytes , watch out for serial IR receiver sending junk 15byte strings
First four bits of each byte are position in message , from 1-14
Last four bits are data , each bit represents one element on LCD

  4     3     2     1     __a_
 __    __    __    __    |    |
|  |  |  |  |  |  |  |   f    b
|__|  |__|  |__|  |__|   |_g__|
|  |  |  |  |  |  |  |   |    |
|__| .|__| .|__| .|__|   e    c
    P3    P2    P1       |__d_|

Digits
0          10           20           30         40        50   |
01234 5678901 2 3456789 0 1234567 8 9012345 67890123456789012345
00110 1111101 0 0011111 0 0100111 1 1111111 00000000000000000100 034.8
00110 1111101 0 1011011 0 0111110 1 0011111 00000000000000000100 025.3
00110 1111101 0 1011011 0 0100111 1 0011111 00000000000000000100 024.3
00110 1111101 0 1011011 0 0000101 1 0111111 00000000000000000100 021.9
00110 1111101 0 1011011 0 0000101 1 0010101 00000000000000000100 021.7
00110 1111101 0 1011011 0 0000101 1 1111110 00000000000000000100 021.6
||||| 4444444 P 3333333 P 2222222 P 1111111 ||||||||||||||||||||
      efadcgb 3 efadcgb 2 efadcgb 1 efadcgb    d i s p l a y

Display
0         10        20        30        40        50   |
01234567890123456789012345678901234567890123456789012345
00110111110100000101111111010000010101000000100000001000 nF
00110101101110000101000001010101101110000000100000001000 uF
01110111110111111101011111010111110100000000000010001000 A
01110111110101111101111111010111110100001000000010001000 mA
01110111110101111101011111011111110110000000000010001000 uA
00110111110101011011001111101101101100000000000000000100 C
00010000010100000101001111111111111100000000000000101000 Hz
00010111110101111101011111101001111100000100000000001000 %
00010000000001111101011010001000000000000000010000001000 Ohm
00010000000011111101011010000000000000100000010000001000 KOhm
00010000000011111101011010000000000000000010010000001000 MOhm
00010000000011111101011010000000000000010000000001001000 Diode
00010000000001111101011010001000000000000001010000001000 Beep
01011111110100000101000001011011111000001000000001001000 mV
01111111110111111101011111010111110100000000000001001000 V
01110111110111111101011111010111110100000000000101001000 Hold
01011111110111111101011111010111110100000000001001001000 Rel
ADAR-|||||||||||||||||||||||||||||||unKDm%MBFORHAVHXXCXX
CCUS             d i g i t s           i   e hEo  z
  T2                                   o   e mLl
  O3                                   d   p   d
   2                                   e
   
51 is probably low battery
'''
from serial import Serial

portname = 'COM3'

#              0    1    2    3    4    5    6    7    8    9    L    
const_seg = [0x5F,0x50,0x6D,0x7C,0x72,0x3E,0x3F,0x54,0x7F,0x7E,0x0B,0x00]
const_chr = [ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'L',  '']

def decode_segment(sdata):
    ''' decode one 7 segment number
    argument: 7bit unsigned number (0..127)
    returns: '0'..'9','L','' '''
    for i in range(len(const_seg)):
        if sdata==const_seg[i]:
            return const_chr[i];
    return ''

def decode_ms8226_data(data):
    s = ''
    # value
    if (((data>>4)&0x01)!=0): # - sign (4th bit)
        s=s+'-'
    s=s+decode_segment((data>>5)&0x7F) #first number
    if (((data>>12)&0x01)!=0): # first decimal divider
        s=s+','
    s=s+decode_segment((data>>13)&0x7F) #second number
    if (((data>>20)&0x01)!=0): # second decimal divider
        s=s+','
    s=s+decode_segment((data>>21)&0x7F) #third number
    if (((data>>28)&0x01)!=0): # third decimal divider
        s=s+','
    s=s+decode_segment((data>>29)&0x7F) #fourth number
    # units
    if (((data>>36)&0x01)!=0): # micro
        s=s+'u'
    if (((data>>37)&0x01)!=0): # nano
        s=s+'n'
    if (((data>>38)&0x01)!=0): # kilo
        s=s+'k'
    if (((data>>40)&0x01)!=0): # mili
        s=s+'m'
    if (((data>>41)&0x01)!=0): # %
        s=s+'%'
    if (((data>>42)&0x01)!=0): # mega
        s=s+'M'
    if (((data>>44)&0x01)!=0): # Farad
        s=s+'F'
    if (((data>>45)&0x01)!=0): # Ohm
        s=s+'Ohm'
    if (((data>>48)&0x01)!=0): # Amper
        s=s+'A'
    if (((data>>49)&0x01)!=0): # Volt
        s=s+'V'
    if (((data>>50)&0x01)!=0): # Hz
        s=s+'Hz'
    if (((data>>53)&0x01)!=0): # °C
        s=s+'°C'
    s=s+' '
    # AC/DC/Diode/Beep
    if (((data>>0)&0x01)!=0): # AC
        s=s+'AC'
    if (((data>>1)&0x01)!=0): # DC
        s=s+'DC'
    if (((data>>39)&0x01)!=0): # Diode
        s=s+'Diode'
    if (((data>>43)&0x01)!=0): # Beep
        s=s+'Beep'
    # Rel,Hold,Auto
    if (((data>>46)&0x01)!=0): # Rel
        s=s+' Rel'
    if (((data>>47)&0x01)!=0): # Hold
        s=s+' Hold'
    if (((data>>2)&0x01)!=0): # Auto
        s=s+' Auto'
    return s

def swap_4bit_nibbles(d):
    r = 0
    for i in range(4):
        if d&(1<<i):
            r |= (1<<(3-i))
    return r


''' main program:
Reads data from serial port. When all 14 byte from MS8226 multimeter
data is received well, tryies to decode it and print output'''

with Serial(portname,2400,timeout=0.1) as port:
    
    cnt = 0 #receive byte conter
    data = 0 #display raw data
    error = False #error flag
    
    while(True):
        s = port.read()
        for b in s:
            # split byte into data and count nibbles
            p = (int(b)>>4)
            d = swap_4bit_nibbles(int(b)&0x0F)

            # if its first byte reset data and variables
            if p == 1:
                data = 0
                cnt = 0
                error = False

            # test if bytes receiving sequence fits
            cnt += 1
            if cnt != p:
                error = True
            else:
                # add received nibbles (4bit) into data word (64bit)
                data = data | (d<<((p-1)*4))

            #if last byte received and all bytes received well than decode
            if (p == 14) and (error!=True)
                print(decode_ms8226_data(data))
