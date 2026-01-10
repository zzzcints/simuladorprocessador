#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 65536 // 2^16
#define MEM_LIMIT 0X2000 // Limite de memória de dados - 8kb palavras
#define IO_BASE 0xF000 //Endereço de e/s
#define STACK_BASE 0x2000; //pilha - cresce para baixo 

#define OP_JMP    0    // 0000
#define OP_JCOND  1    // 0001
#define OP_LDR    2    // 0010
#define OP_STR    3    // 0011
#define OP_MOV    4    // 0100
#define OP_ADD    5    // 0101
#define OP_ADDI   6    // 0110
#define OP_SUB    7    // 0111
#define OP_SUBI   8    // 1000
#define OP_AND    9    // 1001
#define OP_OR     10   // 1010
#define OP_SHR    11   // 1011
#define OP_SHL    12   // 1100
#define OP_CMP    13   // 1101
#define OP_PUSH   14   // 1110
#define OP_POP    15   // 1111

#define COND_JEQ 0
#define COND_JNE 1
#define COND_JLT 2
#define COND_JGE 3


uint16_t R[16];
uint16_t MEM[MEM_SIZE];
bool accessed[MEM_SIZE];
bool Z, C;
uint16_t PC;
uint16_t SP;


void reset();
uint16_t fetch();
bool execute(uint16_t instr);



int main() {
    reset();

    MEM[0] = 0xFFFF;

    while (1) {
        uint16_t instr = fetch();

        //copiar as variaveis para os registos para se precisar de ler o PC ou SP
        //os valores estão certos em R[15]; 
        R[15] = PC;
        R[14] = SP;

        if (!execute(instr))
            break;
    }

    //copia os registos de volta para as variaveis, se a instrução mandou pular para outro lugar, então atualizamos o PC
    PC = R[15];
    SP = R[14];

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
    int opcode = instr >> 12;
    if (instr == 0xFFFF) {
        return false;
    }
    return true;
}
