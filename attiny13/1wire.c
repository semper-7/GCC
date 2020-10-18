/*****************************************************
This program was produced by the
CodeWizardAVR V1.25.9 Professional
Automatic Program Generator
� Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 1wire-slave ����������
Version : 1.01
Date    : 01.02.2013
Author  : oka275 aka alex08cb@mail.ru                          
Company : oka275 home studio              
Comments: ���-��������� ��� ��������� ������ (�������� ��������� 1wire), ����� ����� �� �� 1 �������.
Chip type           : ATmega8
Program type        : Application
Clock frequency     : 8,000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*****************************************************/

#include <mega8.h>  
#include <delay.h>
//#define LED PORTD.0

#define PORTx PORTD.2 //���������� ���������� �����, �� ������� ����� ���� 1ware � ���������� �� LOW LEVEL
#define PINx  PIND.2   //������ ��������� �����, �� ������� ����� ���� 1ware � ���������� �� LOW LEVEL
#define DDRx   DDRD.2   //�������, ����������� ������� ����� (�����/������) �����
//�� �������� ������� ����� � asm-���� ��������� SetPortInput()!!!!!!!!!!!!!! 

unsigned char MY_ROM[]={0x1D,0,0,0,0,0,0x1,0}; //��� ROM! ����� ��� ���������, ������ CRC (������������� ��� ��� ��������� �������!)
//unsigned char MY_ROM[]={0x28,0xE1,0x1F,0x36,0x04,0x00,0x00,0xCC}; //�������� ROM ������ �� DS18b20 (��� ������)
unsigned char ROM_INPUT[]={0,0,0,0,0,0,0,0};  //����� ��� ����� ROM �� ������� Match ROM [55h] � ���������.��� ����� � MY_ROM
unsigned char INPUT_BUFFER[]={0,0,0,0,0,0,0,0}; //�������� ��������� �����
unsigned char OUTPUT_BUFFER[]={1,1,2,3,3,4,5,0};//��������� ����� �������/������ �������.

unsigned char crc,bytecnt;
bit mute_on;  
unsigned char crc_recive_error=1;

void SetPortInput()//��������� ��������� ���� � ����� ������
{
DDRx=0;PORTx=1;delay_us(1);
}

void SetPortOutput()//��������� ��������� ���� � ����� ������
{
DDRx=1;PORTx=1;
}
        
