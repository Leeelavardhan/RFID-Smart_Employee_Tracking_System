#include "delay.h"

void delay_s(unsigned int d_s){
	d_s *= 12000000;
	while(d_s--);
}

void delay_ms(unsigned int d_ms){
	d_ms *= 12000;
	while(d_ms--);
}

void delay_us(unsigned int d_us){
	d_us *= 12;
	while(d_us--);
}
