#ifndef _CPU_6502_H
#define _CPU_6502_H

#define BYTES_PER_PAGE 256
#define PAGES          256
#define ADDRESS_BYTES  BYTES_PER_PAGE*PAGES


struct CPU;

typedef struct Instruction 
{
	char name[3];
	void (*operation)(struct CPU *);
	void (*addr_mode)(struct CPU *, uint8_t *);
	uint8_t clock_cycles;
} Instruction;


typedef struct CPU
{
	uint8_t memory[ADDRESS_BYTES];

	// registers
	// described here: https://codebase64.org/doku.php?id=base:6502_registers
	uint16_t PC;         //program counter
	uint8_t  A;          //accumulator
	uint8_t  X;          //index register x
	uint8_t  Y;          //index register y
	uint8_t  SP;         //stack pointer

	// processor flags 
	unsigned int N : 1;  // 7 negative
	unsigned int V : 1;  // 6 overflow
	unsigned int U : 1;  // 5 UNUSED (always set to 1)
	unsigned int B : 1;  // 4 break
	unsigned int D : 1;  // 3 decimal mode
	unsigned int I : 1;  // 2 interrupt disable
	unsigned int Z : 1;  // 1 zero
	unsigned int C : 1;  // 0 carry

	Instruction *current_inst;
	uint8_t operand;
	uint16_t jmp_addr;

} CPU;

void reset_cpu(CPU *);

void implied(CPU *, uint8_t *);
void accumulator(CPU *, uint8_t *);
void relative(CPU *, uint8_t *);
void immediate(CPU *, uint8_t *);
void indirect(CPU *, uint8_t *);
void zero_page(CPU *, uint8_t *);
void absolute(CPU *, uint8_t *);
void zero_offset_x(CPU *, uint8_t *);
void zero_offset_y(CPU *, uint8_t *);
void abs_offset_x(CPU *, uint8_t *);
void abs_offset_y(CPU *, uint8_t *);
void zero_indirect_x(CPU *, uint8_t *);
void zero_indirect_y(CPU *, uint8_t *);


void ORA(CPU *);
void AND(CPU *);
void EOR(CPU *);
void ADC(CPU *);
void STA(CPU *);
void LDA(CPU *);
void CMP(CPU *);
void SBC(CPU *);
void ASL(CPU *);
void ROL(CPU *);
void LSR(CPU *);
void ROR(CPU *);
void STX(CPU *);
void LDX(CPU *);
void INC(CPU *);
void DEC(CPU *);
void BIT(CPU *);
void JMP(CPU *);
void STY(CPU *);
void LDY(CPU *);
void CPY(CPU *);
void CPX(CPU *);
void BPL(CPU *);
void BMI(CPU *);
void BVC(CPU *);
void BVS(CPU *);
void BCC(CPU *);
void BCS(CPU *);
void BNE(CPU *);
void BEQ(CPU *);
void BRK(CPU *);
void JSR(CPU *);
void RTI(CPU *);
void RTS(CPU *);
void PHP(CPU *);
void PLP(CPU *);
void PHA(CPU *);
void PLA(CPU *);
void DEY(CPU *);
void TAY(CPU *);
void INY(CPU *);
void INX(CPU *);
void CLC(CPU *);
void SEC(CPU *);
void CLI(CPU *);
void SEI(CPU *);
void TYA(CPU *);
void CLV(CPU *);
void CLD(CPU *);
void SED(CPU *);
void TXA(CPU *);
void TXS(CPU *);
void TAX(CPU *);
void TSX(CPU *);
void DEX(CPU *);
void NOP(CPU *);

#endif