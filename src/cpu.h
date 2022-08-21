#ifndef _CPU_6502_H
#define _CPU_6502_H

struct CPU;
struct Instruction;

typedef struct CPU CPU;
typedef struct Instruction Instruction;

void implied(CPU *cpu, uint8_t *bytes);
void accumulator(CPU *cpu, uint8_t *bytes);
void immediate(CPU *cpu, uint8_t *bytes);
void zero_page(CPU *cpu, uint8_t *bytes);
void absolute(CPU *cpu, uint8_t *bytes);
void zero_offset_x(CPU *cpu, uint8_t *bytes);
void zero_offset_y(CPU *cpu, uint8_t *bytes);
void abs_offset_x(CPU *cpu, uint8_t *bytes);
void abs_offset_y(CPU *cpu, uint8_t *bytes);
void zero_indirect_x(CPU *cpu, uint8_t *bytes);
void zero_indirect_y(CPU *cpu, uint8_t *bytes);

void ORA(CPU *cpu);
void AND(CPU *cpu);
void EOR(CPU *cpu);
void ADC(CPU *cpu);
void STA(CPU *cpu);
void LDA(CPU *cpu);
void CMP(CPU *cpu);
void SBC(CPU *cpu);
void ASL(CPU *cpu);
void ROL(CPU *cpu);
void LSR(CPU *cpu);
void ROR(CPU *cpu);
void STX(CPU *cpu);
void LDX(CPU *cpu);
void DEC(CPU *cpu);
void BIT(CPU *cpu);
void JMP(CPU *cpu);
void STY(CPU *cpu);
void LDY(CPU *cpu);
void CPY(CPU *cpu);
void CPX(CPU *cpu);
void BPL(CPU *cpu);
void BMI(CPU *cpu);
void BVC(CPU *cpu);
void BVS(CPU *cpu);
void BCC(CPU *cpu);
void BCS(CPU *cpu);
void BNE(CPU *cpu);
void BEQ(CPU *cpu);
void BRK(CPU *cpu);
void JSR(CPU *cpu);
void RTI(CPU *cpu);
void RTS(CPU *cpu);
void PHP(CPU *cpu);
void PHA(CPU *cpu);
void PLA(CPU *cpu);
void DEY(CPU *cpu);
void TAY(CPU *cpu);
void INY(CPU *cpu);
void INX(CPU *cpu);
void CLC(CPU *cpu);
void SEC(CPU *cpu);
void CLI(CPU *cpu);
void SEI(CPU *cpu);
void TYA(CPU *cpu);
void CLV(CPU *cpu);
void CLD(CPU *cpu);
void SED(CPU *cpu);
void TXA(CPU *cpu);
void TXS(CPU *cpu);
void TAX(CPU *cpu);
void TSX(CPU *cpu);
void DEX(CPU *cpu);
void NOP(CPU *cpu);

#endif