//Mike Park
//July 26, 2014
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

void System_init(void);
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
void Timer16_init(void);

void CountClock(void);
void ReadCol(void);
void ReadRow(void);
void hmscheck(void);
void changeAPM(void);
void convertAscii(void);
void displaytime(unsigned char a, unsigned char b, int c);
void incrementTime(void);
void decrementTime(void);
void Rshift(void);


//keypad global                                                                                                                       
unsigned char buff = 0;
unsigned char exit = 0;

//16bit Timer global
unsigned int TimeArray[61];
unsigned int* hour;
unsigned int* min;
unsigned int* sec;
unsigned int cf=0;
unsigned int p=0;
unsigned char APM;

void main(void){
    
    System_init();

    for(int i=0; i<61; i++){    
        TimeArray[i] = i;
    }

    LCD_OutString("AM 09:00:00");    //default AM 09:00 
    sec = (TimeArray);
	min = (TimeArray);
	hour = (TimeArray+9);
	APM = 'A';

    while(1){    
        OCR1A = 31250; 
    }
}

ISR(TIMER1_COMPA_vect){
    CountClock();
}

void CountClock(void){
    sec++;
    hmscheck();
}

void hmscheck(void){
    if(*sec==60){
        min++;
        sec=TimeArray;
		cf=1;
    }
    if(*min==60){
        hour++;
      	if(*hour==12){changeAPM();}	   
	    min=TimeArray;
        cf=2;
    }
	if(*hour==13){
		hour=(TimeArray+1);
		cf=2;
	}
    convertAscii();
}

void changeAPM(void){
	if(APM=='P'){APM='A';}
	else{APM='P';}
	LCD_Pos(0,0);
	LCD_OutChar(APM);
}

void convertAscii(void){
    unsigned char AsciiArray[] = {'0','1','2','3','4','5','6','7','8','9'};
	unsigned int a, b;
    
	if((cf>=0) || (p==3)){
	    a = (*sec)/10;
	    b = (*sec)%10;
		displaytime(AsciiArray[a], AsciiArray[b], 0);
	}
    if((cf>=1) || (p==2)){
        a = *min/10;
        b = *min%10;
        displaytime(AsciiArray[a], AsciiArray[b], 1);
    }

    if((cf>=2) || (p==1)){
		cf=0;
        a = *hour/10;
        b = *hour%10;
        displaytime(AsciiArray[a], AsciiArray[b], 2);
    }
}

void displaytime(unsigned char a, unsigned char b, int c){
    if(c==0){
        LCD_Pos(0,10);
        LCD_OutChar(b);
        LCD_Pos(0,9);
        LCD_OutChar(a);
    }
    if(c==1){
        LCD_Pos(0,7);
        LCD_OutChar(b);
        LCD_Pos(0,6);
        LCD_OutChar(a);
    }
    if(c==2){
        LCD_Pos(0,4);
        LCD_OutChar(b);
        LCD_Pos(0,3);
        LCD_OutChar(a);
    }
}
ISR(INT0_vect){
    _delay_ms(100); //wait until glitches fade out (wait until interrupt becomes stable)

    unsigned char tmp;
    tmp = PIND;
    tmp &=0x01;
    if(tmp == 0){ //flag=1 only if the interrupt pin is low
        buff=PINC; //read input as soon as the interrupt is called (so read the data within the interrupt function)
        buff &=0x7f;
        if(buff == 0x67){
            TCCR1B = 0x08; //block clock source
            ReadCol();
            TCCR1B = 0x0C;    //provide clock source 
            PORTC = 0xEF;    //set col3 back to 0
        }
    }
}

void ReadCol(void){
    LCD_Pos(0,0);		//default position
	LCD_Command(0x0E);	//cursor on

    while(1){
        PORTC = 0xFB;
        ReadRow();
        if(exit == 1){
            exit = 0;
            break;
        }
        
        PORTC = 0xFE;
        ReadRow();
        if(exit == 1){
            exit = 0;
            break;
        }

        PORTC = 0xEF;
        ReadRow();
        if(exit == 1){
            exit = 0;
            break;
        }    
    }    
}

void ReadRow(void){
    unsigned char temp=0;
    char tmp = PINC;
    tmp &= 0x7F;
    
    if(tmp == 0x79){        //1
        incrementTime();
    }
    else if(tmp == 0x7C){    //2
        decrementTime();
    }
    else if(tmp == 0x6D){    //3
        Rshift();    
    }
    else if(tmp == 0x73){    //42=*
		exit = 1; 
		p=0;				//initialize shift position back to 0 -> APM doesn't depend on convertAscii function 
		LCD_Command(0x0C);	//cursur off
    }
    
    while(!temp)     //wait until the interrupt becomes high (print once when pressed once)
    {
        temp=PIND;
        temp &=0x01; 
    }
}

void incrementTime(void){    
	if(p==1){
		hour++;
		if(*hour==13){hour=(TimeArray+1);}	
	}
	if(p==2){
		min++;
		if(*min==60){min=TimeArray;}
	}
	if(p==3){
		sec++;
		if(*sec==60){sec=(TimeArray);}
	}
	convertAscii();
	
	if(p==0){
		changeAPM();
	}
}

void decrementTime(void){
    if(p==1){
		hour--;
		if(*(hour+1)==1){hour=(TimeArray+12);}	
	}
	if(p==2){
		min--;
		if(*(min+1)==0){min=(TimeArray+59);}
	}
	if(p==3){
		sec--;
		if(*(sec+1)==0){sec=(TimeArray+59);}
	}
	convertAscii();
	
	if(p==0){
		changeAPM();
	}
}

void Rshift(void){
    int pos[] = {0,4,7,10};
    p++;
    if(p==4){p=0;}
    LCD_Pos(0,pos[p]);    
}

   
void System_init(void){
    SevenSeg_init();
    _delay_ms(2);
    LCD_init();
    _delay_ms(2);
    Keypad_init();
    _delay_ms(2);
    Timer16_init();
    _delay_ms(2);
}

void Timer16_init(void){
    TCCR1A = 0x00;    // CTC mode, ignore else
    TCCR1B = 0x0C; // 256분주, ignore capture, OCR
    SREG = 0x80;
    TIMSK = 0x10; //CTC interrupt enable, ignore else
    TIFR = 0x10; //TCNT1 OCR1A 비교일치 interrupt
}

void Keypad_init(void)
{
    DDRC = 0x15; //set input outputs of C pins
    PORTC = 0xEF; //set outputs to ground ->needs to be 1 in order to block the interrupt: set col3 to 0
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
    LCD_Command(0x0C); //display on, cursur off, no blinking
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
      _delay_us(5000);
      LCD_CTRL &= 0xfe; //LCD disable
}

void LCD_Pos(unsigned char row, unsigned char col){ //LCD position
    LCD_Command(0x80 |(row*0x40+col));
    _delay_ms(1);
}

void SevenSeg_init(void){
    DDRD = 0xFE;
    PORTD = 0xFF;
}
