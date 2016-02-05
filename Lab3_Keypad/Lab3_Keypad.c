//Mike Park
//Jun 24, 2014
//KMU ATMega128 Embedded Systems Lab(keypad)
//**************************************************************************************//
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LCD_WDATA PORTA //WRITE DATA
#define LCD_WINST PORTA //WRITE INSTRUCTION
#define LCD_RDATA PINA //READ DATA FROM LCD
#define LCD_CTRL PORTG //LCD CONTROL

void SevenSeg_init(void);
void LCD_init(void);
void LCD_Clear(void);
void LCD_OutChar(char ch);
void LCD_OutString(char *str);
void LCD_Command(char ch);
void LCD_Data(char ch);
void LCD_Pos(unsigned char row, unsigned char col);
void LCD_OutStringLShift();
void Keypad_init(void);
void KeyDisplay(void);
void KeyScan(void);
void Key_Pos(void);
                                                                                                                       
unsigned char flag = 0;
unsigned char buff = 0;
unsigned int keycount = 0;

void main(void){

	SevenSeg_init();
	_delay_ms(2);
	LCD_init();
	_delay_ms(2);
	Keypad_init();
	_delay_ms(2);

	while(1){
		PORTC = 0x11; //activate col1
		KeyDisplay();

		PORTC = 0x14; //activate col2
		KeyDisplay();

		PORTC = 0x05; //activate col3
		KeyDisplay();
	}
}

void KeyDisplay(void)
{
	_delay_ms(20); //delay for port output

	if(flag ==1){
		flag = 0;
		KeyScan();
		Key_Pos();
		LCD_OutChar(buff);

		unsigned char temp=0;
		while(!temp) //wait until the interrupt becomes high (print once when pressed once)
		{
			temp=PIND;
			temp &=0x01;
		}	
	}
}

void Key_Pos(void){ //prints on second row when first row is full, clears lcd and cursor returns to zero position when second row is full
	keycount++;
	if(keycount == 17){
		LCD_Pos(1,0);
		_delay_ms(2);
	}
	if(keycount ==33){
		keycount=1;
		LCD_Clear();
		LCD_Pos(0,0);
		_delay_ms(2);
	}
}


void KeyScan(void)
{
	unsigned char tmp = buff; //port test with sevenseg
	PORTD = tmp<<1;

	switch(buff) //deteremine input
	{
		case 0x79:
		buff = 49; //49 = 1 in ascii
		break;
		case 0x7C:
		buff = 50;
		break;
		case 0x6D:
		buff = 51;
		break;
		case 0x3B:
		buff = 52;
		break;
		case 0x3E:
		buff = 53;
		break;
		case 0x2F:
		buff = 54;
		break;
		case 0x5B:
		buff = 55;
		break;
		case 0x5E:
		buff = 56;	
		break;
		case 0x4F:
		buff = 57;
		break;
		case 0x73:
		buff = 42; //42=*
		break;
		case 0x76:
		buff = 48;
		break;
		case 0x67:
		buff = 35; //35=#
		break;
		default:
		buff = 101; //101=e
	}
}


ISR(INT0_vect){
	_delay_ms(100); //wait until glitches fade out (wait until interrupt becomes stable)

	unsigned char tmp;
	tmp = PIND;
	tmp &=0x01;
	if(tmp == 0){ //flag=1 only if the interrupt pin is low
		flag = 1;
		buff=PINC; //read input as soon as the interrupt is called (so read the data within the interrupt function)
		buff &=0x7f;
	}
}

void Keypad_init(void)
{
	DDRC = 0x15; //set input outputs of C pins
	PORTC = 0xFF; //set outputs to ground ->needs to be 1 in order to block the interrupt
	SREG = 0x80;  //status bit -> enable external interrupt
	EICRA = 0x02; //interrupt enable at falling edge
	EIMSK = 0x01; //enable INT0
}

void LCD_init(void){
	  DDRA = 0xFF;        //set all ports A and G as outputs
	  DDRG = 0x07;
	  LCD_Command(0x38); //set LCD as 2line 5*7dot
	  _delay_ms(2);
	  LCD_Command(0x38);
	  _delay_ms(2);
	  LCD_Command(0x38);
	  _delay_ms(2);
	  LCD_Command(0x0e); //display on, cursur on, no blinking
	 _delay_ms(2);		  
	 LCD_Command(0x01); //LCD CLEAR
  	_delay_ms(2);
  	LCD_Command(0x06); //increment cursor by one to right (entry mode set)
  	_delay_ms(2);        
}

void LCD_Clear(void){
	LCD_Command(0x01); //LCD CLEAR
	_delay_ms(2);
}

void SevenSeg_init(void){
	DDRD = 0xFE;
	PORTD = 0xFF;
}


void LCD_OutChar(char ch){
	LCD_Data(ch);
	_delay_ms(2);
}


void LCD_OutString(char *str){
	while(*str != 0){
		LCD_Data(*str);
		str++;
	}
}


void LCD_OutStringLShift(char *str){ //LCD_OutString_Left_Shift
	char* i = str;
	i++;
	while(*(i++) != 0){
		LCD_Data(*str);
		_delay_ms(30);
		LCD_Command(0x18);
		_delay_ms(30);
		str++;
	}LCD_OutChar(*str);
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
