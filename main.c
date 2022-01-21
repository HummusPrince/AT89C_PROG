#include <8052.h>
#include <stdint.h>

#define F_OSC 12000000L
#define BAUDRATE 2400
#define T1_DIV 256-F_OSC/12/32/BAUDRATE

void delay(uint16_t count){
    for(volatile uint16_t i  = 0; i < count; i++){}
}

void main(){
    
    SM1 = 1;
    PCON |= SMOD;
    TMOD |= T1_M1;
    TH1 = T1_DIV;
    TR1 = 1;

    while(1){
        delay(8192);
        P2_0^=1;
        SBUF = 'U';
    }
}
