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
	uint16_t A;    //accumulator
	uint16_t X;    //index register x
	uint16_t Y;    //index register y
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


int main(void)
{
	CPU *cpu = init_cpu();
	dump_cpu(cpu, stdout);
	delete_cpu(cpu);

	return 0;
}


// ADDRESSING MODES
// see: https://rosettacode.org/wiki/Category:6502_Assembly#Addressing_Modes

void immediate(CPU *cpu, uint8_t *bytes)
{
	cpu->operand = bytes[0];
}

void zero_page(CPU *cpu, uint8_t *bytes)
{
	
}

void absolute(CPU *cpu, uint8_t *bytes)
{
	
}

void zero_offset_x(CPU *cpu, uint8_t *bytes)
{
	
}

void zero_offset_y(CPU *cpu, uint8_t *bytes)
{
	
}

void abs_offset_x(CPU *cpu, uint8_t *bytes)
{
	
}

void abs_offset_y(CPU *cpu, uint8_t *bytes)
{
	
}

void zero_indirect_x(CPU *cpu, uint8_t *bytes)
{
	
}

void zero_indirect_y(CPU *cpu, uint8_t *bytes)
{
	
}


// INSTRUCTIONS
// details: https://llx.com/Neil/a2/opcodes.html

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
