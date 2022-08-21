#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"

#define BYTES_PER_PAGE 256
#define PAGES          256
#define ADDRESS_BYTES  BYTES_PER_PAGE*PAGES
#define STACK_START    0x0100
#define STACK_END      0x01FF
#define N_INSTRUCTIONS 256

FILE *assembly_outfile;


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
	uint16_t PC;   //program counter
	uint8_t A;    //accumulator
	uint8_t X;    //index register x
	uint8_t Y;    //index register y
	uint8_t  S;    //stack pointer

	// processor flags 
	unsigned int N : 1;  //negative
	unsigned int V : 1;  //overflow
	unsigned int B : 1;  //break
	unsigned int D : 1;  //decimal mode
	unsigned int I : 1;  //interrupt disable
	unsigned int Z : 1;  //zero
	unsigned int C : 1;  //carry

	Instruction *current_inst;
	uint16_t operand;

} CPU;


// full instr set: https://www.masswerk.at/6502/6502_instruction_set.html
Instruction instruction_table[] = 
{
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},

	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"LDA", LDA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},
	{"BRK", BRK, implied, 0}, {"ORA", ORA, zero_indirect_x, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, zero_page, 0}, {"ASL", ASL, zero_page, 0}, {"XXX", NULL, NULL, 0}, {"PHP", PHP, implied, 0}, {"ORA", ORA, immediate, 0}, {"ASL", ASL, accumulator, 0}, {"XXX", NULL, NULL, 0}, {"XXX", NULL, NULL, 0}, {"ORA", ORA, absolute, 0}, {"ASL", ASL, absolute, 0}, {"XXX", NULL, NULL, 0},	
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

	// TODO - investigate the 'without carry' condition here
	// "Increments without carry do not affect the hi-byte of an address and no page transitions do occur"
	uint8_t little, big, *val;
	val = &cpu->memory[bytes[cpu->PC + 1] + cpu->X];
	little = *val;
	big = *(val + 1);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X,X)\n", cpu->current_inst->name, *val);

	cpu->operand = cpu->memory[addr];
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

void ADC(CPU *cpu)
{
	(void) cpu;
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

void SBC(CPU *cpu)
{
	(void) cpu;
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
	printf("BRK received; exiting...\n");
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
