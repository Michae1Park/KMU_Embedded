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
void adc_init(void);
void displaytemp(void);
void displayASCII(int temp_16);
char* calibtemp(int temp_16);	

void main(void)
{
	unsigned char tmp;

	sys_init();
	
	while(1)
	{	
		ADCSRA |= 0x40;	//enable conversion	
		_delay_ms(1);

		tmp=1;
		while(tmp)
		{
			tmp = ADCSRA;
			tmp &= 0x40;
		}
		displaytemp();

	}
}

void sys_init(void)
{
	LCD_init();
	_delay_ms(2);
	adc_init();
}
void displaytemp(void)
{
	int temp_16;
	char* a;

	LCD_Pos(0,0);	
	temp_16 = ADC & 0x03FF;
	displayASCII(temp_16);
	LCD_Pos(1,0);

	a = calibtemp(temp_16);

	LCD_OutString("approx ");
	LCD_OutString(a);
	LCD_OutString("degreeC");
}

char* calibtemp(int temp_16){
	char* array;

	if(temp_16>=763 && temp_16<802){ //0
		array = "0";	//"" 쓸땐 시작 주소부터 따옴표안에있는 데이터를 넣어줌->so has to be: address = ""
	}
	if(temp_16>=712 && temp_16<763){ //5
		array = "5";
	}
	if(temp_16>=655 && temp_16<712){//10
		array = "10";
	}
	if(temp_16>=629 && temp_16<655){//15
		array = "15";
	}
	if(temp_16>=614 && temp_16<629){	//16
		array = "16";
	}
	if(temp_16>=600 && temp_16<614){	//17
		array = "17";
	}
	if(temp_16>=586 && temp_16<600){	//18
		array = "18";
	}
	if(temp_16>=571 && temp_16<586){	//19
		array = "19";
	}
	if(temp_16>=556 && temp_16<571){	//20
		array = "20";
	}
	if(temp_16>=545 && temp_16<556){	//21
		array = "21";
	}
	if(temp_16>=534 && temp_16<545){	//22
		array = "22";
	}
	if(temp_16>=523 && temp_16<534){	//23
		array = "23";
	}
	if(temp_16>=512 && temp_16<523){	//24
		array = "24";
	}
	if(temp_16>=499 && temp_16<512){	//25
		array = "25";
	}
	if(temp_16>=488 && temp_16<499){	//26
		array = "26";
	}
	if(temp_16>=477 && temp_16<488){	//27
		array = "27";
	}
	if(temp_16>=466 && temp_16<477){	//28
		array = "28";
	}
	if(temp_16>=455 && temp_16<466){	//29
		array = "29";
	}
	if(temp_16>=440 && temp_16<455){	//30
		array = "30";
	}
	if(temp_16>=436 && temp_16<440){	//31
		array = "31";
	}
	if(temp_16>=432 && temp_16<436){	//32
		array = "32";
	}
	if(temp_16>=428 && temp_16<432){	//33
		array = "33";
	}
	if(temp_16>=423 && temp_16<428){	//34
		array = "34";
	}
	if(temp_16>=418 && temp_16<423){	//35
		array = "35";
	}
	if(temp_16>=413 && temp_16<418){	//36
		array = "36";
	}
	if(temp_16>=392 && temp_16<413){	//37
		array = "37";
	}
	if(temp_16>=371 && temp_16<392){	//38
		array = "38";
	}
	if(temp_16>=350 && temp_16<371){	//39
		array = "39";
	}
	if(temp_16>=328 && temp_16<350){	//40
		array = "40";
	}
	if(temp_16>=283 && temp_16<328){	//45
		array = "45";
	}
	if(temp_16>=244 && temp_16<283){	//50
		array = "50";
	}
	if(temp_16>=209 && temp_16<244){	//55
		array = "55";
	}
	if(temp_16>=179 && temp_16<209){	//60
		array = "60";
	}
	if(temp_16>=153 && temp_16<179){	//65
		array = "65";
	}
	if(temp_16>=131 && temp_16<153){	//70
		array = "70";
	}
	if(temp_16>=112 && temp_16<131){	//75
		array = "75";
	}
	if(temp_16>=93 && temp_16<112){	//80
		array = "80";
	}
	return array;
}

void displayASCII(int temp_16)
{
	unsigned char temp[3];
	unsigned char asc_hex[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46};
	int	i;
	
	i=temp_16;
	
	i=i>>8;
	temp[0]=asc_hex[i];

	i=temp_16 & 0x00f0;
	i=i>>4;
	temp[1]=asc_hex[i];

	i=temp_16 & 0x000f;
	temp[2]=asc_hex[i];

	for(i=0; i<3; i++)
	{
		LCD_OutChar(temp[i]);
	}
	
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

