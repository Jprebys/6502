#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"

#define STACK_START    0x0100
#define STACK_END      0x01FF
#define STK_PTR_START  0xFD
#define EXEC_ADDR      0xFFFC
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
	reset_cpu(cpu);
	return cpu;
}

void reset_cpu(CPU *cpu)
{
	memset(cpu, 0, sizeof(CPU));
	cpu->U = 1;  // unused flag bit 5 is always 1
	cpu->B = 1;
	cpu->D = 1;

	uint8_t little, big;
	little = cpu->memory[EXEC_ADDR];
	big = cpu->memory[EXEC_ADDR + 1];
	cpu->PC = (uint16_t)big << 8 | little;

	cpu->SP = STK_PTR_START;
}

void delete_cpu(CPU *cpu)
{
	free(cpu);
}

uint8_t get_flags(CPU *cpu)
{
	uint8_t result = 0x00;

	result |= cpu->C << 0;
	result |= cpu->Z << 1;
	result |= cpu->I << 2;
	result |= cpu->D << 3;
	result |= cpu->B << 4;
	result |= cpu->U << 5;
	result |= cpu->V << 6;
	result |= cpu->N << 7;

	return result;
}

void set_flags(CPU *cpu, uint8_t flags)
{
	cpu->C = flags & 1 << 0 ? 1 : 0;
	cpu->Z = flags & 1 << 1 ? 1 : 0;
	cpu->I = flags & 1 << 2 ? 1 : 0;
	cpu->D = flags & 1 << 3 ? 1 : 0;
	cpu->B = flags & 1 << 4 ? 1 : 0;
	cpu->U = flags & 1 << 5 ? 1 : 0;
	cpu->V = flags & 1 << 6 ? 1 : 0;
	cpu->N = flags & 1 << 7 ? 1 : 0;
}

void dump_cpu(CPU *cpu, FILE *f)
{
	fprintf(f, "Registers:\nA:%02X X:%02X Y:%02X ", cpu->A, cpu->X, cpu->Y);
	fprintf(f, "PC:%04X S:%02X\n", cpu->PC, cpu->SP);
	fprintf(f, "Flags:  %02X   NVUBDIZC\n             %d%d1%d%d%d%d%d\n\n", get_flags(cpu), cpu->N, cpu->V, cpu->B, cpu->D, cpu->I, cpu->Z, cpu->C);
}

void inc_stack_ptr(CPU *cpu)
{
	if (cpu->SP == 0xFF)
	{
		fprintf(stderr, "[ERROR] Stack underflow; exiting...");
		exit(EXIT_FAILURE);
	}
	cpu->SP++;
}

void dec_stack_ptr(CPU *cpu)
{
	if (cpu->SP == 0x00)
	{
		fprintf(stderr, "[ERROR] Stack overflow; exiting...");
		exit(EXIT_FAILURE);
	}
	cpu->SP--;
}

void stack_push(CPU *cpu, uint8_t value)
{
	cpu->memory[STACK_START + cpu->SP] = value;
	dec_stack_ptr(cpu);
}

