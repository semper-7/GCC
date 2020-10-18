/*****************************************************
This program was produced by the
CodeWizardAVR V1.25.9 Professional
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 1wire-slave устройство
Version : 1.01
Date    : 01.02.2013
Author  : oka275 aka alex08cb@mail.ru                          
Company : oka275 home studio              
Comments: Код-заготовка под различные задачи (эмуляция устройств 1wire), связь между МК по 1 проводу.
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

#define PORTx PORTD.2 //управление состоянием порта, на котором висит шина 1ware и прерывание по LOW LEVEL
#define PINx  PIND.2   //чтение состояния порта, на котором висит шина 1ware и прерывание по LOW LEVEL
#define DDRx   DDRD.2   //регистр, управляющий режимом порта (ввода/вывода) порта
//Не забудьте сменить порты в asm-коде процедуры SetPortInput()!!!!!!!!!!!!!! 

unsigned char MY_ROM[]={0x1D,0,0,0,0,0,0x1,0}; //наш ROM! слева код семейства, справа CRC (расчитывается сам при включении питания!)
//unsigned char MY_ROM[]={0x28,0xE1,0x1F,0x36,0x04,0x00,0x00,0xCC}; //Реальный ROM одного из DS18b20 (для тестов)
unsigned char ROM_INPUT[]={0,0,0,0,0,0,0,0};  //буфер для приёма ROM по команде Match ROM [55h] и последующ.его сравн с MY_ROM
unsigned char INPUT_BUFFER[]={0,0,0,0,0,0,0,0}; //Входящий командный буфер
unsigned char OUTPUT_BUFFER[]={1,1,2,3,3,4,5,0};//Исходящий буфер ответов/команд Мастеру.

unsigned char crc,bytecnt;
bit mute_on;  
unsigned char crc_recive_error=1;

void SetPortInput()//Процедура переводит порт в режим чтения
{
DDRx=0;PORTx=1;delay_us(1);
}

void SetPortOutput()//Процедура переводит порт в режим записи
{
DDRx=1;PORTx=1;
}
        
void DelayForUpDown() //Процедура ожидает конец текущего таймслота и начало следующего
        {
SetPortInput(); //Принудительно переводим порт в чтение (на случай если забыли)

 #asm
;процедура ожидания поднятия шины (конец текущего таймслота)
;ВНИМАНИЕ!!! В случае использования программы на другом МК, обязательно смените
;адрес и пин порта. Он задается командой sbis $10,2 для процедуры ожидания понятия шины 
;и sbic $10,2 в процедуре ожидания падения шины. Цифра 10 - адрес порта (смотрите даташит
;к Вашему МК, цифра 2 - номер пина. Это эквивалент Сишной команды PORTD.2 для ATMega8.

ldi r30,2      ;младший байт цикла
ldi r31,70     ;старший байт цикла
cpi r30,0      ;дальше идет сравнение, проверка порта и возможный лимитированный..
BRNE lab1      ;..зацикл (выход либо по окончанию задержки, либо по условию)
cpi r31,0   
BREQ ext1    
lab1: sbiw  R30,1 
;не забудьте сменить адрес порта в строчке ниже!!!
sbis $10,2  ; sbic - сравнить порт $10(pinD) пин 2. Если пин очишен, то пропускаем команду ниже (избегаем зацикла)
brne  lab1   
ext1:           

;процедура ожидания опускания шины шины (начало таймслота)
;создана по образу и подобию процедуры выше, но условие сравнения состояния порта противоположное.
ldi r30,2
ldi r31,70
cpi r30,0
BRNE lab2  
cpi r31,0   
BREQ ext2    
lab2: sbiw  R30,1 
;не забудьте сменить адрес порта в строчке ниже!!! 
sbic $10,2  ; sbic - сравнить порт $10(pinD) пин 2. Если пин очишен, то пропускаем команду ниже (избегаем зацикла)
brne  lab2   
ext2:           
        
;задержка после отработки всех процедур
; ldi r17,1 ;35
; cc: subi  R17,1  ; вычесть из регистра 1
; cpi r17,0
; NOP
; brne cc    ; если все еще есть чего вычитать, то переход на метку (еще + 2 такта  или 0.25мкс)
 #endasm
        }

void SendPresense() //Процедура посылки импульса приветствия
        {
        delay_us(15);
        SetPortOutput();
        PORTx=0;
        delay_us(100);
        PORTx=1;
        SetPortInput();
        delay_us(20);
        } 
        
unsigned int crc8(unsigned int seed, unsigned int inData)//проверка CRC.
//пример: c=00; crc=0; for(c=0; c<7; c++) crc=crc8(crc,новый байт из массива);//
{ 
  //Процедура не моя. Я даже не знаю чья!!! Спасибо автору!!!
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

unsigned char get_byte() //Процедура получения байта с шины
        {
static unsigned char c=0;
static unsigned char GetByte=0;
         for (c=0; c<8; c++) 
           {
           DelayForUpDown(); //ожидание таймслота
           delay_us(25);     //не большое отступление от его начала
           GetByte>>=1;      //Прокручивание байта вправо на 1 бит (напоминает револьвер)
           GetByte|=PINx<<7; //Чтение шины и установка старшего бита в ее состояние.
           }                       
           return GetByte;   //Возвращаем полученный байт
        }        
        
void send_byte(unsigned char sndbyte) //Процедура отправки байта в шину
        {
static unsigned char v=0;


         for (v=0; v<8; v++) 
           {
           DelayForUpDown(); //Ждем таймслот
           SetPortOutput();  //Переводим порт в состояние выхода
           delay_us(6);      //Дожидаем, что бы импульс упал куда надо по даташиту
           PORTx=sndbyte&0b00000001; //Выставляем на шине бит, равный текущему передаваемому биту
           delay_us(33);     //Ждем для создания необходимой продолжительности импульса
           PORTx=1;          //Поднимаем шину, передача бита закончена
           
           sndbyte>>=1;      //Крутим байт вправо на следующую позицию
          }                       

        }        

void load_inbox_Match_ROM() //Получение ROM, который ищет Мастер для последующего сравнения "Наш - не наш"
{
static unsigned char c=0;
for (c=0; c<8; c++)   //Организуем цикл из 8ми. У нас же 8 бит в байте?
   {
   ROM_INPUT[c]=get_byte(); //Циклически выгребаем каждый бит
   }
}                
      
void get_MY_SCRATCHPAD() //Получаение SCRATCHPAD. У нас это входной управляющий буфер из 7(!) байт. 
{                        //Уже в простом цикле можно смотреть INPUT_BUFFER[х] и исполнять команды Мастера
static unsigned char c=0;
for (c=0; c<8; c++) //Организуем цикл из 8ми. У нас же 8 бит в байте?
   {
   INPUT_BUFFER[c]=get_byte(); //Циклически выгребаем каждый бит. Все просто.
   }
c=0;
for(c=0; c<7; c++)crc=crc8(crc,INPUT_BUFFER[c]); //Прогнались по crc, высчитали 7 байт.
crc_recive_error=crc-INPUT_BUFFER[7];           //Если принятое crc совпадает с рассчитанным, то crc_recive_error=0
}      
 
void send_MY_SCRATCHPAD()//Отправка SCRATCHPAD. У 18b20 это ОЗУ. У нас это ВЫходной буфер из 7(!) байт. 
{                        //Сюда можно поместить ответы Мастеру или команды.
 unsigned char c=0;
 crc=0; 
 for(c=0; c<7; c++)crc=crc8(crc,OUTPUT_BUFFER[c]);//посчитали crc для первых 7 байт. Получили crc, который надо отправить.
 OUTPUT_BUFFER[7]=crc; //загружаем crc на законное место в буфер для отправки (восьмой байт)
 for (c=0; c<8; c++)send_byte(OUTPUT_BUFFER[c]);   //Отправить буфер мастеру, пусть разбирается.
}

void send_MY_ROM() //Выставляет на шину ROM_CODE (ROM-код нашего slave-устройства)
{        
 static unsigned char c=0;
 for (c=0; c<8; c++) send_byte(MY_ROM[c]);   //Отправить в шину MY_SCRATCHPAD
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
        
unsigned char compare_Match_ROM()//Сравнивает полученный ROM (запрос от мастера) и свой. Если все совпадает, то на выходе "0"
{        
 static unsigned char c=0;
 static unsigned char compare=0;
 for (c=0; c<8; c++) if (ROM_INPUT[c]!=MY_ROM[c]) compare++; //Побайтное сравнение массивов в ROM-ами
 return compare;
}

void search_rom() //процедура ответа на команду 0xF0 (SEARCH ROM)
{
unsigned char byte_count,bit_count,snd_byte;
bit snd_bit;
#define delay 30 //Длительность импульса "0" на шине, в микросекундах

      for (byte_count=0; byte_count<8; byte_count++) //Организуем цикл 8 раз, ведь у нас же 8 байт в ROM-е, да?
             {
             snd_byte=MY_ROM[byte_count]; //Что бы не покосячить ROM, выдергиваем байт по одному в переменную snd_byte
             for (bit_count=0; bit_count<8; bit_count++) //Организуем вложенный цикл 8 раз, ведь у нас же 8 бит, да?
                        {
                           //Выделение бита и подготовка его к отправке
                           snd_bit=snd_byte&0b00000001; //Сбрасываем в байте все биты кроме самого младшего. Младший бит готов к отправке!
                           //отправка неинверсного (прямого) бита:
                           DelayForUpDown(); //Дожидаемся таймслота
                           SetPortOutput();  //Переводим порт в режим вывода. В режим ввода он перейдет сам после отработки процедуры.
                           if(!mute_on) PORTx=snd_bit; //Если бит отправки 0, то прижимаем линию (говорим "0" мастеру).Если бит 1, то молчим (мастер сам понимает, что это "1")
                           delay_us(delay); //Создаем длительность. Если snd_bit=0, то это будет длительность прижатия. Если 1, то просто деваем куда то время.
                           PORTx=1;       //Поднимаем порт. Если он и без того был в 1, то хуже ему не будет.
                           //Бит отправлен. Если mute_on было 1, то мы просто молчали и делали пустые телодвижения. Но все необходимо для
                           //того, что бы не потерять синхронизацию с шиной. Пробовал делать break, но так и не понял почему криво работает.
                           
                           //Подготовка к отправке инверсного бита, согласно даташиту
                           DelayForUpDown(); //Дожидаемся таймслота
                           SetPortOutput();   //Переводим порт в режим вывода. В режим ввода он перейдет сам после отработки процедуры.
                           if(!mute_on) PORTx=~snd_bit; //Если молчание не запрещено, то отправляем инверсный бит.
                           delay_us(delay);  //Создаем длительность. Если snd_bit=0, то это будет длительность прижатия. Если 1, то просто деваем куда то время.
                           PORTx=1;        //Поднимаем порт. Если он и без того был в 1, то хуже ему не будет.
                           //Инверсивный бит отправлен, переходим к слушанию ответа Мастера. 

                           //Читаем что нам ответит Мастер. По даташиту, если ответ Мастера совпадает с отправленным "прямым" битом, то
                           //это означает, что Мастер хочет нас слушать дальше, и потому надо продолжать в том же духе.
                           //Если ответ Мастера совпадает с инверсным битом, то значит он переключил внимание на другое устройство
                           //и нам следует замолчать до следующего импульса сброса. При следующем цикле(ах) запроса(ов) он спросит уже нас по любому.
                           //Итак, поехали:
                           DelayForUpDown(); //Дожидаемся таймслота                          
                           delay_us(5);      //создаем паузу для стабилизации шины
                           if((!PINx==snd_bit)&&(!mute_on)) mute_on=1; //Читаем шину. Если с нами больше не хотят работать, то выставляем флаг mute_on=1 и до след.сброса молчим как рыба об лед
                           snd_byte>>=1;     //Сдвигаем все биты байта отправки вправо на 1 бит. Тем самым бит7 становится на место бит6, 
                                             //бит 6 встает на место бит5 и так до бит0.
                        }
             }
        // DelayForUpDown(); //При выходе ловим еще один фронт. Зачем он нужен, я так и не понял. Просто пропускаем его и все.
}

  
interrupt [EXT_INT0] void ext_int0_isr(void)
{// Place your code here
unsigned char command=0;           //Содержит код получаемой команды
static unsigned char time_reset_count=0; //Счетчик для примерного подсчета времени длительности импульса сброса
start:
//Обработка прерывания (начало программы) находится здесь.
time_reset_count=0;  //Обнуляем счетчик, нам нужен там гарантированный ноль.
mute_on=0;           //Обнуляем флаг молчания (нужно для процедуры SEARCH_ROM)

//Ждем, пока мастер не отпустит линию, то есть пока линия не поднимется в 1.
while(!PINx){time_reset_count++;} //Пока линия лежит, цикл будет вечным. Кроме того подсчитывается его длительность (приблизительно)
if(time_reset_count<8) goto exit;   //Если вызвавший прерывание импульс примерно менее 480мксек (по даташиту сигнал сброса МИНИМУМ 480мксек!)
                                    //то игнорируем его и выходим из прерывания

//если импульс дольше 480мксек, то это значит был импульс сброса и на него надо ответить присутствием
SendPresense();     //Шлём импульс присутствия, после чего ожидаем получение команды. 
command=get_byte(); //Получаем первый байт/команду и решаем что делать

//Если это команда Match ROM [55h], то значит мастер обращается к устройству посредством ROM-адресации.
if(command==0x55)//Значит надо приготовиться принять 64бита и оценить - нам этот запрос или нет
                {//если ДА!!!!
                load_inbox_Match_ROM(); //загружаем Match ROM, который ищет мастер, в буфер для последующего сравнения (к нам ли обращаются??)
                if(!compare_Match_ROM()) //Сравниваем наш ROM с поисковым. Если 0, то ХОТЯТ РАБОТАТЬ С НАМИ! Значит будут слать команды еще!
                {//НИЖЕ ВЫПОЛНЯЕТСЯ ЕСЛИ Match_ROM СОВПАЛ С НАШИМ, значит обращаются именно к нам!!
                command=get_byte(); //получаем возможную команду (обычно это 0xBE - Read Scratchpad [BEh])
            if(command==0xBE) send_MY_SCRATCHPAD(); //Если так и есть, то шлем мастеру наш Scratchpad, пусть радуется, если это была не команда 0xBE, то грустим 
            if(command==0x4E) get_MY_SCRATCHPAD();  //Команда загрузки Scratchpad к нам (8 байт целиком)+расчет ccrc_recive_error
            if(command==0x4F) send_byte(crc_recive_error); //Команда запроса "Удачно ли приняли мою посылку?" (get_MY_SCRATCHPAD();)
                //зарезервировано. тут место под возможные обработчики команд
                //зарезервировано. тут место под возможные обработчики команд
                }
                }//конец If (command==0x55)

if(command==0xf0) //получена команда "SEARCH ROM"
                {
                search_rom(); //Запускаем процедуру
                delay_us(200); //Делаем задержку, что бы МК додрыгал какие то там импульсы (так и не понял что это за импульсы)
                goto start; //Отправляем в начало данной подрпограммы обработки прерывания
                }

if(command==0x33)//Поступила команда READ ROM [33h] (отсылаем наш ROM запросившему)
                {
                send_MY_ROM(); // Процедура отсылки ROM
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
             
//Расчет crc нашего ROM, что бы мы не запаривались над этим при его смене на другой
crc=0;
for(bytecnt=0; bytecnt<7; bytecnt++) crc=crc8(crc,MY_ROM[bytecnt]);//посчитали crc для первых 7 байт. Получили crc.
MY_ROM[7]=crc; //загружаем crc на законное место (восьмой байт)
//crc нашего ROM посчитан!!!


while (1)
      {

//delay_ms(1000);
  
      };



}
