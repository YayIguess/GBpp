#ifndef GBPP_SRC_GAMEBOY_HPP_
#define GBPP_SRC_GAMEBOY_HPP_

#include <filesystem>
#include <cstdint>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <SDL.h>
#include "defines.hpp"

//two bits per colour
enum Colour {
	black = 0b11,
	darkGray = 0b10,
	lightGray = 0b01,
	white = 0b00
};

enum PPUMode {
	mode0, // Horizontal Blank (Mode 0): No access to video RAM, occurs during horizontal blanking period.
	mode1, // Vertical Blank (Mode 1): No access to video RAM, occurs during vertical blanking period.
	mode2, // OAM Search (Mode 2): Access to OAM (Object Attribute Memory) only, sprite evaluation.
	mode3 // Pixel Transfer (Mode 3): Access to both OAM and video RAM, actual pixel transfer to the screen.
};

union RegisterPair {
	Word reg; //register.reg == (hi << 8) + lo. (hi is more significant than lo)

	struct {
		Byte lo;
		Byte hi;
	};
};

class AddressSpace {
private:
	bool bootromLoaded = true;
	Byte bootrom[BOOTROM_SIZE];
	std::ifstream game;

public:
	AddressSpace() {
		// Initialize the memory to zero
		memoryLayout = {};
		std::memset(memoryLayout.memory, 0, sizeof(memoryLayout.memory));
	}

	// Nested union for the memory layout
	union MemoryLayout {
		Byte memory[0x10000];

		struct {
			Byte romBank1[ROM_BANK_SIZE]; // Mapped to 0x0000
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
	} memoryLayout;

	bool getBootromState();
	void unmapBootrom();
	void mapBootrom();
	void loadBootrom(std::string filename);
	void loadGame(std::string filename);

	//overload [] for echo ram and bootrom support
	Byte operator[](uint32_t address) const {
		if (address >= 0xE000 && address < 0xFE00)
			return memoryLayout.echoRam[address - 0xE000];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];
		else
			return memoryLayout.memory[address];
	}

	Byte& operator[](uint32_t address) {
		if (address >= 0xE000 && address < 0xFE00)
			return memoryLayout.echoRam[address - 0xE000];
		if (address < 0x0100 && bootromLoaded)
			return bootrom[address];
		else
			return memoryLayout.memory[address];
	}
};

class GameBoy {
private:
	uint32_t cycles = 0;
	uint32_t lastOpTicks = 0;
	uint32_t lastRefresh = 0;
	uint32_t lastScanline = 0;
	uint32_t cyclesToStayInHblank = -1;

	uint8_t IME = 0; //enables interupts

	//Accumulator and flags
	RegisterPair AF;
	//General purpose CPU registers
	RegisterPair BC;
	RegisterPair DE;
	RegisterPair HL;

	Word SP = 0xFFFE; //stack pointer
	Word PC = 0x0000; //program counter

	AddressSpace addressSpace;

	//General purpose hardware registers
	Byte* JOYP = &addressSpace[0xFF00];
	Byte* SB = &addressSpace[0xFF01];
	Byte* SC = &addressSpace[0xFF02];
	Byte* DIV = &addressSpace[0xFF04];

	//Timer registers
	Byte* TIMA = &addressSpace[0xFF05];
	Byte* TMA = &addressSpace[0xFF15]; //unused
	Byte* TAC = &addressSpace[0xFF16];

	//interrupt flag and enable
	Byte* IF = &addressSpace[0xFF0F];
	Byte* IE = &addressSpace[0xFFFF];

