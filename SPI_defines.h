#include <LPC21xx.h>

#define PULL_CS IOCLR0 = (1<<7);
#define PUSH_CS IOSET0 = (1<<7);
#define READ_MODE         0x03
#define WRITE_MODE        0x02
#define WRITE_ENABLE      0x06
#define WRITE_DISABLE     0x04