uint8_t stack_pop(CPU *cpu)
{
	inc_stack_ptr(cpu);
	return cpu->memory[STACK_START + cpu->SP];
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
void run_program(CPU *cpu, FILE *logfile)
{
	uint8_t opcode;
	Instruction *current_inst;
	dump_cpu(cpu, stdout);

	fprintf(logfile, "\n");
	for (size_t inst_count = 0; cpu->PC < 0xFFFF; ++inst_count) 
	{
		fprintf(logfile, "-----RESULT OF INST %lu-----\n", inst_count);

		cpu->operand = 0x0000;

		opcode = cpu->memory[cpu->PC];
		current_inst = &instruction_table[opcode];
		cpu->current_inst = current_inst;

		current_inst->addr_mode(cpu);
		current_inst->operation(cpu);

		dump_cpu(cpu, stdout);
		getchar();
		// TODO - implement clock
	}
	
}


int main(void)
{
	size_t file_len;
	char *fname = "nestest.nes";
	uint8_t *bytes = read_file_as_bytes(fname, &file_len);

	assembly_outfile = stdout;
	if (assembly_outfile == NULL) 
	{
		perror("fopen outfile");
		exit(EXIT_FAILURE);
	}

	CPU *cpu = init_cpu();

	memcpy(&cpu->memory[0x8000], bytes + 0x0010, 0xC000 - 0x8000);
	memcpy(&cpu->memory[0xC000], bytes + 0x0010, 0xC000 - 0x8000);
	cpu->PC = 0xC000;

	run_program(cpu, stdout);

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
void implied(CPU *cpu)
{
	
	cpu->PC += 1;
	
	fprintf(assembly_outfile, "%s\n", cpu->current_inst->name);
}

// Operand is accumulator
void accumulator(CPU *cpu)
{
	
	cpu->operand = cpu->A;
	cpu->PC += 1;

	fprintf(assembly_outfile, "%s A\n", cpu->current_inst->name);
}

// The operand of an immediate instruction is only one byte, and denotes a constant value
void immediate(CPU *cpu)
{
	cpu->operand = cpu->memory[cpu->PC + 1];
	cpu->jmp_addr = cpu->operand;
	cpu->PC += 2;

	fprintf(assembly_outfile, "%s #$%02X\n", cpu->current_inst->name, cpu->operand);
}

// The operand of a zeropage instruction is one byte, and denotes an address in the zero page
void zero_page(CPU *cpu)
{
	fprintf(assembly_outfile, "%s $%02X\n", cpu->current_inst->name, cpu->memory[cpu->PC + 1]);

	cpu->jmp_addr = cpu->memory[cpu->PC + 1];
	cpu->operand = cpu->memory[cpu->PC + 1];
	cpu->PC += 2;
}

// The operand of an absolute instruction is two bytes, and denotes an address in memory
void absolute(CPU *cpu)
{
	uint8_t little, big;
	little = cpu->memory[cpu->PC + 1];
	big = cpu->memory[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	cpu->jmp_addr = addr;

	cpu->operand = cpu->memory[addr];
	cpu->PC += 3;

	fprintf(assembly_outfile, "%s $%02X%02X\n", cpu->current_inst->name, big, little);
}

// Indirect: operand is address; effective address is contents of word at address
// only used by JMP
void indirect(CPU *cpu)
{
	uint8_t little, big;
	little = cpu->memory[cpu->PC + 1];
	big = cpu->memory[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X%02X)\n", cpu->current_inst->name, big, little);

	little = cpu->memory[addr];
	if (little == 0xFF)
		little = 0x00;  // no carry bug
	big = cpu->memory[addr + 1];

	cpu->jmp_addr = (uint16_t)big << 8 | little;
	cpu->operand = cpu->memory[cpu->jmp_addr];
	cpu->PC += 3;
}

// Set the operand to PC + the *signed* byte in the next 
void relative(CPU *cpu)
{
	uint8_t offset = cpu->memory[cpu->PC + 1];

	cpu->operand = offset;
	// cpu->jmp_addr = cpu->PC + (int8_t)offset;
	// printf("%u = %u + %d   %u  %d\n", cpu->jmp_addr, cpu->PC, (int8_t)offset, offset, offset);
	cpu->PC += 2;
	fprintf(assembly_outfile, "%s $%04X\n", cpu->current_inst->name, cpu->PC + (int8_t)offset);
}

// A zero page memory address offset by X
void zero_offset_x(CPU *cpu)
{
	fprintf(assembly_outfile, "%s $%02X,X\n", cpu->current_inst->name, cpu->memory[cpu->PC + 1]);

	uint8_t index = (cpu->memory[cpu->PC + 1] + cpu->X) % 256;
	cpu->jmp_addr = (uint16_t)index;	
	cpu->operand = cpu->memory[index];
	cpu->PC += 2;
}

// A zero page memory address offset by Y
void zero_offset_y(CPU *cpu)
{
	fprintf(assembly_outfile, "%s $%02X,Y\n", cpu->current_inst->name, cpu->memory[cpu->PC + 1]);

	uint8_t index = (cpu->memory[cpu->PC + 1] + cpu->Y) % 256;

	cpu->jmp_addr = (uint16_t)index;

	cpu->operand = cpu->memory[index];
	cpu->PC += 2;
}

// An absolute memory address offset by X
void abs_offset_x(CPU *cpu)
{
	uint8_t little, big;
	little = cpu->memory[cpu->PC + 1];
	big = cpu->memory[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,X\n", cpu->current_inst->name, little, big);

	cpu->jmp_addr = addr + cpu->X;

	cpu->operand = cpu->memory[addr + cpu->X];
	cpu->PC += 3;
}

// An absolute memory address offset by Y
void abs_offset_y(CPU *cpu)
{
	uint8_t little, big;
	little = cpu->memory[cpu->PC + 1];
	big = cpu->memory[cpu->PC + 2];
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,Y\n", cpu->current_inst->name, little, big);

	cpu->jmp_addr = addr + cpu->Y;

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 3;
}

void zero_indirect_x(CPU *cpu)
{

	uint8_t little, big, addr, *val;
	addr = cpu->memory[cpu->PC + 1] + cpu->X;
	val = &cpu->memory[addr];
	little = *val;
	// "Increments without carry do not affect the hi-byte of an address and no page transitions do occur"
	if (addr == 0xFF)
		big = cpu->memory[0x00];
	else
		big = *(val + 1);
	uint16_t final_addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X,X)\n", cpu->current_inst->name, *val);

	cpu->jmp_addr = final_addr;

	cpu->operand = cpu->memory[final_addr];
	cpu->PC += 2;
}

void zero_indirect_y(CPU *cpu)
{
	uint8_t little, big, *val;
	val = &cpu->memory[cpu->memory[cpu->PC + 1]];
	little = *val;
	big = *(val + 1);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X),Y\n", cpu->current_inst->name, *val);	

	cpu->jmp_addr = addr + cpu->Y;

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 2;
}


// INSTRUCTIONS
// details: https://llx.com/Neil/a2/opcodes.html
// more: http://www.emulator101.com/reference/6502-reference.html

uint8_t check_negative(uint8_t value)
{
	return value & 0x80 ? 1 : 0;
}

uint8_t check_zero(uint8_t value)
{
	return value ? 0 : 1;
}

uint8_t check_carry(uint8_t value)
{
	return value & 0x80 ? 1 : 0;
}

// group 1
void ORA(CPU *cpu)
{
	cpu->A |= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}

void AND(CPU *cpu)
{
	cpu->A &= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}

void EOR(CPU *cpu)
{
	cpu->A ^= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}


// See details for complicated ADC flag settings here:
// https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp#L597
void ADC(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->A + (uint16_t)cpu->operand + (uint16_t)cpu->C;
	cpu->C = temp > 255 ? 1 : 0;
	cpu->Z = (temp & 0x00FF) == 0 ? 1 : 0;

	/*
	The over flow is set if the two operands of the addition have the same sign, and the result has the opposite sign
	so if the MSB of both operands is 1 and the MSB of the result (temp) is 0, or vice versa,
	then the overflow flag is set. Notice that the final '&' here will result in 
	0x80 if this is true on the left-hand side and it will be 0x00 otherwise.
	*/
	cpu->V = (~((uint16_t)cpu->A ^ (uint16_t)cpu->operand) & ((uint16_t)cpu->A ^ (uint16_t)temp)) & 0x0080 ? 1 : 0;
	cpu->N = temp & 0x80 ? 1 : 0;

	cpu->A = (uint8_t)(temp & 0x00FF);
}

void STA(CPU *cpu)
{
	cpu->memory[cpu->jmp_addr] = cpu->A;
}

void LDA(CPU *cpu)
{
	cpu->A = cpu->operand;
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void CMP(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->A - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->A >= cpu->operand) ? 1 : 0;
}

void SBC(CPU *cpu)
{
	// invert the operand bits, and SBC becomes the same as ADC (i.e. ADC(x) == SBC(~x))
	cpu->operand ^= 0x00FF;

	// TODO - determine if this is sufficient
	ADC(cpu);
}


// group 2
void ASL(CPU *cpu)
{
	uint8_t temp = cpu->operand << 1;
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu->memory[cpu->jmp_addr] = temp;
	cpu->C = check_carry(cpu->operand);
	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}

void ROL(CPU *cpu)
{
	cpu->C = check_carry(cpu->operand);

	uint8_t temp = (cpu->operand << 1) + cpu->C;
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu->memory[cpu->jmp_addr] = temp;

	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}	

void LSR(CPU *cpu)
{
	uint8_t temp = cpu->operand >> 1;
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu->memory[cpu->jmp_addr] = temp;
	cpu->C = cpu->operand & 0x01 ? 1 : 0;
	cpu->N = 0;
	cpu->Z = check_zero(temp);
}

void ROR(CPU *cpu)
{
	cpu->C = check_negative(cpu->operand);

	uint8_t temp = (cpu->operand >> 1) | (cpu->C << 7);

	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu->memory[cpu->jmp_addr] = temp;

	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}

void STX(CPU *cpu)
{
	cpu->memory[cpu->jmp_addr] = cpu->X;
}

void LDX(CPU *cpu)
{
	cpu->X = cpu->operand;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void INC(CPU *cpu)
{
	cpu->operand++;
	cpu->Z = check_zero(cpu->operand);
	cpu->N = check_negative(cpu->operand);
}

void DEC(CPU *cpu)
{
	cpu->operand--;
	cpu->Z = check_zero(cpu->operand);
	cpu->N = check_negative(cpu->operand);
}


// group 3
void BIT(CPU *cpu)
{
	cpu->Z = check_zero((cpu->operand & cpu->A) & 0x00FF);
	cpu->V = cpu->operand & (1 << 6) ? 1 : 0;
	cpu->N = cpu->operand & (1 << 7) ? 1 : 0;
}

void JMP(CPU *cpu)
{
	cpu->PC = cpu->jmp_addr;
}

void STY(CPU *cpu)
{
	cpu->memory[cpu->jmp_addr] = cpu->Y;
}

void LDY(CPU *cpu)
{
	cpu->Y = cpu->operand;
}

void CPY(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->Y - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->Y >= cpu->operand) ? 1 : 0;
}

void CPX(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->X - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->X >= cpu->operand) ? 1 : 0;
}


// conditional branches
void BPL(CPU *cpu)
{
	if (!cpu->N)
		cpu->PC += (int8_t)cpu->operand;
}

void BMI(CPU *cpu)
{
	if (cpu->N)
		cpu->PC += (int8_t)cpu->operand;
}

void BVC(CPU *cpu)
{
	if (!cpu->V)
		cpu->PC += (int8_t)cpu->operand;
}

void BVS(CPU *cpu)
{
	if (cpu->V)
		cpu->PC += (int8_t)cpu->operand;
}

void BCC(CPU *cpu)
{
	if (!cpu->C)
		cpu->PC += (int8_t)cpu->operand;
}

void BCS(CPU *cpu)
{
	if (cpu->C)
		cpu->PC += (int8_t)cpu->operand;
}

void BNE(CPU *cpu)
{
	if (!cpu->Z)
		cpu->PC += (int8_t)cpu->operand;
}

void BEQ(CPU *cpu)
{
	if (cpu->Z)
		cpu->PC += (int8_t)cpu->operand;
}


// other
void BRK(CPU *cpu)
{
	// TODO - implement BRK correctly
	(void) cpu;
	printf("BRK received; exiting...\n\n");
	exit(EXIT_SUCCESS);
}

void JSR(CPU *cpu)
{
	// TODO - verify that we want the high bit of this on the stack
	stack_push(cpu, (cpu->jmp_addr >> 8) & 0x00FF); 

	cpu->PC = cpu->jmp_addr;
}

void RTI(CPU *cpu)
{
	uint8_t B = cpu->B;
	set_flags(cpu, stack_pop(cpu));
	cpu->B = B;

	cpu->PC = stack_pop(cpu);
	cpu->PC |= stack_pop(cpu) << 8;
}

void RTS(CPU *cpu)
{
	cpu->PC = stack_pop(cpu);
	cpu->PC |= stack_pop(cpu) << 8;
	cpu->PC++;
}

void PHP(CPU *cpu)
{
	stack_push(cpu, get_flags(cpu));
}

void PLP(CPU *cpu)
{
	set_flags(cpu, stack_pop(cpu));
}

void PHA(CPU *cpu)
{
	stack_push(cpu, cpu->A);
}

void PLA(CPU *cpu)
{
	cpu->A = stack_pop(cpu);
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void DEY(CPU *cpu)
{
	cpu->Y--;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void TAY(CPU *cpu)
{
	cpu->Y = cpu->A;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void INY(CPU *cpu)
{
	cpu->Y++;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void INX(CPU *cpu)
{
	cpu->X++;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}	

void CLC(CPU *cpu)
{
	cpu->C = 0;
}

void SEC(CPU *cpu)
{
	cpu->C = 1;
}

void CLI(CPU *cpu)
{
	cpu->I = 0;
}

void SEI(CPU *cpu)
{
	cpu->I = 1;
}

void TYA(CPU *cpu)
{
	cpu->A = cpu->Y;
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void CLV(CPU *cpu)
{
	cpu->V = 0;
}

void CLD(CPU *cpu)
{
	cpu->D = 0;
}

void SED(CPU *cpu)
{
	cpu->D = 1;
}

void TXA(CPU *cpu)
{
	cpu->A = cpu->X;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void TXS(CPU *cpu)
{
	stack_push(cpu, cpu->X);
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void TAX(CPU *cpu)
{
	cpu->X = cpu->A;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void TSX(CPU *cpu)
{
	cpu->X = stack_pop(cpu);
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void DEX(CPU *cpu)
{
	cpu->X--;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void NOP(CPU *cpu)
{
	(void) cpu;
}
