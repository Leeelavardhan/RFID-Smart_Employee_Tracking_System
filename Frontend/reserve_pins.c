#include <LPC21xx.h>
#include "types.h"

void ReservePins(ui32 port,ui32 pins,ui32 from,ui32 dir){
	if(port == 0){
		while(pins--){
			if((from < 32)){
					if(dir){
						IODIR0 |= (1 << from);
					}
					else{
						IODIR0 &= ~(1 << from);
					}
					from++;
			}
		}
	}
	else if(port == 1){
		while(pins--){
			if((from > 15) && (from < 32)){
					if(dir){
						IODIR1 |= (1 << from);
					}
					else{
						IODIR1 &= ~(1 << from);
					}
					from++;
			}
		}
	}
}
