#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BYTES_PER_PAGE 256
#define PAGES          256
#define ADDRESS_BYTES  BYTES_PER_PAGE*PAGES
#define STACK_START    0x0100
#define STACK_END      0x01FF
#define N_INSTRUCTIONS 256


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
	Instruction inst_table[N_INSTRUCTIONS];

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


	uint16_t operand;

} CPU;


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
	fprintf(f, "\nStack bytes:\n");
	for (size_t i = STACK_START; i <= STACK_END; ++i)
	{
		if (i % 8 == 0)
			fprintf(f, "\n");
		fprintf(f, "%2X ", cpu->memory[i]);
	}

	fprintf(f, "\n\nRegisters:\n A:%4hu X:%4hu Y:%4hu\n", cpu->A, cpu->X, cpu->Y);
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


int main(void)
{
	// size_t file_len;
	// char *fname = "program";
	// uint8_t *bytes = read_file_as_bytes(fname, &file_len);
	// for (size_t i = 0; i < file_len; ++i)
		// printf("%2X ", bytes[i]);
	// printf("\n");

	CPU *cpu = init_cpu();
	dump_cpu(cpu, stdout);

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
	cpu->PC += 1;
}

// The operand of an immediate instruction is only one byte, and denotes a constant value
void immediate(CPU *cpu, uint8_t *bytes)
{
	cpu->operand = bytes[cpu->PC + 1];
	cpu->PC += 2;
}

// The operand of a zeropage instruction is one byte, and denotes an address in the zero page
void zero_page(CPU *cpu, uint8_t *bytes)
{
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
}

// A zero page memory address offset by X
void zero_offset_x(CPU *cpu, uint8_t *bytes)
{
	uint8_t index = (bytes[cpu->PC + 1] + cpu->X) % 256;
	cpu->operand = cpu->memory[index];
	cpu->PC += 2;
}

// A zero page memory address offset by Y
void zero_offset_y(CPU *cpu, uint8_t *bytes)
{
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

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 3;
}

void zero_indirect_x(CPU *cpu, uint8_t *bytes)
{
	uint8_t little, big, *val;
	val = &cpu->memory[bytes[cpu->PC + 1] + cpu->X];
	little = *val;
	big = *(val + 1);
	uint16_t addr = (uint16_t)big << 8 | little;

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

	cpu->operand = cpu->memory[addr + cpu->Y];
	cpu->PC += 2;
}


// INSTRUCTIONS
// details: https://llx.com/Neil/a2/opcodes.html
// more: http://www.emulator101.com/reference/6502-reference.html

// group 1
void ORA(CPU *cpu)
{

}

void AND(CPU *cpu)
{
	
}

void EOR(CPU *cpu)
{
	
}

void ADC(CPU *cpu)
{
	
}

void STA(CPU *cpu)
{
	
}

void LDA(CPU *cpu)
{
	
}

void CMP(CPU *cpu)
{
	
}

void SBC(CPU *cpu)
{
	
}


// group 2
void ASL(CPU *cpu)
{
	
}

void ROL(CPU *cpu)
{
	
}

void LSR(CPU *cpu)
{
	
}

void ROR(CPU *cpu)
{
	
}

void STX(CPU *cpu)
{
	
}

void LDX(CPU *cpu)
{
	
}

void DEC(CPU *cpu)
{
	
}


// group 3
void BIT(CPU *cpu)
{
	
}

void JMP(CPU *cpu)
{
	
}

void STY(CPU *cpu)
{
	
}

void LDY(CPU *cpu)
{
	
}

void CPY(CPU *cpu)
{
	
}

void CPX(CPU *cpu)
{
	
}


// conditional branch
void BPL(CPU *cpu)
{

}

void BMI(CPU *cpu)
{

}

void BVC(CPU *cpu)
{

}

void BVS(CPU *cpu)
{

}

void BCC(CPU *cpu)
{

}

void BCS(CPU *cpu)
{

}

void BNE(CPU *cpu)
{

}

void BEQ(CPU *cpu)
{

}


// other
void BRK(CPU *cpu)
{

}

void JSR(CPU *cpu)
{

}

void RTI(CPU *cpu)
{

}

void RTS(CPU *cpu)
{

}


// unused instruction
void XXX(CPU *cpu);
