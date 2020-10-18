/*Arduino 12V PIC programmer
Semper@2017-2018

	12V VPP Sheet
PB4___        VPP(12V)
D12   )|       |
      )|       |
100uH )|       | Zener 10V
      )| 1n4148|  _    LED yellow
     _|---|>|--+--|<|--|>|--
PB3_|_ 2n7000  |            |
D11   |       === 1,0       |
GND___|________|____________|

PIC12F635:
PIN	PINNAME	Arduino 
1	VDD	PB5(D13)
2	NC
3	NC
4	VPP	VPP(12V)
5	NC
6	CLOCK	PC1(A1)
7	DATA	PC0(A0)
8       GND	GND

*/

#define  CLOCK	15
#define  VPP	12
#define  VDD	13
#define  DATA	14

unsigned int addr = 0;
char command = '\0';
unsigned int wal;
char strF;
char strVal[4];

void setup() {
Serial.begin(9600);
uint8_t oldSREG = SREG;
cli();
TIMSK2 = TCCR2B = TCCR2A = 0;
OCR2A = 3; 
TCCR2A = 0x42;
TCCR2B = 1;
SREG = oldSREG;
Power();
}

void loop() {

 if (Serial.available() > 0) {
 command = Serial.read();
  switch(command) {

  case 'e' :
  BulkEraseProgramMemory();
  Serial.println("Program Memory is erased!!!");
  break;
 
  case 'r' :
  HexPrintLine();
  break;
 
  case 'd' :
  for(byte k=0;k<64;k++) HexPrintLine();
  break;
 
  case 'p' :
  Reset();
  Serial.println("Start programming");
   while (true) {
   Serial.println("");
   HexPrint(addr);
   Serial.print(": ");
    for(byte j=0;j<16;j++) {
    wal=0;
     for(byte i=0;i<4;i++) {
     while(Serial.available() == 0) {}
     strVal[i] = Serial.read();
      if (strVal[i] == '$') {
      Serial.println("\r\nEnd programming");
      goto lll;
      }
     Serial.print(strVal[i]);
     if (strVal[i]>0x40) strVal[i]-=7;
     wal=(wal<<4)|int(strVal[i]&=0x0f);
     }
     LoadDataCommandForProgramMemory(wal);
     BeginProgrammingCycle();
      if (wal != ReadDataFromProgramMemory()) {
      Serial.println("\r\nERROR PROGRAMMING!!!");
      goto lll;
      }
     IncrementAddress();
     addr++;
   }}
  lll:
  break;
 
  case 'c' :
  Serial.println("Configuration memory:");
  LoadConfiguration(0);
  addr = 0x2000;
  HexPrintLine();
  break;
 
  case 'f' :
  Serial.print("Fuse 200");
  while(Serial.available() == 0) {}
  strF = Serial.read();
  Serial.print(strF);
  Serial.print("=");
  wal=0;
   for(byte i=0;i<4;i++) {
   while(Serial.available() == 0) {}
   strVal[i] = Serial.read();
   Serial.print(strVal[i]);
   if (strVal[i]>0x40) strVal[i]-=7;
   wal=(wal<<4)|int(strVal[i]&=0x0f);
   }
  LoadConfiguration(0);
  for(byte i=strF;i>48;i--) IncrementAddress();
  LoadDataCommandForProgramMemory(wal);
  BeginProgrammingCycle();
  if (wal != ReadDataFromProgramMemory()) Serial.println("\r\nERROR PROGRAMMING!!!");
  Serial.println("\r\nEnd fuse programming");
  break;
 
  case 'R' :
  Serial.println("Reset");
  Reset();
  break;
 
  case 'q' :
  Serial.println("Power OFF");
  Power();
  break;
}}}
   
void Power() {
DDRB = DDRC = PORTB = PORTC = 0;
goto sss;
 while(true) {
 if (Serial.available() > 0) {
  if(Serial.read()=='s') {
  DDRB = B00111000;
  Reset();
  DDRC = 3;
  Serial.println("Pic detected. Supported commands: e,r,d,p,c,f,R,q");
  break;
  } else {
sss:
   Serial.println("Send s to start");
}}}}
  
void Reset() {
PORTB = B00010000;
delay(500);
PORTB = B00110000;
addr = 0;
} 

void WriteBit(uint8_t v) {
PORTC =v|2;
delayMicroseconds(3);
PORTC &=1 ;
delayMicroseconds(3);
PORTC =0;
}

byte ReadBit() {
uint8_t v = 0;
PORTC =2;
delayMicroseconds(3);
v = PINC&1;
PORTC =0;
delayMicroseconds(3);
return v;
}

unsigned int ReadDataFromProgramMemory() {
uint16_t v = 0;
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
DDRC =2;
delayMicroseconds(3);
ReadBit();
for(byte i=0;i<14;i++) bitWrite(v,i,ReadBit());
ReadBit();
DDRC =3;
return v;
}

void LoadDataCommandForProgramMemory(unsigned int v) {
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
delayMicroseconds(3);
WriteBit(0);
for(byte i=0;i<14;i++) WriteBit(bitRead(v,i));
WriteBit(0);
delayMicroseconds(3);
}

void BeginProgrammingCycle() {
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
delay(10);
}

void BulkEraseProgramMemory() {
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
delay(100);
}

void IncrementAddress() {
WriteBit(0);
WriteBit(1);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
delayMicroseconds(3);
}

void LoadConfiguration(unsigned int v) {
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
delayMicroseconds(3);
WriteBit(0);
for(byte i=0;i<14;i++) WriteBit(bitRead(v,i));
WriteBit(0);
delayMicroseconds(3);
}

void HexPrint(uint16_t v) {
 if (v<4096) {
 Serial.print("0");
  if (v<256) {
  Serial.print("0");
  if (v<16) Serial.print("0");
 }}
Serial.print(v,HEX);
} 
  
void HexPrintLine() {
HexPrint(addr);
Serial.print(": ");
 for(byte j=0;j<16;j++) {
 HexPrint(ReadDataFromProgramMemory());
 IncrementAddress();
 }
Serial.println("");
addr=addr+16;
}


