#include <8052.h>
#include <stdint.h>

void main(){
volatile uint32_t i = 0;
    while(1){
        for(i = 0; i < 10000; i++){}
        P2_0^=1;
    }
}
