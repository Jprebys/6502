#include <cstdint>
#include <array>

#define BYTES_PER_PAGE 256
#define PAGES          256
#define ADDRESS_BYTES  BYTES_PER_PAGE*PAGES

using BYTE = uint8_t;
using WORD = uint16_t;

struct CPU {
	// BYTE RAM[ADDRESS_BYTES]; 
	std::array<BYTE, ADDRESS_BYTES> RAM;

	CPU() {RAM = { 0 };}
};

int main(void)
{
	CPU cpu;
	return 0;
}