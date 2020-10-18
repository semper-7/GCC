#ifndef OneWire_h
#define OneWire_h

#define PORT_LETTER C

class OneWire {
private:
volatile uint8_t mask;
void write_bit(uint8_t v);
uint8_t read_bit(void);

public:
OneWire(uint8_t m);
uint8_t reset(void);
void write(uint8_t v);
void write_bytes(const uint8_t *buf, uint16_t count);
uint8_t read(void);
void read_bytes(uint8_t *buf, uint16_t count);
void select(const uint8_t rom[8]);
static uint8_t crc8(const uint8_t *addr, uint8_t len);
};

#endif
