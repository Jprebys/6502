#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"

#define STACK_START    0x0100
#define STACK_END      0x01FF
#define N_INSTRUCTIONS 256

FILE *assembly_outfile;

// full instr set: https://www.masswerk.at/6502/6502_instruction_set.html
Instruction instruction_table[N_INSTRUCTIONS] = 
{// -0                          -1                                -2                          -3                      -4                              -5                              -6                              -7                      -8                        -9                             -A                            -B                      -C                             -D                             -E                             -F
	{"BRK", BRK, implied, 7},   {"ORA", ORA, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ORA", ORA, zero_page, 3},     {"ASL", ASL, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PHP", PHP, implied, 3}, {"ORA", ORA, immediate, 2},    {"ASL", ASL, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ORA", ORA, absolute, 4},     {"ASL", ASL, absolute, 6},     {"XXX", NULL, NULL, 2}, // 2-
	{"BPL", BPL, relative, 2},  {"ORA", ORA, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ORA", ORA, zero_offset_x, 4}, {"ASL", ASL, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLC", CLC, implied, 2}, {"ORA", ORA, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ORA", ORA, abs_offset_x, 4}, {"ASL", ASL, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 1-
	{"JSR", JSR, absolute, 6},  {"AND", AND, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"BIT", BIT, zero_page, 3},     {"AND", AND, zero_page, 3},     {"ROL", ROL, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PLP", PLP, implied, 4}, {"AND", AND, immediate, 2},    {"ROL", ROL, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"BIT", BIT, absolute, 4},     {"AND", AND, absolute, 4},     {"ROL", ROL, absolute, 6},     {"XXX", NULL, NULL, 2}, // 2-
	{"BMI", BMI, relative, 2},  {"AND", AND, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"AND", AND, zero_offset_x, 4}, {"ROL", ROL, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SEC", SEC, implied, 2}, {"AND", AND, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"AND", AND, abs_offset_x, 4}, {"ROL", ROL, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 3-
	{"RTI", RTI, implied, 6},   {"EOR", EOR, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"EOR", EOR, zero_page, 3},     {"LSR", LSR, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PHA", PHA, implied, 3}, {"EOR", EOR, immediate, 2},    {"LSR", LSR, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"JMP", JMP, absolute, 3},     {"EOR", EOR, absolute, 4},     {"LSR", LSR, absolute, 6},     {"XXX", NULL, NULL, 2}, // 4-
	{"BVC", BVC, relative, 2},  {"EOR", EOR, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"EOR", EOR, zero_offset_x, 4}, {"LSR", LSR, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLI", CLI, implied, 2}, {"EOR", EOR, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"EOR", EOR, abs_offset_x, 4}, {"LSR", LSR, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 5-
	{"RTS", RTS, implied, 6},   {"ADC", ADC, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ADC", ADC, zero_page, 3},     {"ROR", ROR, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PLA", PLA, implied, 4}, {"ADC", ADC, immediate, 2},    {"ROR", ROR, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"JMP", JMP, indirect, 5},     {"ADC", ADC, absolute, 4},     {"ROR", ROR, absolute, 6},     {"XXX", NULL, NULL, 2}, // 6-
	{"BVS", BVS, relative, 2},  {"ADC", ADC, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ADC", ADC, zero_offset_x, 4}, {"ROR", ROR, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SEI", SEI, implied, 2}, {"ADC", ADC, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ADC", ADC, abs_offset_x, 4}, {"ROR", ROR, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 7-
	{"XXX", NULL, NULL, 2},     {"STA", STA, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, zero_page, 3},     {"STA", STA, zero_page, 3},     {"STX", STX, zero_page, 3},     {"XXX", NULL, NULL, 2}, {"DEY", DEY, implied, 2}, {"XXX", NULL, NULL, 2},        {"TXA", TXA, implied, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, absolute, 4},     {"STA", STA, absolute, 4},     {"STX", STX, absolute, 4},     {"XXX", NULL, NULL, 2}, // 8-
	{"BCC", BCC, relative, 2},  {"STA", STA, zero_indirect_y, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, zero_offset_x, 4}, {"STA", STA, zero_offset_x, 4}, {"STX", STX, zero_offset_y, 4}, {"XXX", NULL, NULL, 2}, {"TYA", TYA, implied, 2}, {"STA", STA, abs_offset_y, 5}, {"TXS", TXS, implied, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"STA", STA, abs_offset_x, 5}, {"XXX", NULL, NULL, 2},        {"XXX", NULL, NULL, 2}, // 9-
	{"LDY", LDY, immediate, 2}, {"LDA", LDA, zero_indirect_x, 6}, {"LDX", LDX, immediate, 2}, {"XXX", NULL, NULL, 2}, {"LDY", LDY, zero_page, 3},     {"LDA", LDA, zero_page, 3},     {"LDX", LDX, zero_page, 3},     {"XXX", NULL, NULL, 2}, {"TAY", TAY, implied, 2}, {"LDA", LDA, immediate, 2},    {"TAX", TAX, implied, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, absolute, 4},     {"LDA", LDA, absolute, 4},     {"LDX", LDX, absolute, 4},     {"XXX", NULL, NULL, 2}, // A-
	{"BCS", BCS, relative, 2},  {"LDA", LDA, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, zero_offset_x, 4}, {"LDA", LDA, zero_offset_x, 4}, {"LDX", LDX, zero_offset_y, 4}, {"XXX", NULL, NULL, 2}, {"CLV", CLV, implied, 2}, {"LDA", LDA, abs_offset_y, 4}, {"TSX", TSX, implied, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, abs_offset_x, 4}, {"LDA", LDA, abs_offset_x, 4}, {"LDX", LDX, abs_offset_y, 4}, {"XXX", NULL, NULL, 2}, // B-
	{"CPY", CPY, immediate, 2}, {"CMP", CMP, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"CPY", CPY, zero_page, 3},     {"CMP", CMP, zero_page, 3},     {"DEC", DEC, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"INY", INY, implied, 2}, {"CMP", CMP, immediate, 2},    {"DEX", DEX, implied, 2},     {"XXX", NULL, NULL, 2}, {"CPY", CPY, absolute, 4},     {"CMP", CMP, absolute, 4},     {"DEC", DEC, absolute, 6},     {"XXX", NULL, NULL, 2}, // C-
	{"BNE", BNE, relative, 2},  {"CMP", CMP, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"CMP", CMP, zero_offset_x, 4}, {"DEC", DEC, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLD", CLD, implied, 2}, {"CMP", CMP, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"CMP", CMP, abs_offset_x, 4}, {"DEC", DEC, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // D-
	{"CPX", CPX, immediate, 2}, {"SBC", SBC, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"CPX", CPX, zero_page, 3},     {"SBC", SBC, zero_page, 3},     {"INC", INC, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"INX", INX, implied, 2}, {"SBC", SBC, immediate, 2},    {"NOP", NOP, implied, 2},     {"XXX", NULL, NULL, 2}, {"CPX", CPX, absolute, 4},     {"SBC", SBC, absolute, 4},     {"INC", INC, absolute, 6},     {"XXX", NULL, NULL, 2}, // E-
	{"BEQ", BEQ, relative, 2},  {"SBC", SBC, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"SBC", SBC, zero_offset_x, 4}, {"INC", INC, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SED", SED, implied, 2}, {"SBC", SBC, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"SBC", SBC, abs_offset_x, 4}, {"INC", INC, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // F-
};


CPU *init_cpu()
{
	CPU *cpu = malloc(sizeof(CPU));
	memset(cpu, 0, sizeof(CPU));
	return cpu;
}


void delete_cpu(CPU *cpu)
{
	free(cpu);
}


void dump_cpu(CPU *cpu, FILE *f)
{
	fprintf(f, "Registers:\n A:%4hu X:%4hu Y:%4hu\n", cpu->A, cpu->X, cpu->Y);
	fprintf(f, "PC:%4hu S:%4hu\n", cpu->PC, cpu->S);
	fprintf(f, "Flags:  NVBDIZC\n        %d%d%d%d%d%d%d\n\n", cpu->N, cpu->V, cpu->B, cpu->D, cpu->I, cpu->Z, cpu->C);
}


// This will read the whole file into an array of bytes...
// maybe there is a smarter buffered way to do this
uint8_t *read_file_as_bytes(char *file_name, size_t *file_len)
{
	FILE *f = fopen(file_name, "rb");
	if (f == NULL)
	{
		perror("fopen");
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	*file_len = (size_t)ftell(f);
	rewind(f);

	uint8_t *buffer = malloc(*file_len);
	fread(buffer, 1, *file_len, f);
	fclose(f);

	return buffer;
}


// 6502 assembler: https://www.masswerk.at/6502/assembler.html
void run_program(CPU *cpu, uint8_t *program, size_t file_len, FILE *logfile)
{
	uint8_t opcode;
	Instruction *current_inst;

	fprintf(logfile, "\n");
	for (size_t inst_count = 0; cpu->PC < file_len; ++inst_count) 
	{
		fprintf(logfile, "-----RESULT OF INST %lu-----\n", inst_count);

		opcode = program[cpu->PC];
		current_inst = &instruction_table[opcode];
		cpu->current_inst = current_inst;

		current_inst->addr_mode(cpu, program);
		current_inst->operation(cpu);

		dump_cpu(cpu, stdout);

		// TODO - implement clock
	}
}


int main(void)
{
	size_t file_len;
	char *fname = "program";
	char *outfile_name = "program.asm";
	uint8_t *bytes = read_file_as_bytes(fname, &file_len);

	assembly_outfile = fopen(outfile_name, "w");
	if (assembly_outfile == NULL) 
	{
		perror("fopen outfile");
		exit(EXIT_FAILURE);
	}

	CPU *cpu = init_cpu();
	run_program(cpu, bytes, file_len, stdout);

	fclose(assembly_outfile);
	delete_cpu(cpu);
	return 0;
}


/* 
ADDRESSING MODES
see: https://rosettacode.org/wiki/Category:6502_Assembly#Addressing_Modes
more: http://www.emulator101.com/6502-addressing-modes.html
*/

// Implied instructions have no operands
void implied(CPU *cpu, uint8_t *bytes)
{
	(void) bytes;
	cpu->PC += 1;
	
	fprintf(assembly_outfile, "%s\n", cpu->current_inst->name);
}

// Operand is accumulator
void accumulator(CPU *cpu, uint8_t *bytes)
{
	(void) bytes;
	cpu->operand = cpu->A;
	cpu->PC += 1;

	fprintf(assembly_outfile, "%s A\n", cpu->current_inst->name);
}

// The operand of an immediate instruction is only one byte, and denotes a constant value
void immediate(CPU *cpu, uint8_t *bytes)
{
	cpu->operand = bytes[cpu->PC + 1];
	cpu->PC += 2;

	fprintf(assembly_outfile, "%s #$%02X\n", cpu->current_inst->name, cpu->operand);
}

// The operand of a zeropage instruction is one byte, and denotes an address in the zero page
void zero_page(CPU *cpu, uint8_t *bytes)
{
	fprintf(assembly_outfile, "%s $%02X\n", cpu->current_inst->name, bytes[cpu->PC + 1]);

	cpu->operand = cpu->memory[bytes[cpu->PC + 1]];
	cpu->PC += 2;
}

// The operand of an absolute instruction is two bytes, and denotes an address in memory
void absolute(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big;
	little = bytes[cpu->PC + 1];
	big = bytes[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	cpu->operand = cpu->memory[addr];
	cpu->PC += 3;

	fprintf(assembly_outfile, "%s $%02X%02X\n", cpu->current_inst->name, little, big);
}

// Indirect: operand is address; effective address is contents of word at address
void indirect(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big;
	little = bytes[cpu->PC + 1];
	big = bytes[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X%02X)\n", cpu->current_inst->name, little, big);

	little = cpu->memory[addr];
	big = cpu->memory[addr + 1];

	cpu->operand = (uint16_t)big << 8 | little;
	cpu->PC += 3;
}

// Set the operand to PC + the *signed* byte in the next 
void relative(CPU *cpu, uint8_t *bytes)
{
	int8_t offset = bytes[cpu->PC + 1];

	fprintf(assembly_outfile, "%s $%02X\n", cpu->current_inst->name, offset);

	cpu->PC += offset;
}

// A zero page memory address offset by X
void zero_offset_x(CPU *cpu, uint8_t *bytes)
{
	fprintf(assembly_outfile, "%s $%02X,X\n", cpu->current_inst->name, bytes[cpu->PC + 1]);

	uint8_t index = (bytes[cpu->PC + 1] + cpu->X) % 256;
	cpu->operand = cpu->memory[index];
	cpu->PC += 2;
}

// A zero page memory address offset by Y
void zero_offset_y(CPU *cpu, uint8_t *bytes)
{
	fprintf(assembly_outfile, "%s $%02X,Y\n", cpu->current_inst->name, bytes[cpu->PC + 1]);

	uint8_t index = (bytes[cpu->PC + 1] + cpu->Y) % 256;
	cpu->operand = cpu->memory[index];
	cpu->PC += 2;
}

// An absolute memory address offset by X
void abs_offset_x(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big;
	little = bytes[cpu->PC + 1];
	big = bytes[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,X\n", cpu->current_inst->name, little, big);


	cpu->operand = cpu->memory[addr + cpu->X];
	cpu->PC += 3;
}

// An absolute memory address offset by Y
void abs_offset_y(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big;
	little = bytes[cpu->PC + 1];
	big = bytes[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,Y\n", cpu->current_inst->name, little, big);

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 3;
}

void zero_indirect_x(CPU *cpu, uint8_t *bytes)
{

	uint8_t little, big, addr, *val;
	addr = bytes[cpu->PC + 1] + cpu->X;
	val = &cpu->memory[addr];
	little = *val;
	// "Increments without carry do not affect the hi-byte of an address and no page transitions do occur"
	if (addr == 0xFF)
		big = cpu->memory[0x00];
	else
		big = *(val + 1);
	uint16_t final_addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X,X)\n", cpu->current_inst->name, *val);

	cpu->operand = cpu->memory[final_addr];
	cpu->PC += 2;
}

void zero_indirect_y(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big, *val;
	val = &cpu->memory[bytes[cpu->PC + 1]];
	little = *val;
	big = *(val + 1);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X),Y\n", cpu->current_inst->name, *val);	

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 2;
}


// INSTRUCTIONS
// details: https://llx.com/Neil/a2/opcodes.html
// more: http://www.emulator101.com/reference/6502-reference.html

// group 1
void ORA(CPU *cpu)
{
	(void) cpu;
}

void AND(CPU *cpu)
{
	(void) cpu;
}

void EOR(CPU *cpu)
{
	(void) cpu;
}

// See details for complicated ADC flag settings here:
// https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp#L597
void ADC(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->A + (uint16_t)cpu->operand + (uint16_t)cpu->C;
	cpu->C = temp > 255 ? 1 : 0;
	cpu->Z = temp & 0x00FF ? 1 : 0;
	cpu->V = (~((uint16_t)cpu->A ^ (uint16_t)cpu->operand) & ((uint16_t)cpu->A ^ (uint16_t)temp)) & 0x0080 ? 1 : 0;
	cpu->N = temp & 0x80 ? 1 : 0;

	cpu->A = temp & 0x00FF;
}

void STA(CPU *cpu)
{
	(void) cpu;
}

void LDA(CPU *cpu)
{
	cpu->A = cpu->operand;
}

void CMP(CPU *cpu)
{
	(void) cpu;
}

// See details for complicated ADC flag settings here:
// https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp#L688 
void SBC(CPU *cpu)
{
	(void)cpu;
}


// group 2
void ASL(CPU *cpu)
{
	(void) cpu;
}

void ROL(CPU *cpu)
{
	(void) cpu;
}

void LSR(CPU *cpu)
{
	(void) cpu;
}

void ROR(CPU *cpu)
{
	(void) cpu;
}

void STX(CPU *cpu)
{
	(void) cpu;
}

void LDX(CPU *cpu)
{
	(void) cpu;
}

void INC(CPU *cpu)
{
	(void) cpu;
}

void DEC(CPU *cpu)
{
	(void) cpu;
}


// group 3
void BIT(CPU *cpu)
{
	(void) cpu;
}

void JMP(CPU *cpu)
{
	(void) cpu;
}

void STY(CPU *cpu)
{
	(void) cpu;
}

void LDY(CPU *cpu)
{
	(void) cpu;
}

void CPY(CPU *cpu)
{
	(void) cpu;
}

void CPX(CPU *cpu)
{
	(void) cpu;
}


// conditional branch
void BPL(CPU *cpu)
{
	(void) cpu;
}

void BMI(CPU *cpu)
{
	(void) cpu;
}

void BVC(CPU *cpu)
{
	(void) cpu;
}

void BVS(CPU *cpu)
{
	(void) cpu;
}

void BCC(CPU *cpu)
{
	(void) cpu;
}

void BCS(CPU *cpu)
{
	(void) cpu;
}

void BNE(CPU *cpu)
{
	(void) cpu;
}

void BEQ(CPU *cpu)
{
	(void) cpu;
}


// other
void BRK(CPU *cpu)
{
	(void) cpu;
	printf("BRK received; exiting...\n\n");
	exit(EXIT_SUCCESS);
}

void JSR(CPU *cpu)
{
	(void) cpu;
}

void RTI(CPU *cpu)
{
	(void) cpu;
}

void RTS(CPU *cpu)
{
	(void) cpu;
}

void PHP(CPU *cpu)
{
	(void) cpu;
}

void PLP(CPU *cpu)
{
	(void) cpu;
}

void PHA(CPU *cpu)
{
	(void) cpu;
}

void PLA(CPU *cpu)
{
	(void) cpu;
}

void DEY(CPU *cpu)
{
	(void) cpu;
}

void TAY(CPU *cpu)
{
	(void) cpu;
}

void INY(CPU *cpu)
{
	(void) cpu;
}

void INX(CPU *cpu)
{
	(void) cpu;
}	

void CLC(CPU *cpu)
{
	(void) cpu;
}

void SEC(CPU *cpu)
{
	(void) cpu;
}

void CLI(CPU *cpu)
{
	(void) cpu;
}

void SEI(CPU *cpu)
{
	(void) cpu;
}

void TYA(CPU *cpu)
{
	(void) cpu;
}

void CLV(CPU *cpu)
{
	(void) cpu;
}

void CLD(CPU *cpu)
{
	(void) cpu;
}

void SED(CPU *cpu)
{
	(void) cpu;
}

void TXA(CPU *cpu)
{
	(void) cpu;
}

void TXS(CPU *cpu)
{
	(void) cpu;
}

void TAX(CPU *cpu)
{
	(void) cpu;
}

void TSX(CPU *cpu)
{
	(void) cpu;
}

void DEX(CPU *cpu)
{
	(void) cpu;
}

void NOP(CPU *cpu)
{
	(void) cpu;
}
