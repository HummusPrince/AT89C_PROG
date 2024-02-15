#include <8052.h>
#include <stdint.h>
#include <stdbool.h>

#define F_OSC 22118400L
#define BAUDRATE 115200
#define T1_DIV (uint8_t)(256-F_OSC/6/32/BAUDRATE)

#define RW_BUFSIZE 0x10
#define RXTX_BUFSIZE 0x18

#define SMOD0 0x40
#define FE SM0

//Globals
    volatile __code uint8_t *flashaddr = 0;
    volatile __data uint8_t rx_buf[RXTX_BUFSIZE];
    volatile __data uint8_t tx_buf[RXTX_BUFSIZE];
    volatile __data uint8_t rw_buf[RW_BUFSIZE];
    volatile uint8_t rx_buf_cnt = 0;
    volatile uint8_t tx_buf_cnt = 0;
    volatile uint8_t rw_buf_cnt = 0;
    volatile uint8_t fe_cnt = 0;
    
void delay(uint16_t count){
    for(volatile uint16_t i  = 0; i < count; i++){}
}

/* R/W FUNCTIONALITY */

//Read <numbytes> bytes of CROM from <*baseaddr> up to <*baseaddr + numbytes - 1>
void readbytes(__code const uint8_t *baseaddr, uint8_t numbytes, uint8_t *buffer){
    for(uint8_t i = 0; i < numbytes; i++){
        *buffer++ = *(baseaddr + i);
    }
}

/* UART CONTROL */
void set_uart() {   //For now it's all hard coded
    //Set UART mode 1
    SM1 = 1;
    SM0 = 0;
    // Enable double baud rate mode + Frame Error detection enable
    PCON |= SMOD | SMOD0;
    //Set timer1 to auto-reload 8-bit mode and set baud rate
    TMOD |= T1_M1;
    TH1 = T1_DIV;
    //Run timer 1
    TR1 = 1;
    //Enable UART interrupts
    ES = 1;
    //Enable UART RX
    REN = 1;
}

/* ISRs */

void ser_isr(void) __interrupt SI0_VECTOR {
    EA = 0;
    if(FE){
        fe_cnt++;
        FE = 0;
        //P2_0 = 1;
        return;
    }
    if(RI){
        RI = 0;
        if(rx_buf_cnt < RXTX_BUFSIZE) rx_buf[rx_buf_cnt++] = SBUF;
    }
    if(TI || tx_buf_cnt){
        TI = 0;
        if(tx_buf_cnt) SBUF = tx_buf[--tx_buf_cnt];
    }
    EA = 1;
}

    // Packet: "<'R'><ADDR_LSB><ADDR_MSB><SIZE>"

void main(){
    
    set_uart();
    P2_0 = 0;
    EA = 1;

    while(1){
        PCON |= IDL;
        if(rx_buf_cnt >= 4 && tx_buf_cnt == 0){
            //while(rx_buf_cnt) tx_buf[tx_buf_cnt++] = rx_buf[rx_buf_cnt-- - 1];
            //tx_buf_cnt++;
            //SBUF = ':';
            EA = 0;
            rx_buf_cnt = 0;
            if(rx_buf[0] == 'R'){
                if(rx_buf[3] > RW_BUFSIZE){
                }
                else {
                    //P2_0 ^= 1;
                    tx_buf_cnt = rx_buf[3];
                    flashaddr = (__code uint8_t*)(((uint16_t)rx_buf[2] << 8)|(rx_buf[1]));
                    //flashaddr = *(rx_buf + 1);
                    readbytes(flashaddr, tx_buf_cnt, tx_buf);
                    TI = 1;
                }
            }
            EA = 1;
        }

    }
}
