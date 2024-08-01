#include <8051.h>

#define A_L P0
#define A8 P3_7
#define A9 P3_6
#define A10 P3_5
#define A11 P3_4

#define D P1

#define OPT_BITS P2
#define OPT_MASK 0x58

#define nENA P2_7
#define RDY P2_5

#define VPP P2_2
#define ALE P2_1

#define OPT_READ_SIGNATURE 0x00
#define OPT_READ_CODE 0x18
