#ifndef __MS8226DEC_H__
#define __MS8226DEC_H__

#define DEC_SEPARATOR ','
//#define DEC_SEPARATOR '.'

int ms8226_read(char chr);
void ms8226_convert(char *value_str);

#endif
