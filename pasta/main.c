#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE 65536 // 2^16

uint16_t R[16];
uint16_t MEM[MEM_SIZE];
uint16_t PC;
uint16_t SP;
bool Z, N;

void reset();
uint16_t fetch();
bool execute(uint16_t instr);



int main() {
    reset();

    MEM[0] = 0xFFFF;

    while (1) {
        uint16_t instr = fetch();
        if (!execute(instr))
            break;
    }

    printf("llll\n");
    return 0;
}






void reset() {
    for (int i = 0; i < 16; i++)
        R[i] = 0;

    for (int i = 0; i < MEM_SIZE; i++)
        MEM[i] = 0;

    PC = 0;
    SP = MEM_SIZE - 1;

    Z = false;
    N = false;
}

uint16_t fetch() {
    uint16_t instr = MEM[PC];
    PC++;
    return instr;
}

bool execute(uint16_t instr) {
    if (instr == 0xFFFF) {
        return false;
    }
    return true;
}
