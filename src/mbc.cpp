#include "addressSpace.hpp"

void AddressSpace::determineMBCInfo() {
	MBC = static_cast<MBCType>(memoryLayout.romBank0[0x147]);
	romSize = 32768 * (1 << memoryLayout.romBank0[0x147]);
	romBanks = 1 << (memoryLayout.romBank0[0x147] + 1);

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

	if (MBC == MBC2 || MBC == MBC2Battery) {
		//only the lower 4 bits are usable
		externalRamSize = 512;
	}
}

bool AddressSpace::testMBCWrite(const Word address) {
	if (address <= 0x7FFF)
		return true;
	return false;
}

Byte* AddressSpace::MBCRead(const Word address) {
	if (MBC == MBC1) {
		if (address <= 0x1FFF)
			return &ramEnable;
		if (address <= 0x3FFF)
			return &romBankRegister;
		if (address <= 0x5FFF)
			return &twoBitBankRegister;
		if (address <= 0x7FFF)
			return &romRamSelect;
	}
	if (MBC == MBC1Ram || MBC == MBC1RamBattery) {
		if (address <= 0x1FFF)
			return &ramEnable;
		//bits 0-4
		if (address <= 0x3FFF)
			return &romBankRegister;
		if (address <= 0x5FFF) {
			return &twoBitBankRegister;
		}
		if (address <= 0x7FFF)
			return &romRamSelect;
	}
	return &dummyVal;
}

void AddressSpace::MBCUpdate() {
	if (MBC == MBC1) {
		//TODO: multicart roms need to be able to switch the first rom bank as well
		//see: https://gbdev.io/pandocs/MBC1.html

		//Selected ROM Bank = (Secondary Bank << 5) + ROM Bank
		romBankRegister &= 0x1F;
		twoBitBankRegister &= 0x3;

		//512 KiB can only have 8KiB of ram
		if (romSize > 524288) {
			if (romBankRegister == 0)
				selectedRomBank = (twoBitBankRegister << 5) + 1;
			selectedRomBank = (twoBitBankRegister << 5) + romBankRegister;
			selectedExternalRamBank = 0;
		}
		else {
			if (romBankRegister == 0)
				selectedRomBank = 1;
			else
				selectedRomBank = romBankRegister;
			selectedExternalRamBank = twoBitBankRegister;
		}
		loadRomBank();
		loadRamBank();
	}
	if (MBC == MBC1Ram || MBC == MBC1RamBattery) {}
}

void AddressSpace::loadRomBank() {
	Byte* old = memoryLayout.romBankSwitch;
	memoryLayout.romBankSwitch = game.data() + (ROM_BANK_SIZE * selectedRomBank);
	if (old != memoryLayout.romBankSwitch)
		printf("\n");
}

void AddressSpace::createRamBank() {
	if (externalRamSize)
		cartridgeRam = new Byte[externalRamSize];
}

void AddressSpace::loadRamBank() {
	Byte* old = memoryLayout.externalRam;
	if (cartridgeRam != nullptr)
		memoryLayout.externalRam = cartridgeRam + (RAM_BANK_SIZE * selectedExternalRamBank);
	if (old != memoryLayout.externalRam)
		printf("\n");
}

