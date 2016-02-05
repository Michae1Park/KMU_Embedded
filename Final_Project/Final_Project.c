//Mike Park
//August 08, 2014
//KMU ATMega128 Embedded Systems Lab
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
void Keypad_init(void);
void ReadSharp(void);
void ReadStar(void);
void SelectFunc(void);
void DoSevenseg(void);
unsigned int ReadCol(unsigned int mode);
unsigned int ReadRow(void);
void adc_init(void);
void Dotempsensor(void);
void displaytemp(void);
char* calibtemp(int temp_16);
void displayASCII(int temp_16);
void Dobuzzer(void);
void Timer2_init(void);
void Dokeypad(void);
void KeyDisplay(void);
void Key_Pos(void);
void KeyScan(void);
void Timer16_init(void);
void Doclock(void);
void CountClock(void);
void hmscheck(void);
void changeAPM(void);
void convertAscii(void);
void displaytime(unsigned char a, unsigned char b, int c);
void clockCol(void);
void clockRow(void);
void incrementTime(void);
void decrementTime(void);
void Rshift(void);
                                                                                                                       
volatile unsigned char flag = 0;
volatile unsigned char buff = 0;
volatile unsigned char keycount = 0;

volatile unsigned char exit = 0;

//16bit Timer global
volatile unsigned int TimeArray[61];
volatile unsigned int* hour;
volatile unsigned int* min;
volatile unsigned int* sec;
volatile unsigned int cf=0;
volatile unsigned int p=0;
volatile unsigned char APM;

void main(void){

	System_init();

	while(1){
		LCD_Clear();
		LCD_OutString("Press # to Start");
		_delay_ms(2);
		LCD_Pos(1,0);
	
		ReadSharp();
		ReadStar();
		SelectFunc();
	}
}

void SelectFunc(void){	//case by case 되야함
	unsigned int index = ReadCol(0);
	
	if(index==1){DoSevenseg();}	//else if index=2,3,4,5 일때도 다 해줘야함
	if(index==2){Dotempsensor();}
	if(index==3){Dobuzzer();}
	if(index==4){Dokeypad();}
	if(index==5){Doclock();}
}

unsigned int ReadCol(unsigned int mode){
   unsigned int index=0;
   
   while(1){
        PORTC = 0xFB;	//col1
        index = ReadRow();
        if((index>0) || (mode==2)){break;}

        PORTC = 0xFE;	//col2
        index = ReadRow();
		if(index>0){break;}

        PORTC = 0xEF;	//col3
        index = ReadRow();
    	if(index>0){break;}
	}    
	return index;
}

unsigned int ReadRow(void){
	unsigned int index=0;

	if(flag==1){
		flag=0;

	    unsigned char temp=0;
    	char tmp = PINC;
    	tmp &= 0x7F;

		if(tmp==0x79){index=1;}
		if(tmp==0x7C){index=2;}
		if(tmp==0x6D){index=3;}
		if(tmp==0x3B){index=4;}
		if(tmp==0x3E){index=5;}
		if(tmp==0x73){index=10;}  //if * then return 10	  

   	 	while(!temp)     //wait until the interrupt becomes high (print once when pressed once)
  	 	{
        	temp=PIND;
        	temp &=0x01; 
    	}
	}
	return index;
}

void Timer16_init(void){
    TCCR1A = 0x00;    // CTC mode, ignore else
    TCCR1B = 0x0C; // 256분주, ignore capture, OCR
    SREG = 0x80;
    TIMSK = 0x10; //CTC interrupt enable, ignore else
    TIFR = 0x10; //TCNT1 OCR1A 비교일치 interrupt
}

void Doclock(void){
	unsigned int checkend=0;

	LCD_Clear();
	Timer16_init();
	_delay_ms(2);

	for(int i=0; i<61; i++){    
        TimeArray[i] = i;
    }

    LCD_OutString("AM 09:00:00");    //default AM 09:00 
    sec = (TimeArray);
	min = (TimeArray);
	hour = (TimeArray+9);
	APM = 'A';
    
	while(1){ 
		PORTC = 0x05;
		_delay_ms(2);
	   
		if(flag==1 && buff==0x67){
			flag=0;
			TCCR1B = 0x08; //block clock source
            clockCol();
            TCCR1B = 0x0C;    //provide clock source 
            PORTC = 0xEF;    //set col3 back to 0
		}

        OCR1A = 31250; 

		checkend = ReadCol(2);
		if(checkend==10){
			TCCR1B = 0x08; //need to turn the timer off
			break;
		}
    }
}


