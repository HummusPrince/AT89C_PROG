#include <8052.h>
#include <stdint.h>

#define F_OSC 12000000L
#define BAUDRATE 4800
#define T1_DIV 256-F_OSC/6/32/BAUDRATE
#define SMOD0 0x40

void delay(uint16_t count){
    for(volatile uint16_t i  = 0; i < count; i++){}
}

void ser_isr(void) __interrupt SI0_VECTOR {
    EA = 0;
}

void main(){

    uint8_t i = 0;

    SM1 = 1;
    PCON |= SMOD | SMOD0;
    TMOD |= T1_M1;
    TH1 = T1_DIV;
    TR1 = 1;
    ES = 1;

    while(1){
        EA = 1;
        SBUF = i++;
        PCON |= IDL;
        TI = 0;
    }
}
