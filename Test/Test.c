#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

void main(void){
	DDRD=0xff;

	while(1){
		PORTD=0xff;
		_delay_ms(1000);
		PORTD=0x00;
		_delay_ms(1000);
	}
}
