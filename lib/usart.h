#ifndef usart_h
#define usart_h

void UsartInit(unsigned long);
void UsartWrite(char);
void UsartPrint(const char *);
void UsartPrint(const char *, uint16_t);
void UsartPrintPGM(const char *);
char UsartRead(bool);

#endif
