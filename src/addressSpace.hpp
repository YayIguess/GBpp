#ifndef ADDRESSSPACE_HPP
#define ADDRESSSPACE_HPP
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "defines.hpp"

class AddressSpace {
	bool bootromLoaded = true;
	Byte bootrom[BOOTROM_SIZE] = {0};

public:
	std::vector<Byte> game;

	AddressSpace() {
		// Initialize the memory to zero
		memoryLayout = {};
		std::memset(memoryLayout.memory, 0, sizeof(memoryLayout.memory));
	}

	// Nested union for the memory layout
	union MemoryLayout {
		Byte memory[0x10000];

		struct {
			Byte romBank0[ROM_BANK_SIZE]; // Mapped to 0x0000
			Byte romBankSwitch[ROM_BANK_SIZE]; // Mapped to 0x4000
			Byte vram[0x2000]; // Mapped to 0x8000
			Byte externalRam[0x2000]; // Mapped to 0xA000
			Byte memoryBank1[0x1000]; // Mapped to 0xC000
			Byte memoryBank2[0x1000]; // Mapped to 0xD000
			Byte echoRam[0x1E00]; // Mapped to 0xE000 (Echo RAM, mirrors 0xC000 to 0xDFFF)
			Byte spriteAttributeTable[0xA0]; // Mapped to 0xFE00
			Byte notUsable[0x60]; // Mapped to 0xFEA0
			Byte io[0x80]; // Mapped to 0xFF00, 0xFF0F is interrupt flag
			Byte specialRam[0x7F]; // Mapped to 0xFF80
			Byte interuptEnableReg; // Mapped to 0xFFFF
		};
	} memoryLayout{};

	void unmapBootrom();
	void mapBootrom();
	bool getBootromState() const;
	void loadBootrom(const std::string& filename);
	void loadGame(const std::string& filename);

	void determineMBCInfo();
	MBCType MBC = {};
	uint32_t romSize = 0;
	uint32_t romBanks = 0;
	uint32_t externalRamSize = 0;
	uint32_t externalRamBanks = 0;
	Byte romBankRegister = 0x01;
	Byte ramBankRegister = 0x00;
	Byte romRamSelect = 0x00;
	//MBC3
	Byte latchClockData = 0x00;
	Byte ramBankRTCRegister = 0x00;

	//overload [] for echo ram and bootrom support
	Byte operator[](const uint32_t address) const {
		if (address >= 0xE000 && address < 0xFE00)
			return memoryLayout.echoRam[address - 0x2000];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];

		return memoryLayout.memory[address];
	}

	Byte& operator[](const uint32_t address) {
		if (address >= 0xE000 && address < 0xFE00)
			return memoryLayout.echoRam[address - 0x2000];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];

		return memoryLayout.memory[address];
	}
};


#endif //ADDRESSSPACE_HPP
