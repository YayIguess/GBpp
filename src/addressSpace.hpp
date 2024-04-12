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
	std::vector<Byte> game;
	bool testing;
	Byte testRam[0xFFFF];
	Byte* cartridgeRam = nullptr;

public:
	AddressSpace() {
		// Initialize the memory to zero
		memoryLayout = {};
	}

	struct {
		Byte* romBank0; //[ROM_BANK_SIZE] Mapped to 0x0000
		Byte* romBankSwitch; //[ROM_BANK_SIZE] Mapped to 0x4000
		Byte vram[0x2000]; //Mapped to 0x8000
		Byte* externalRam; //[0x2000]; Mapped to 0xA000
		Byte memoryBank1[0x1000]; //Mapped to 0xC000
		Byte memoryBank2[0x1000]; //Mapped to 0xD000
		Byte oam[0xA0]; //Mapped to 0xFE00
		Byte notUsable[0x60]; //Mapped to 0xFEA0
		//General purpose hardware registers
		Byte JOYP;
		Byte SB;
		Byte SC;
		Byte DIV;
		//Timer registers
		Byte TIMA;
		Byte TMA;
		Byte TAC = 0xF8;
		//interrupt flag and enable
		Byte IF = 0xE1;;
		//Sound registers
		Byte NR10;
		Byte NR11;
		Byte NR12;
		Byte NR13;
		Byte NR14;
		Byte NR20; //not used
		Byte NR21;
		Byte NR22;
		Byte NR23;
		Byte NR24;
		Byte NR30;
		Byte NR31;
		Byte NR32;
		Byte NR33;
		Byte NR34;
		Byte NR40; //unused
		Byte NR41;
		Byte NR42;
		Byte NR43;
		Byte NR44;
		Byte NR50;
		Byte NR51;
		Byte NR52;
		Byte waveRam[0x10];
		//PPU registers
		Byte LCDC;
		Byte STAT;
		Byte SCY;
		Byte SCX;
		Byte LY;
		Byte LYC;
		Byte DMA;
		Byte BGP;
		Byte OBP0;
		Byte OBP1;
		Byte WY;
		Byte WX;
		Byte specialRam[0x7F]; //Mapped to 0xFF80
		Byte IE; // Mapped to 0xFFFF
	} memoryLayout{};

	void unmapBootrom();
	void mapBootrom();
	bool getBootromState() const;
	void loadBootrom(const std::string& filename);
	void loadGame(const std::string& filename);

	void determineMBCInfo();
	static bool testMBCWrite(Word address);
	Byte* MBCRead(Word address);
	//prevents seg faults when programs with no MBC try to write to ROM
	Byte dummyVal = 0;
	void MBCUpdate();
	void loadRomBank();
	void createRamBank();
	void loadRamBank();
	MBCType MBC = {};
	uint32_t romSize = 0;
	uint32_t romBanks = 0;
	uint32_t externalRamSize = 0;
	uint32_t externalRamBanks = 0;

	//Selected ROM Bank = (Secondary Bank << 5) + ROM Bank
	Byte selectedRomBank = 0;
	Byte romBankRegister = 0x00;
	//2 bit register acts as secondary rom bank register or ram bank number
	Byte twoBitBankRegister = 0x0;
	Byte selectedExternalRamBank = 0;
	Byte romRamSelect = 0x00;
	Byte ramEnable = 0x00;
	//MBC3
	Byte latchClockData = 0x00;
	Byte ramBankRTCRegister = 0x00;

	void setTesting(bool state);

	//read
	Byte operator[](const Word address) const {
		if (testing)
			return testRam[address];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];
		if (address < 0x4000)
			return memoryLayout.romBank0[address];
		if (address < 0x8000)
			return memoryLayout.romBankSwitch[address - 0x4000];
		if (address < 0xA000)
			return memoryLayout.vram[address - 0x8000];
		if (address < 0xC000) {
			if (externalRamSize == 0)
				return 0xFF;
			return memoryLayout.externalRam[address - 0xA000];
		}
		if (address < 0xD000)
			return memoryLayout.memoryBank1[address - 0xC000];
		if (address < 0xE000)
			return memoryLayout.memoryBank2[address - 0xD000];
		if (address < 0xFE00)
			return memoryLayout.memoryBank1[address - 0xE000];
		if (address < 0xFEA0)
			return memoryLayout.oam[address - 0xFE00];
		if (address < 0xFF00) {
			if ((memoryLayout.STAT & 0x03) == 2 || (memoryLayout.STAT & 0x03) == 3)
				return 0xFF;
			return 0x00;
		}
		if (address < 0xFF80)
			switch (address) {
			case 0xFF00:
				return memoryLayout.JOYP;
			case 0xFF01:
				return memoryLayout.SB;
			case 0xFF02:
				return memoryLayout.SC;
			case 0xFF04:
				return memoryLayout.DIV;
			case 0xFF05:
				return memoryLayout.TIMA;
			case 0xFF06:
				return memoryLayout.TMA;
			case 0xFF07:
				return memoryLayout.TAC | 0xF8;;
			case 0xFF0F:
				return memoryLayout.IF | 0xE0;
			case 0xFF10:
				return memoryLayout.NR10;
			case 0xFF11:
				return memoryLayout.NR11;
			case 0xFF12:
				return memoryLayout.NR12;
			case 0xFF13:
				return memoryLayout.NR13;
			case 0xFF14:
				return memoryLayout.NR14;
			case 0xFF16:
				return memoryLayout.NR21;
			case 0xFF17:
				return memoryLayout.NR22;
			case 0xFF18:
				return memoryLayout.NR23;
			case 0xFF19:
				return memoryLayout.NR24;
			case 0xFF1A:
				return memoryLayout.NR30;
			case 0xFF1B:
				return memoryLayout.NR31;
			case 0xFF1C:
				return memoryLayout.NR32;
			case 0xFF1D:
				return memoryLayout.NR33;
			case 0xFF1E:
				return memoryLayout.NR34;
			case 0xFF20:
				return memoryLayout.NR41;
			case 0xFF21:
				return memoryLayout.NR42;
			case 0xFF22:
				return memoryLayout.NR43;
			case 0xFF23:
				return memoryLayout.NR44;
			case 0xFF24:
				return memoryLayout.NR50;
			case 0xFF25:
				return memoryLayout.NR51;
			case 0xFF26:
				return memoryLayout.NR52;
			// PPU registers
			case 0xFF40:
				return memoryLayout.LCDC;
			case 0xFF41:
				return memoryLayout.STAT;
			case 0xFF42:
				return memoryLayout.SCY;
			case 0xFF43:
				return memoryLayout.SCX;
			case 0xFF44:
				//for debugging only
				//return 0x90;
				return memoryLayout.LY;
			case 0xFF45:
				return memoryLayout.LYC;
			case 0xFF46:
				return memoryLayout.DMA;
			case 0xFF47:
				return memoryLayout.BGP;
			case 0xFF48:
				return memoryLayout.OBP0;
			case 0xFF49:
				return memoryLayout.OBP1;
			case 0xFF4A:
				return memoryLayout.WY;
			case 0xFF4B:
				return memoryLayout.WX;
			default:
				if (address >= 0xFF30 && address <= 0xFF3F) {
					return memoryLayout.waveRam[address - 0xFF30];
				}
				return 0xFF;
			}
		if (address < 0xFFFF)
			return memoryLayout.specialRam[address - 0xFF80];
		//0xFFFF
		return memoryLayout.IE;
	}

	//write
	Byte& operator[](const Word address) {
		dummyVal = 0xFF;
		if (testing)
			return testRam[address];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];
		if (address < 0x8000)
			return (*MBCRead(address));
		if (address < 0xA000)
			return memoryLayout.vram[address - 0x8000];
		if (address < 0xC000)
			return memoryLayout.externalRam[address - 0xA000];
		if (address < 0xD000)
			return memoryLayout.memoryBank1[address - 0xC000];
		if (address < 0xE000)
			return memoryLayout.memoryBank2[address - 0xD000];
		if (address < 0xFE00)
			return memoryLayout.memoryBank1[address - 0xE000];
		if (address < 0xFEA0)
			return memoryLayout.oam[address - 0xFE00];
		if (address < 0xFF00)
			return memoryLayout.notUsable[address - 0xFEA0];
		if (address < 0xFF80)
			switch (address) {
			case 0xFF00:
				return memoryLayout.JOYP;
			case 0xFF01:
				return memoryLayout.SB;
			case 0xFF02:
				return memoryLayout.SC;
			case 0xFF04:
				memoryLayout.DIV = 0;
				return dummyVal;
			// Timer registers
			case 0xFF05:
				return memoryLayout.TIMA;
			case 0xFF06:
				return memoryLayout.TMA;
			case 0xFF07:
				return memoryLayout.TAC;
			case 0xFF0F:
				return memoryLayout.IF;
			case 0xFF10:
				return memoryLayout.NR10;
			case 0xFF11:
				return memoryLayout.NR11;
			case 0xFF12:
				return memoryLayout.NR12;
			case 0xFF13:
				return memoryLayout.NR13;
			case 0xFF14:
				return memoryLayout.NR14;
			case 0xFF16:
				return memoryLayout.NR21;
			case 0xFF17:
				return memoryLayout.NR22;
			case 0xFF18:
				return memoryLayout.NR23;
			case 0xFF19:
				return memoryLayout.NR24;
			case 0xFF1A:
				return memoryLayout.NR30;
			case 0xFF1B:
				return memoryLayout.NR31;
			case 0xFF1C:
				return memoryLayout.NR32;
			case 0xFF1D:
				return memoryLayout.NR33;
			case 0xFF1E:
				return memoryLayout.NR34;
			case 0xFF20:
				return memoryLayout.NR41;
			case 0xFF21:
				return memoryLayout.NR42;
			case 0xFF22:
				return memoryLayout.NR43;
			case 0xFF23:
				return memoryLayout.NR44;
			case 0xFF24:
				return memoryLayout.NR50;
			case 0xFF25:
				return memoryLayout.NR51;
			case 0xFF26:
				return memoryLayout.NR52;
			case 0xFF40:
				return memoryLayout.LCDC;
			case 0xFF41:
				return memoryLayout.STAT;
			case 0xFF42:
				return memoryLayout.SCY;
			case 0xFF43:
				return memoryLayout.SCX;
			case 0xFF44:
				dummyVal = memoryLayout.LY;
				return dummyVal;
			case 0xFF45:
				return memoryLayout.LYC;
			case 0xFF46:
				return memoryLayout.DMA;
			case 0xFF47:
				return memoryLayout.BGP;
			case 0xFF48:
				return memoryLayout.OBP0;
			case 0xFF49:
				return memoryLayout.OBP1;
			case 0xFF4A:
				return memoryLayout.WY;
			case 0xFF4B:
				return memoryLayout.WX;
			default:
				if (address >= 0xFF30 && address <= 0xFF3F) {
					return memoryLayout.waveRam[address - 0xFF30];
				}
				return dummyVal;
			}
		if (address < 0xFFFF)
			return memoryLayout.specialRam[address - 0xFF80];
		//0xFFFF
		return memoryLayout.IE;
	}
};


#endif //ADDRESSSPACE_HPP