	//Sound registers
	Byte* NR10 = &addressSpace[0xFF10];
	Byte* NR11 = &addressSpace[0xFF11];
	Byte* NR12 = &addressSpace[0xFF12];
	Byte* NR13 = &addressSpace[0xFF13];
	Byte* NR14 = &addressSpace[0xFF14];
	Byte* NR20 = &addressSpace[0xFF15]; //unused
	Byte* NR21 = &addressSpace[0xFF16];
	Byte* NR22 = &addressSpace[0xFF17];
	Byte* NR23 = &addressSpace[0xFF18];
	Byte* NR24 = &addressSpace[0xFF19];
	Byte* NR30 = &addressSpace[0xFF1A];
	Byte* NR31 = &addressSpace[0xFF1B];
	Byte* NR32 = &addressSpace[0xFF1C];
	Byte* NR33 = &addressSpace[0xFF1D];
	Byte* NR34 = &addressSpace[0xFF1E];
	Byte* NR40 = &addressSpace[0xFF1F]; //unused
	Byte* NR41 = &addressSpace[0xFF20];
	Byte* NR42 = &addressSpace[0xFF21];
	Byte* NR43 = &addressSpace[0xFF22];
	Byte* NR44 = &addressSpace[0xFF23];
	Byte* NR50 = &addressSpace[0xFF24];
	Byte* NR51 = &addressSpace[0xFF25];
	Byte* NR52 = &addressSpace[0xFF26];
	Byte* waveRam = &addressSpace[0xFF30]; //WaveRam[0x10]

	//PPU registers
	Byte* LCDC = &addressSpace[0xFF40];
	Byte* STAT = &addressSpace[0xFF41];
	Byte* SCY = &addressSpace[0xFF42];
	Byte* SCX = &addressSpace[0xFF43];
	Byte* LY = &addressSpace[0xFF44];
	Byte* LYC = &addressSpace[0xFF45];
	Byte* DMA = &addressSpace[0xFF46];
	Byte* BGP = &addressSpace[0xFF47];
	Byte* OBP0 = &addressSpace[0xFF48];
	Byte* OBP1 = &addressSpace[0xFF49];
	Byte* WY = &addressSpace[0xFF4A];
	Byte* WX = &addressSpace[0xFF4B];

	PPUMode currentMode;

	//3 colour channels
	uint32_t* framebuffer = new uint32_t[RESOLUTION_X * RESOLUTION_Y * SCREEN_BPP];
	SDL_Window* screen;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Event event;

	void opcodeHandler();
	void incLY();
	void ppuUpdate();
	void drawLine();
	void SDL2present();

	void checkPPUMode();
	void setPPUMode(PPUMode mode);
	uint32_t cyclesSinceLastScanline();
	uint32_t cyclesSinceLastRefresh();

	void interruptHandler();
	bool testInterruptEnabled(Byte interrupt);
	void resetInterrupt(Byte interrupt);

	void VBlankHandle();
	void LCDStatHandle();
	void timerHandle();
	void serialHandle();
	void joypadHandle();

	void setFlag(Byte bit);
	void resetFlag(Byte bit);
	bool getFlag(Byte bit) const;

	Word getWordPC();
	Byte getBytePC();
	Word getWordSP();
	Byte getByteSP();

	void addCycles(Byte ticks);

	//OPCODE FUNCTIONS
	template <typename T>
	void ld(T& dest, T src);
	template <typename T>
	void orBitwise(T& dest, T src);
	template <typename T>
	void andBitwise(T& dest, T src);
	template <typename T>
	void xorBitwise(T& dest, T src);
	template <typename T>
	void bit(T testBit, T reg);
	template <typename T>
	void jp(T address);
	template <typename T>
	bool jrNZ(T offset);
	template <typename T>
	void inc(T& reg);
	template <typename T>
	void call(T address);
	void halt();
	void stop();
	template <typename T>
	void ldW(T dest, T src);
	template <typename T>
	void cp(T value);
	template <typename T>
	void dec(T& reg);
	template <typename T>
	bool jrZ(T offset);
	template <typename T>
	void sub(T value);
	template <typename T>
	void jr(T OFFSET);
	template <typename T>
	void push(T reg);
	template <typename T>
	void rl(T& reg);
	template <typename T>
	void pop(T& reg);
	template <typename T>
	void rla(T& reg);
	template <typename T>
	void rst(T address);
	void ret();
	template <typename T>
	void add(T& reg, T value);
	void cpl();
	void ccf();
	void swap(Byte& value);

public:
	void start(std::string bootrom, std::string game);
	void SDL2setup();
	void SDL2destroy();
};

#endif //GBPP_SRC_GAMEBOY_HPP_
