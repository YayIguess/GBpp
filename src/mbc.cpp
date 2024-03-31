#include "addressSpace.hpp"
#include "gameboy.hpp"

void AddressSpace::determineMBCInfo() {
	MBC = static_cast<MBCType>(memoryLayout.romBank0[0x147]);
	romSize = 32768 * (1 << memoryLayout.romBank0[0x147]);
	romBanks = (1 << (memoryLayout.romBank0[0x147] + 1));

	switch (memoryLayout.romBank0[0x0149]) {
	case 0x02:
		externalRamSize = 8196;
		externalRamBanks = 1;
		break;
	case 0x03:
		externalRamSize = 32768;
		externalRamBanks = 4;
		break;
	case 0x04:
		externalRamSize = 131072;
		externalRamBanks = 16;
		break;
	case 0x05:
		externalRamSize = 65536;
		externalRamBanks = 8;
		break;
	default:
		externalRamSize = 0;
		externalRamBanks = 0;
		break;
	}

	if (MBC == MBC2 || MBC2Battery) {
		//only the lower 4 bits are usable
		externalRamSize = 512;
	}
}

bool GameBoy::testMBCWrite(const Byte& address) {
	const Byte* ptr = &address;
	if (ptr >= &addressSpace[0x0] && ptr <= &addressSpace[0x7FFF])
		return true;
	return false;
}

