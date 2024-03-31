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
#include "addressSpace.hpp"

union RegisterPair {
	Word reg; //register.reg == (hi << 8) + lo. (hi is more significant than lo)

	struct {
		Byte lo;
		Byte hi;
	};
};

class GameBoy {
	//T-cycles not M-cycles (4 T-cycles = 1 M-cycle)
	uint64_t cycles = 0;
	//Start at 2 T-cycles https://github.com/Gekkio/mooneye-test-suite/blob/main/acceptance/ppu/lcdon_timing-GS.s
	uint64_t ppuCycles = 2;
	bool ppuEnabled = false;
	uint64_t lastOpTicks = 0;
	uint64_t lastRefresh = 0;
	uint64_t lastScanline = 0;
	uint64_t cyclesToStayInHblank = -1;
	uint64_t lastDivUpdate = 0;
	bool rendered = false;

	uint8_t IME = 0; //enables interupts

	//Accumulator and flags
	RegisterPair AF = {0};
	//General purpose CPU registers
	RegisterPair BC = {0};
	RegisterPair DE = {0};
	RegisterPair HL = {0};

	Word SP = 0xFFFE; //stack pointer
	Word PC = 0x0000; //program counter

	AddressSpace addressSpace;

	//General purpose hardware registers
	Byte* const JOYP = &addressSpace[0xFF00];
	Byte* const SB = &addressSpace[0xFF01];
	Byte* const SC = &addressSpace[0xFF02];
	Byte* const DIV = &addressSpace[0xFF04];

	//Timer registers
	Byte* const TIMA = &addressSpace[0xFF05];
	Byte* const TMA = &addressSpace[0xFF15]; //unused
	Byte* const TAC = &addressSpace[0xFF16];

	//interrupt flag and enable
	Byte* const IF = &addressSpace[0xFF0F];
	Byte* const IE = &addressSpace[0xFFFF];

	//Sound registers
	Byte* const NR10 = &addressSpace[0xFF10];
	Byte* const NR11 = &addressSpace[0xFF11];
	Byte* const NR12 = &addressSpace[0xFF12];
	Byte* const NR13 = &addressSpace[0xFF13];
	Byte* const NR14 = &addressSpace[0xFF14];
	Byte* const NR20 = &addressSpace[0xFF15]; //unused
	Byte* const NR21 = &addressSpace[0xFF16];
	Byte* const NR22 = &addressSpace[0xFF17];
	Byte* const NR23 = &addressSpace[0xFF18];
	Byte* const NR24 = &addressSpace[0xFF19];
	Byte* const NR30 = &addressSpace[0xFF1A];
	Byte* const NR31 = &addressSpace[0xFF1B];
	Byte* const NR32 = &addressSpace[0xFF1C];
	Byte* const NR33 = &addressSpace[0xFF1D];
	Byte* const NR34 = &addressSpace[0xFF1E];
	Byte* const NR40 = &addressSpace[0xFF1F]; //unused
	Byte* const NR41 = &addressSpace[0xFF20];
	Byte* const NR42 = &addressSpace[0xFF21];
	Byte* const NR43 = &addressSpace[0xFF22];
	Byte* const NR44 = &addressSpace[0xFF23];
	Byte* const NR50 = &addressSpace[0xFF24];
	Byte* const NR51 = &addressSpace[0xFF25];
	Byte* const NR52 = &addressSpace[0xFF26];
	Byte* const waveRam = &addressSpace[0xFF30]; //WaveRam[0x10]

	//PPU registers
	Byte* const LCDC = &addressSpace[0xFF40];
	Byte* const STAT = &addressSpace[0xFF41];
	Byte* const SCY = &addressSpace[0xFF42];
	Byte* const SCX = &addressSpace[0xFF43];
	Byte* const LY = &addressSpace[0xFF44];
	Byte* const LYC = &addressSpace[0xFF45];
	Byte* const DMA = &addressSpace[0xFF46];
	Byte* const BGP = &addressSpace[0xFF47];
	Byte* const OBP0 = &addressSpace[0xFF48];
	Byte* const OBP1 = &addressSpace[0xFF49];
	Byte* const WY = &addressSpace[0xFF4A];
	Byte* const WX = &addressSpace[0xFF4B];

	PPUMode currentMode = PPUMode::mode0;

	//3 colour channels
	uint32_t* framebuffer = new uint32_t[RESOLUTION_X * RESOLUTION_Y * SCREEN_BPP];
	SDL_Window* screen = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	SDL_Event event = {0};
	uint32_t frameStart = 0;
	uint32_t frameTime = 0;
	const int frameDelay = 1000 / V_SYNC;

	void opcodeResolver();
	void incLY();
	void ppuUpdate();
	void drawLine();
	void SDL2present();

	void checkPPUMode();
	void setPPUMode(PPUMode mode);
	uint64_t cyclesSinceLastScanline() const;
	uint64_t cyclesSinceLastRefresh() const;

	void timingHandler();

	void interruptHandler();
	bool testInterruptEnabled(Byte interrupt) const;
	void resetInterrupt(Byte interrupt) const;

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
	void ldW(Byte& dest, Word src);
	template <typename T>
	void orBitwise(T& dest, T src);
	template <typename T>
	void andBitwise(T& dest, T src);
	template <typename T>
	void xorBitwise(T& dest, T src);
	void bit(Byte testBit, Byte reg);
	void extendedOpcodeResolver();
	static void set(const uint8_t testBit, uint8_t& reg);
	static void res(const uint8_t testBit, uint8_t& reg);
	template <typename T>
	void jp(T address);
	template <typename T>
	bool jrNZ(T offset);
	template <class T>
	bool jrNC(T offset);
	template <class T>
	bool jrC(T offset);
	template <typename T>
	void inc(T& reg);
	template <typename T>
	void call(T address);
	void halt();
	void daa();
	void stop();
	template <typename T>
	void cp(T value);
	template <typename T>
	void dec(T& reg);
	template <typename T>
	bool jrZ(T offset);
	template <typename T>
	void sub(T value);
	template <class T>
	void sbc(T value);
	template <typename T>
	void jr(T OFFSET);
	template <typename T>
	void push(T reg);
	void rl(Byte& reg);
	void sla(Byte& reg);
	void sra(uint8_t& reg);
	void srl(uint8_t& reg);
	void rrc(Byte& reg);
	void rrca();
	void rra();
	void rr(Byte& reg);
	void rlc(Byte& reg);
	void rlca();
	void rla();
	template <typename T>
	void pop(T& reg);
	template <typename T>
	void rst(T address);
	void ret();
	template <typename T>
	void add(T& reg, T value);
	template <class T>
	void adc(T& reg, T value);
	void cpl();
	void scf();
	void ccf();
	void swap(Byte& value);

public:
	void start(std::string bootrom, std::string game);
	void SDL2setup();
	void SDL2destroy() const;
};

#endif //GBPP_SRC_GAMEBOY_HPP_