void clockCol(void){
    LCD_Pos(0,0);		//default position
	LCD_Command(0x0E);	//cursor on

    while(1){
        PORTC = 0xFB;
        clockRow();
        if(exit == 1){
            exit = 0;
            break;
        }
        
        PORTC = 0xFE;
        clockRow();
        if(exit == 1){
            exit = 0;
            break;
        }

        PORTC = 0xEF;
        clockRow();
        if(exit == 1){
            exit = 0;
            break;
        }    
    }    
}

void clockRow(void){
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

void Dokeypad(void){
	LCD_Clear();

	while(1){
		PORTC = 0x11; //activate col1
		KeyDisplay();
		if(buff==42){break;}				

		PORTC = 0x14; //activate col2
		KeyDisplay();
		if(buff==42){break;}

		PORTC = 0x05; //activate col3
		KeyDisplay();
		if(buff==42){break;}
	}
}


void KeyDisplay(void)
{
	_delay_ms(20); //delay for port output

	if(flag == 1){
		flag = 0;
		KeyScan();
		Key_Pos();
		if(!(buff==42 || buff==35)){
			LCD_OutChar(buff);
		}

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
	if(keycount == 33){
		keycount=1;
		LCD_Clear();
		LCD_Pos(0,0);
		_delay_ms(2);
	}
}

void KeyScan(void)
{
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

void Dobuzzer(void){
	unsigned int checkend = 0;
	char array[] = {28, 25, 22, 21, 18, 16, 14, 13};
	DDRB=0x80;	//sound on	

	while(1){
		for(int i=0; i<8; i++){
			OCR2 = array[i];
			for(int j=0; j<25000; j++){
				checkend = ReadCol(2);
				if(checkend==10){break;}
			}
			if(checkend==10){break;}
		}
		if(checkend==10){break;}
	}

	DDRB=0x00;	//sound off
}

void Timer2_init(void){
	DDRB=0x80;
	//TCCR2=0x6A;	//8분주, F PWM mode, OC2 high for top
	//SREG=0x80; //enable all interrupts
	//TIMSK=0x80;
	TCCR2=0x1B; //64분주, CTC mode, OC2 toggle
}

void Dotempsensor(void){
	unsigned int checkend=0;
	LCD_Clear();

	while(1)
	{	
		ADCSRA |= 0x40;	//enable conversion	
		_delay_ms(1);

		char tmp=1;
		while(tmp)
		{
			tmp = ADCSRA;
			tmp &= 0x40;
		}
		displaytemp();
		checkend = ReadCol(2);
		if(checkend==10){break;}
	}
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
	_delay_ms(5000);
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

void DoSevenseg(void){
	char array[] = {0x81, 0xE7, 0x49, 0x43, 0x27, 0x13, 0x11, 0x87, 0x01, 0x03};
	unsigned int checkend=0;

	while(1){
		for(int i=0; i<10; i++){
			PORTD = array[i];
			_delay_ms(2500);
			checkend = ReadCol(2);
			
			if(checkend==10){break;}
		}
		if(checkend==10){break;}
	}
	PORTD=0xFF;
}

void ReadSharp(void){
	PORTC = 0x05;
	_delay_ms(2);

	unsigned char temp;

	do{
		temp=PINC;
		temp&=0x7F;
	}while((!flag) || (temp!=0x67));	//when flag=1 and PINC=#
	
	flag=0;
	temp=0;

	while(!temp){
		temp=PIND;
		temp&=0x01;
	}

	LCD_OutChar('w');
}

void ReadStar(void){
	PORTC=0x11;		//to read star(waiting for input status)
	_delay_ms(2);

	unsigned char temp;

	do{
		temp=PINC;
		temp&=0x7F;
	}while((!flag) || (temp!=0x73));	//when flag=1 and PINC=*
	
	flag=0;
	temp=0;
	
	while(!temp){
		temp=PIND;
		temp&=0x01;
	}
	LCD_OutChar('y');
}

void System_init(void){
	SevenSeg_init();
	_delay_ms(2);
	LCD_init();
	_delay_ms(2);
	Keypad_init();
	_delay_ms(2);
	adc_init();
	_delay_ms(2);
	Timer2_init();
	_delay_ms(2);
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
	PORTC = 0x05; //set outputs to ground ->needs to be 1 in order to block the interrupt
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
	  LCD_Command(0x0E); //display on, cursur on, no blinking
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
  _delay_ms(20);
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
