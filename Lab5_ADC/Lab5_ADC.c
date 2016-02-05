#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LCD_WDATA PORTA
#define LCD_WINST PORTA
#define LCD_CTRL PORTG

void LCD_init(void);
void LCD_Command(char ch);
void LCD_Data(char ch);
void LCD_Pos(unsigned char row, unsigned char col);
void LCD_OutString(char *str);
void LCD_OutChar(char ch);
void LCD_Clear(void);
void Keypad_init(void);
void adc_init(void);
void displaytemp(void);
void displayASCII(void);
void SevenSeg_init(void);

unsigned char flag = 0;

void main(void){
//	SevenSeg_init();
//	_delay_ms(2);
	LCD_init();
	_delay_ms(2);
//	Keypad_init();
//	_delay_ms(2);
	adc_init();
	
	while(1){	
		ADCSRA |= 0x40;	//enable conversion	
		_delay_us(10);
		while(1){
			int tmp = ADCSRA;
			tmp &= 0x40;
			if(tmp == 0x00){
				 displaytemp();
				 break;
			}
		}LCD_OutChar('7');
		while(1);
	}
}

void displaytemp(void){
	int temp_16 = ADC;
	displayASCII();
	LCD_Pos(1,0);
	temp_16 &= 0x03FF;

	if(temp_16>=221 && temp_16<286){				//0 degrees <5
		LCD_OutString("approx 0degC");
	}
	if(temp_16>=286 && temp_16<395){				//10 5<x<15
		LCD_OutString("approx 10degC");
	}
	if(temp_16>=395 && temp_16<485){				//20 normal
		LCD_OutString("approx 20degC");
	}
	if(temp_16>=485 && temp_16<540){				//25 normal
		LCD_OutString("approx 25degC");
	}
	if(temp_16>=540 && temp_16<595){				//30 normal
		LCD_OutString("approx 30degC");
	}
	if(temp_16>=595 && temp_16<648){				//35 normal
		LCD_OutString("approx 35degC");
	}
	if(temp_16>=648 && temp_16<720){				//40 normal
		LCD_OutString("approx 40degC");
	}
	if(temp_16>=720 && temp_16<798){				//50
		LCD_OutString("approx 50degC");
	}
	if(temp_16>=798 && temp_16<859){				//60
		LCD_OutString("approx 60degC");
	}
	if(temp_16>=859 && temp_16<903){				//70
		LCD_OutString("approx 70degC");
	}
	if(temp_16>=903 && temp_16<932){				//80
		LCD_OutString("approx 80degC");
	}
}

void displayASCII(void){
	char temp1 = ADCL;
	char temp3 = ADCH;
	char temp2 = temp1;
	char temp4 = 0x30;	//0
	char tmp;

	temp3 &= 0x03;
	temp2 >> 4;
	temp2 &= 0x0F;
	temp1 &= 0x0F;

	temp3 +=0x30;
	
	tmp = temp2;
	if(tmp<0x0A)
		tmp += 0x30;
	else if(tmp>=0x0A){
		tmp -= 0x0A;
		tmp += 0x41;
	}temp2 = tmp;

	tmp = temp1;
	if(tmp<0x0A)
		tmp += 0x30;
	else if(tmp>=0x0A){
		tmp -= 0x0A;
		tmp += 0x41;
	}temp1 = tmp;
	
	
	LCD_OutChar(temp4);
	LCD_OutChar(temp3);
	LCD_OutChar(temp2);
	LCD_OutChar(temp1);
	LCD_OutChar(0x20);
}

ISR(INT0_vect){
	_delay_ms(100); //wait until glitches fade out (wait until interrupt becomes stable)
	unsigned char tmp;
	tmp = PIND;
	tmp &=0x01;
	if(tmp == 0){ //flag=1 only if the interrupt pin is low
		LCD_OutChar('z');
		flag = 1;
	}
}

void Keypad_init(void){
	DDRC = 0x15; //set input outputs of C pins
	PORTC = 0x00; //set outputs to ground ->needs to be 1 in order to block the interrupt
	SREG = 0x80;  //status bit -> enable external interrupt
	EICRA = 0x02; //interrupt enable at falling edge
	EIMSK = 0x01; //enable INT0
} 

void SevenSeg_init(void){
	DDRD = 0xFE;
	PORTD = 0xFF;
}

void LCD_OutString(char *str){
	while(*str != 0){
		LCD_Data(*str);
		str++;
	}
}

void LCD_OutChar(char ch){
	LCD_Data(ch);
	_delay_ms(2);
}

void LCD_Clear(void){
	LCD_Command(0x01); //LCD CLEAR
 	_delay_ms(2);
}

void LCD_init(void){
	DDRA = 0xFF;
	DDRG = 0xFF;
	LCD_Command(0x38);
	_delay_ms(2);
	LCD_Command(0x38);
	_delay_ms(2);
	LCD_Command(0x38);
	_delay_ms(2);
	LCD_Command(0x0D);
	_delay_ms(2);
	LCD_Command(0x01);
	_delay_ms(2);
	LCD_Command(0x06);
	_delay_ms(2);
}


void LCD_Command(char ch){
  	LCD_CTRL &= 0xf9; //write instruction
  	LCD_CTRL |= 0x01; //LCD enable
  	_delay_us(50);
  	LCD_WINST = ch; //write instruction
 	_delay_us(50);
 	LCD_CTRL &= 0xfe; //LCD disable
}


void LCD_Data(char ch){
 	LCD_CTRL |= 0x04; //set as DR
 	LCD_CTRL &= 0xfd; //write
 	LCD_CTRL |= 0x01; //LCD enable
 	_delay_us(50);
	 LCD_WDATA = ch; //write data
 	_delay_us(1200000);
 	LCD_CTRL &= 0xfe; //LCD disable
}

void LCD_Pos(unsigned char row, unsigned char col){ //LCD position
	LCD_Command(0x80 |(row*0x40+col));
	_delay_ms(1);
}

void adc_init(void){
	ADMUX = 0x41;	//ADLAR = 0	
	ADCSRA = 0x83;	// 1/8 system clock, adc enable, conversion disable
}

