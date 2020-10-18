#ifndef delay_h
#define delay_h

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

void DelayInit(void (*)() = 0);
void delay(dword);

#endif
