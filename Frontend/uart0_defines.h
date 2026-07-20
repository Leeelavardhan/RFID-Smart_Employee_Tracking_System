#include<LPC21xx.h>

#define TXD0 0
#define RXD0 2
#define DLAB 7
#define TEMT 6
#define RDR  0

#define BAUDRATE0(baud) U0DLL = (15000000/(16 * baud))
