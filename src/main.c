#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BYTES_PER_PAGE 256
#define PAGES          256
#define ADDRESS_BYTES  BYTES_PER_PAGE*PAGES
#define STACK_START    0x0100
#define STACK_END      0x01FF

typedef struct {
	uint8_t memory[ADDRESS_BYTES]; 

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
} CPU;


void init_cpu(CPU *cpu)
{
	memset(cpu, 0, sizeof(CPU));
}

void dump_cpu(CPU *cpu)
{
	//print stack bytes
	for (size_t i = STACK_START; i <= STACK_END; ++i)
	{
		if (i % 8 == 0)
			printf("\n");
		printf("%2X ", cpu->memory[i]);
	}
	printf("\n");

	//TODO - print registers
}


int main(void)
{
	CPU cpu;
	init_cpu(&cpu);

	dump_cpu(&cpu);


	return 0;
}