#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 65536 // 2^16
#define MEM_LIMIT 0X2000 // Limite de memória de dados - 8kb palavras
#define IO_BASE 0xF000 //Endereço de e/s
#define STACK_BASE 0x2000 //pilha - cresce para baixo 

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

#define COND_JEQ 0 // Z= 1
#define COND_JNE 1 // Z= 0
#define COND_JLT 2 // Z= 0
#define COND_JGE 3 // C= 0


uint16_t R[16];
uint16_t MEM[MEM_SIZE];
bool accessed[MEM_SIZE];
bool Z, C;
uint16_t PC;
uint16_t SP;


void load_program(const char *filename);
void print_state();
int16_t sign_extend(uint16_t val, int bits);
void update_flags(uint32_t res, bool is_sub);

void reset();
uint16_t fetch();
bool execute(uint16_t instr);

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("%s", argv[0]);
        return 1;
    }

    reset();
    load_program(argv[1]);

    MEM[0] = 0xFFFF;

    while (1) {
        uint16_t instr = fetch();

        //copiar as variaveis para os registos para se precisar de ler o PC ou SP
        //os valores estão certos em R[15]; 
        R[15] = PC;
        R[14] = SP;

        if(instr == 0xFFFF){
            printf_stage();
            break;
        }

        if (!execute(instr))
            break;

        PC = R[15];
        SP = R[14];

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
    C = false;
}

uint16_t fetch() {
    uint16_t instr = MEM[PC];
    PC++;
    return instr;
}


int16_t sign_extend(uint16_t val, int bits){
    int shift = 16 - bits;
    int16_t s_val = (int16_t)(val << shift);
    return s_val >> shift;
}


void update_flags(uint32_t res, bool is_sub){
    Z = ((res&0xFFFF) == 0);
    if(is_sub){
        C=(res>0xFFFF); // ajusta se o teste exigir lógica específica de ARM/X86  
    } else {
        C=(res>0xFFFF);
    }
}



bool execute(uint16_t instr) {
    int opcode = instr >> 12;
    // Formato 1 (ALU): Opcode(4) | Rd(4) | Rm(4) | Rn(4)
    uint8_t rd = (instr >> 12) & 0xF;
    uint8_t r_dest = (instr >> 8) & 0xF;
    uint8_t r_src1 = (instr >> 4) & 0xF;
    uint8_t r_src2 = instr & 0xF;

    //imediados
    uint8_t imm8 = instr & 0xFF;
    uint16_t imm12 = instr & 0xFFF;
    uint16_t imm10 = instr & 0x3FF;

    //auxiliares
    uint32_t res = 0;
    int16_t offset = 0;
    uint16_t addr = 0;

    switch (opcode){
            case OP_JMP:
                offset = sing_extend(imm12, 12);
                R[15] += offset;
                break;

            case OP_JCOND:
            {
                    uint8_t cond = (instr >> 10) & 0x3;
                    offset = sing_extend(instr & 0x3FF,10);
                    bool jump = false; 
                    switch(cond){
                        case COND_JEQ: jump = Z; break;
                        case COND_JNE: jump = !Z; break;
                        case COND_JLT: jump = (!Z && C); break;
                        case COND_JGE: jump = (!C && Z); break;
                    }
                    if(jump) R[15] += offset;
                    break;
            }

            case OP_LDR:
                    addr = R[r_src1] + r_src2;
                    if(addr == 0xF002){
                        int val;
                        scanf("%d", &val);
                        R[r_dest] = val;
                    } else if(addr == 0xF000){
                        R[r_dest] = getchar();

                    } else if (addr < MEM_SIZE){
                        R[r_dest] = MEM[addr];
                        accessed[addr] = true;
                    }
                    break;

             case OP_STR:
                    addr = R[r_src1] + r_src2;
                    if(addr == 0xF003){
                        printf("%d\n", (short)R[r_dest]);
                    } else if (addr == 0xF001){
                        printf("%c", (char)R[r_dest]);
                    } else if(addr < MEM_SIZE){
                        MEM[addr] = R[r_dest];
                        accessed[addr] = true;
                    }   
                    break;
            case OP_MOV:
                    R[r_dest]= sign_extend(imm8, 8);
                    break;
            case OP_ADD:
                    res = (uint32_t)R[r_src1] + R[r_src2];
                    R[r_dest] = (uint16_t)res;
                    update_falgs(res, false);
                    break;
            case OP_ADDI:
                    res = (uint32_t)R[r_src1] + r_src2;
                    R[r_dest] = (uint16_t)res;
                    update_flags(res, false);
                    break;
            case OP_SUB:
                    res = (uint32_t) R[r_src1] - R[r_src2];
                    R[r_dest] = (uint16_t)res;
                    update_flags(res, true);
                    break;
            case OP_SUBI:
                    res = (uint32_t)R[r_src1] - r_src2;
                    R[r_dest] = (uint16_t)res;
                    update_flags(res, true);
                    break;
            case OP_AND:
                    R[r_dest] = R[r_src1] & R[r_src2];
                    Z = (R[r_dest] == 0);
                    C = false;
                    break;
            case OP_OR:
                    R[r_dest] = R[r_src1] | R[r_src2];
                    Z = (R[r_dest] == 0);
                    C = false;
                    break;
            case OP_SHR:
                    R[r_dest] = R[r_src1] >> r_src2; 
                    Z = (R[r_dest] == 0);
                    C = false;
                    break;
            case OP_SHL:
                    res = (uint32_t)R[r_src1] << r_src2;
                    R[r_dest] = (uint16_t)res;
                    Z = (R[r_dest] == 0);
                    C = (res > 0xFFFF);
                    break;
            case OP_CMP: 
                    res = (uint32_t)R[r_dest] - R[r_src1]; 
                    update_flags(res, true);
                    break;
            case OP_PUSH: 
                    R[14]--; 
                    if (R[14] < MEM_SIZE) MEM[R[14]] = R[r_dest]; 
                    break;
            case OP_POP: 
                    if (R[14] < MEM_SIZE) R[r_dest] = MEM[R[14]];
                    R[14]++;
                    break;
            default:
            return false;
    }

    if (instr == 0xFFFF) {
        return false;
    }
    return true;
}

void load_program(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Erro ao abrir arquivo %s\n", filename);
        exit(1);
    }



    unsigned int addr, val;
    while (fscanf(fp, "%x %x", &addr, &val) == 2) {
        if (addr < MEM_SIZE) {
            MEM[addr] = (uint16_t)val;
        }
    }
    fclose(fp);
}

void print_state() {
    for (int i = 0; i < 16; i++) printf("R%d=0x%04hX\n", i, R[i]); //registrador
    // flags
    printf("Z=%d\n", Z);
    printf("C=%d\n", C);
    // memória de dados
    for (int i = 0; i < MEM_LIMIT; i++) {
        if (accessed[i]) printf("[%04X]=0x%04hX\n", i, MEM[i]);
    }
    // pilha
    if (SP != STACK_BASE){
        for (int i = STACK_BASE - 1; i >= SP; i--){
             printf("[%04X]=0x%04hX\n", i, MEM[i]);
        }
    }
}
