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
	const AddressSpace& readOnlyAddressSpace = addressSpace;

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
	void ldW(Byte& dest, Word src);
	template <typename T>
	void orBitwise(T& dest, T src);
	template <typename T>
	void andBitwise(T& dest, T src);
	template <typename T>
	void xorBitwise(T& dest, T src);
	void bit(Byte testBit, Byte reg);
	void extendedOpcodeResolver();
	static void set(uint8_t testBit, uint8_t& reg);
	static void res(uint8_t testBit, uint8_t& reg);
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
