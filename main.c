#include <8052.h>
#include <stdint.h>
#include <stdbool.h>
#include "pinout.h"

#define F_OSC 22118400L
#define BAUDRATE 115200
#define T1_DIV (uint8_t)(256-F_OSC/6/32/BAUDRATE)

#define RW_BUFSIZE 0x10
#define RXTX_BUFSIZE 0x18

#define SMOD0 0x40
#define FE SM0


//Globals
    volatile __code uint8_t *flashaddr = 0;     //Internal CROM address
    volatile __data uint8_t rx_buf[RXTX_BUFSIZE];   //Buffer for incoming data
    volatile __data uint8_t tx_buf[RXTX_BUFSIZE];   //Buffer for outgoing data
    volatile __data uint8_t rw_buf[RW_BUFSIZE];     //Buffer for DUT reads/writes
    volatile uint8_t rx_buf_cnt = 0;
    volatile uint8_t tx_buf_cnt = 0;
    volatile uint8_t rw_buf_cnt = 0;
    volatile uint8_t fe_cnt = 0;

void delay(uint16_t count){
    for(volatile uint16_t i  = 0; i < count; i++){}
}

void set_addr(uint16_t addr){
    A_L = (uint8_t)(addr & 0xff);
    A8 = (addr & 0x0800) > 0;
    A9 = (addr & 0x0400) > 0;
    A10 = (addr & 0x0200) > 0;
    A11 = (addr & 0x0100) > 0;
}

void set_opts(uint8_t opts){
    OPT_BITS = ((OPT_BITS & ~OPT_MASK) | (opts & OPT_MASK));
}
    
void read_signature(uint8_t *buffer){
    RST = 1;
    VPP = 1;
    set_opts(OPT_READ_SIGNATURE);
    for(uint16_t i = 0x30; i < 0x33; i++){
        set_addr(i);
        delay(200);
        *buffer++ = D;
        //*buffer++ = A_L;
    }
}
        

/* R/W FUNCTIONALITY */

//Read <numbytes> bytes of CROM from <*baseaddr> up to <*baseaddr + numbytes - 1>
void readbytes(__code const uint8_t *baseaddr, uint8_t numbytes, uint8_t *buffer){
    for(uint8_t i = 0; i < numbytes; i++){
        *buffer++ = *(baseaddr + i);
    }
}

/* UART CONTROL */
void set_uart(void) {   //For now it's all hard coded
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

void ser_isr(void) __interrupt (SI0_VECTOR) {
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

//Handle all UART commands
void rx_handler(void) {
    if(rx_buf_cnt){
        EA = 0;
        uint8_t token = rx_buf[0];
        switch (token){
            
            //Ping
            //Send: 'P'
            //Receive: 'P'
            case 'P':
                tx_buf[0] = 'P';    //Pong
                rx_buf_cnt = 0;
                tx_buf_cnt = 1;
                TI = 1;
                break;

            //Get frame error count
            //Send: 'F'
            //Receive: 'F' + unsigned byte of error count
            case 'F':
                tx_buf[1] = 'F';    //Frame Error reply
                tx_buf[0] = fe_cnt; //The count itself
                rx_buf_cnt = 0;
                tx_buf_cnt = 2;
                TI = 1;
                break;

            //Read internal flash of programmer chip flash
            //Send: "<'R'><ADDR_LSB><ADDR_MSB><SIZE>"
            //Receive: SIZE bytes of raw data, single byte 'E' if SIZE>RW_BUFSIZE
            case 'R':
                if(rx_buf_cnt >= 4) {
                    if(rx_buf[3] > RW_BUFSIZE){
                        tx_buf[0] = 'E';    //Error!
                        rx_buf_cnt = 0;
                        tx_buf_cnt = 1;
                        TI = 1;
                    }
                    else {
                        rx_buf_cnt = 0;
                        tx_buf_cnt = rx_buf[3];
                        flashaddr = (__code uint8_t*)(((uint16_t)rx_buf[2] << 8)|(rx_buf[1]));
                        //flashaddr = *(rx_buf + 1);
                        readbytes(flashaddr, tx_buf_cnt, tx_buf);
                        TI = 1;
                    }
                }
                break;

            //Read signature bytes of DUT chip
            //Send: "<'S'>"
            //Receive: 'S' + 3 signature bytes
            case 'S':
                rx_buf_cnt = 0;
                tx_buf_cnt = 4;
                *tx_buf = 'S';
                read_signature(tx_buf + 1);
                TI = 1;
                break;

            //NACK - when command is invalid
            //Send: Anything else
            //Receive: 'N'
            default:
                tx_buf[0] = 'N';    //NACK
                rx_buf_cnt = 0;
                tx_buf_cnt = 1;
                TI = 1;
                break;
        }
        EA = 1;
    }
}


void main(void){
    
    set_uart();
    P2_0 = 0;
    EA = 1;

    while(1){
        PCON |= IDL;
        rx_handler();
    }
}