void DelayForUpDown() //��������� ������� ����� �������� ��������� � ������ ����������
        {
SetPortInput(); //������������� ��������� ���� � ������ (�� ������ ���� ������)

 #asm
;��������� �������� �������� ���� (����� �������� ���������)
;��������!!! � ������ ������������� ��������� �� ������ ��, ����������� �������
;����� � ��� �����. �� �������� �������� sbis $10,2 ��� ��������� �������� ������� ���� 
;� sbic $10,2 � ��������� �������� ������� ����. ����� 10 - ����� ����� (�������� �������
;� ������ ��, ����� 2 - ����� ����. ��� ���������� ������ ������� PORTD.2 ��� ATMega8.

ldi r30,2      ;������� ���� �����
ldi r31,70     ;������� ���� �����
cpi r30,0      ;������ ���� ���������, �������� ����� � ��������� ��������������..
BRNE lab1      ;..������ (����� ���� �� ��������� ��������, ���� �� �������)
cpi r31,0   
BREQ ext1    
lab1: sbiw  R30,1 
;�� �������� ������� ����� ����� � ������� ����!!!
sbis $10,2  ; sbic - �������� ���� $10(pinD) ��� 2. ���� ��� ������, �� ���������� ������� ���� (�������� �������)
brne  lab1   
ext1:           

;��������� �������� ��������� ���� ���� (������ ���������)
;������� �� ������ � ������� ��������� ����, �� ������� ��������� ��������� ����� ���������������.
ldi r30,2
ldi r31,70
cpi r30,0
BRNE lab2  
cpi r31,0   
BREQ ext2    
lab2: sbiw  R30,1 
;�� �������� ������� ����� ����� � ������� ����!!! 
sbic $10,2  ; sbic - �������� ���� $10(pinD) ��� 2. ���� ��� ������, �� ���������� ������� ���� (�������� �������)
brne  lab2   
ext2:           
        
;�������� ����� ��������� ���� ��������
; ldi r17,1 ;35
; cc: subi  R17,1  ; ������� �� �������� 1
; cpi r17,0
; NOP
; brne cc    ; ���� ��� ��� ���� ���� ��������, �� ������� �� ����� (��� + 2 �����  ��� 0.25���)
 #endasm
        }

void SendPresense() //��������� ������� �������� �����������
        {
        delay_us(15);
        SetPortOutput();
        PORTx=0;
        delay_us(100);
        PORTx=1;
        SetPortInput();
        delay_us(20);
        } 
        
unsigned int crc8(unsigned int seed, unsigned int inData)//�������� CRC.
//������: c=00; crc=0; for(c=0; c<7; c++) crc=crc8(crc,����� ���� �� �������);//
{ 
  //��������� �� ���. � ���� �� ���� ���!!! ������� ������!!!
unsigned int bitsLeft=0;
unsigned int tempp=0;

	for (bitsLeft=8;bitsLeft>0;bitsLeft--)
	{
	tempp=((seed^inData)&0x01);
	if(tempp==0) seed >>= 1;
        	else
		{
		 seed ^=0x18;
		 seed >>=1;
		 seed |=0x80;
		}
		inData >>= 1;
	}
	return seed;
}

unsigned char get_byte() //��������� ��������� ����� � ����
        {
static unsigned char c=0;
static unsigned char GetByte=0;
         for (c=0; c<8; c++) 
           {
           DelayForUpDown(); //�������� ���������
           delay_us(25);     //�� ������� ����������� �� ��� ������
           GetByte>>=1;      //������������� ����� ������ �� 1 ��� (���������� ���������)
           GetByte|=PINx<<7; //������ ���� � ��������� �������� ���� � �� ���������.
           }                       
           return GetByte;   //���������� ���������� ����
        }        
        
void send_byte(unsigned char sndbyte) //��������� �������� ����� � ����
        {
static unsigned char v=0;


         for (v=0; v<8; v++) 
           {
           DelayForUpDown(); //���� ��������
           SetPortOutput();  //��������� ���� � ��������� ������
           delay_us(6);      //��������, ��� �� ������� ���� ���� ���� �� ��������
           PORTx=sndbyte&0b00000001; //���������� �� ���� ���, ������ �������� ������������� ����
           delay_us(33);     //���� ��� �������� ����������� ����������������� ��������
           PORTx=1;          //��������� ����, �������� ���� ���������
           
           sndbyte>>=1;      //������ ���� ������ �� ��������� �������
          }                       

        }        

void load_inbox_Match_ROM() //��������� ROM, ������� ���� ������ ��� ������������ ��������� "��� - �� ���"
{
static unsigned char c=0;
for (c=0; c<8; c++)   //���������� ���� �� 8��. � ��� �� 8 ��� � �����?
   {
   ROM_INPUT[c]=get_byte(); //���������� ��������� ������ ���
   }
}                
      
void get_MY_SCRATCHPAD() //���������� SCRATCHPAD. � ��� ��� ������� ����������� ����� �� 7(!) ����. 
{                        //��� � ������� ����� ����� �������� INPUT_BUFFER[�] � ��������� ������� �������
static unsigned char c=0;
for (c=0; c<8; c++) //���������� ���� �� 8��. � ��� �� 8 ��� � �����?
   {
   INPUT_BUFFER[c]=get_byte(); //���������� ��������� ������ ���. ��� ������.
   }
c=0;
for(c=0; c<7; c++)crc=crc8(crc,INPUT_BUFFER[c]); //���������� �� crc, ��������� 7 ����.
crc_recive_error=crc-INPUT_BUFFER[7];           //���� �������� crc ��������� � ������������, �� crc_recive_error=0
}      
 
void send_MY_SCRATCHPAD()//�������� SCRATCHPAD. � 18b20 ��� ���. � ��� ��� �������� ����� �� 7(!) ����. 
{                        //���� ����� ��������� ������ ������� ��� �������.
 unsigned char c=0;
 crc=0; 
 for(c=0; c<7; c++)crc=crc8(crc,OUTPUT_BUFFER[c]);//��������� crc ��� ������ 7 ����. �������� crc, ������� ���� ���������.
 OUTPUT_BUFFER[7]=crc; //��������� crc �� �������� ����� � ����� ��� �������� (������� ����)
 for (c=0; c<8; c++)send_byte(OUTPUT_BUFFER[c]);   //��������� ����� �������, ����� �����������.
}

void send_MY_ROM() //���������� �� ���� ROM_CODE (ROM-��� ������ slave-����������)
{        
 static unsigned char c=0;
 for (c=0; c<8; c++) send_byte(MY_ROM[c]);   //��������� � ���� MY_SCRATCHPAD
}
 
/*
void LED_FLASH()
{        
LED=0;
delay_ms(10);
LED=1;
}
*/
 
/*
void LED_PIN()
{        
LED=0;
delay_us(1);
LED=1;
} 
*/
        
unsigned char compare_Match_ROM()//���������� ���������� ROM (������ �� �������) � ����. ���� ��� ���������, �� �� ������ "0"
{        
 static unsigned char c=0;
 static unsigned char compare=0;
 for (c=0; c<8; c++) if (ROM_INPUT[c]!=MY_ROM[c]) compare++; //��������� ��������� �������� � ROM-���
 return compare;
}

void search_rom() //��������� ������ �� ������� 0xF0 (SEARCH ROM)
{
unsigned char byte_count,bit_count,snd_byte;
bit snd_bit;
#define delay 30 //������������ �������� "0" �� ����, � �������������

      for (byte_count=0; byte_count<8; byte_count++) //���������� ���� 8 ���, ���� � ��� �� 8 ���� � ROM-�, ��?
             {
             snd_byte=MY_ROM[byte_count]; //��� �� �� ���������� ROM, ����������� ���� �� ������ � ���������� snd_byte
             for (bit_count=0; bit_count<8; bit_count++) //���������� ��������� ���� 8 ���, ���� � ��� �� 8 ���, ��?
                        {
                           //��������� ���� � ���������� ��� � ��������
                           snd_bit=snd_byte&0b00000001; //���������� � ����� ��� ���� ����� ������ ��������. ������� ��� ����� � ��������!
                           //�������� ������������ (�������) ����:
                           DelayForUpDown(); //���������� ���������
                           SetPortOutput();  //��������� ���� � ����� ������. � ����� ����� �� �������� ��� ����� ��������� ���������.
                           if(!mute_on) PORTx=snd_bit; //���� ��� �������� 0, �� ��������� ����� (������� "0" �������).���� ��� 1, �� ������ (������ ��� ��������, ��� ��� "1")
                           delay_us(delay); //������� ������������. ���� snd_bit=0, �� ��� ����� ������������ ��������. ���� 1, �� ������ ������ ���� �� �����.
                           PORTx=1;       //��������� ����. ���� �� � ��� ���� ��� � 1, �� ���� ��� �� �����.
                           //��� ���������. ���� mute_on ���� 1, �� �� ������ ������� � ������ ������ ������������. �� ��� ���������� ���
                           //����, ��� �� �� �������� ������������� � �����. �������� ������ break, �� ��� � �� ����� ������ ����� ��������.
                           
                           //���������� � �������� ���������� ����, �������� ��������
                           DelayForUpDown(); //���������� ���������
                           SetPortOutput();   //��������� ���� � ����� ������. � ����� ����� �� �������� ��� ����� ��������� ���������.
                           if(!mute_on) PORTx=~snd_bit; //���� �������� �� ���������, �� ���������� ��������� ���.
                           delay_us(delay);  //������� ������������. ���� snd_bit=0, �� ��� ����� ������������ ��������. ���� 1, �� ������ ������ ���� �� �����.
                           PORTx=1;        //��������� ����. ���� �� � ��� ���� ��� � 1, �� ���� ��� �� �����.
                           //����������� ��� ���������, ��������� � �������� ������ �������. 

                           //������ ��� ��� ������� ������. �� ��������, ���� ����� ������� ��������� � ������������ "������" �����, ��
                           //��� ��������, ��� ������ ����� ��� ������� ������, � ������ ���� ���������� � ��� �� ����.
                           //���� ����� ������� ��������� � ��������� �����, �� ������ �� ���������� �������� �� ������ ����������
                           //� ��� ������� ��������� �� ���������� �������� ������. ��� ��������� �����(��) �������(��) �� ������� ��� ��� �� ������.
                           //����, �������:
                           DelayForUpDown(); //���������� ���������                          
                           delay_us(5);      //������� ����� ��� ������������ ����
                           if((!PINx==snd_bit)&&(!mute_on)) mute_on=1; //������ ����. ���� � ���� ������ �� ����� ��������, �� ���������� ���� mute_on=1 � �� ����.������ ������ ��� ���� �� ���
                           snd_byte>>=1;     //�������� ��� ���� ����� �������� ������ �� 1 ���. ��� ����� ���7 ���������� �� ����� ���6, 
                                             //��� 6 ������ �� ����� ���5 � ��� �� ���0.
                        }
             }
        // DelayForUpDown(); //��� ������ ����� ��� ���� �����. ����� �� �����, � ��� � �� �����. ������ ���������� ��� � ���.
}

  
interrupt [EXT_INT0] void ext_int0_isr(void)
{// Place your code here
unsigned char command=0;           //�������� ��� ���������� �������
static unsigned char time_reset_count=0; //������� ��� ���������� �������� ������� ������������ �������� ������
start:
//��������� ���������� (������ ���������) ��������� �����.
time_reset_count=0;  //�������� �������, ��� ����� ��� ��������������� ����.
mute_on=0;           //�������� ���� �������� (����� ��� ��������� SEARCH_ROM)

//����, ���� ������ �� �������� �����, �� ���� ���� ����� �� ���������� � 1.
while(!PINx){time_reset_count++;} //���� ����� �����, ���� ����� ������. ����� ���� �������������� ��� ������������ (��������������)
if(time_reset_count<8) goto exit;   //���� ��������� ���������� ������� �������� ����� 480����� (�� �������� ������ ������ ������� 480�����!)
                                    //�� ���������� ��� � ������� �� ����������

//���� ������� ������ 480�����, �� ��� ������ ��� ������� ������ � �� ���� ���� �������� ������������
SendPresense();     //��� ������� �����������, ����� ���� ������� ��������� �������. 
command=get_byte(); //�������� ������ ����/������� � ������ ��� ������

//���� ��� ������� Match ROM [55h], �� ������ ������ ���������� � ���������� ����������� ROM-���������.
if(command==0x55)//������ ���� ������������� ������� 64���� � ������� - ��� ���� ������ ��� ���
                {//���� ��!!!!
                load_inbox_Match_ROM(); //��������� Match ROM, ������� ���� ������, � ����� ��� ������������ ��������� (� ��� �� ����������??)
                if(!compare_Match_ROM()) //���������� ��� ROM � ���������. ���� 0, �� ����� �������� � ����! ������ ����� ����� ������� ���!
                {//���� ����������� ���� Match_ROM ������ � �����, ������ ���������� ������ � ���!!
                command=get_byte(); //�������� ��������� ������� (������ ��� 0xBE - Read Scratchpad [BEh])
            if(command==0xBE) send_MY_SCRATCHPAD(); //���� ��� � ����, �� ���� ������� ��� Scratchpad, ����� ��������, ���� ��� ���� �� ������� 0xBE, �� ������� 
            if(command==0x4E) get_MY_SCRATCHPAD();  //������� �������� Scratchpad � ��� (8 ���� �������)+������ ccrc_recive_error
            if(command==0x4F) send_byte(crc_recive_error); //������� ������� "������ �� ������� ��� �������?" (get_MY_SCRATCHPAD();)
                //���������������. ��� ����� ��� ��������� ����������� ������
                //���������������. ��� ����� ��� ��������� ����������� ������
                }
                }//����� If (command==0x55)

if(command==0xf0) //�������� ������� "SEARCH ROM"
                {
                search_rom(); //��������� ���������
                delay_us(200); //������ ��������, ��� �� �� �������� ����� �� ��� �������� (��� � �� ����� ��� ��� �� ��������)
                goto start; //���������� � ������ ������ ������������ ��������� ����������
                }

if(command==0x33)//��������� ������� READ ROM [33h] (�������� ��� ROM ������������)
                {
                send_MY_ROM(); // ��������� ������� ROM
                }

exit:
}

// Declare your global variables here


void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTB=0x00;
DDRB=0x00;

// Port C initialization
// Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=T State4=T State3=T State2=T State1=T State0=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=In Func1=Out Func0=Out 
// State7=1 State6=1 State5=1 State4=1 State3=1 State2=P State1=1 State0=1 
PORTD=0xFF;
DDRD=0xFB;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=Out Func2=In Func1=In Func0=Out 
// State7=T State6=T State5=T State4=T State3=1 State2=P State1=T State0=1 
//PORTD=0x0D;
//DDRD=0x09;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
TCCR0=0x00;
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer 1 Stopped
// Mode: Normal top=FFFFh
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Low level
// INT1: Off
GICR|=0x40;
MCUCR=0x00;
GIFR=0x40;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// Global enable interrupts
#asm("sei")
             
//������ crc ������ ROM, ��� �� �� �� ������������ ��� ���� ��� ��� ����� �� ������
crc=0;
for(bytecnt=0; bytecnt<7; bytecnt++) crc=crc8(crc,MY_ROM[bytecnt]);//��������� crc ��� ������ 7 ����. �������� crc.
MY_ROM[7]=crc; //��������� crc �� �������� ����� (������� ����)
//crc ������ ROM ��������!!!


while (1)
      {

//delay_ms(1000);
  
      };



}
