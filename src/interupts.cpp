#include "defines.hpp"
#include "gameboy.hpp"

bool GameBoy::testInterruptEnabled(const Byte interrupt) const {
	return readOnlyAddressSpace.memoryLayout.IE & static_cast<Byte>(1 << interrupt);
}

void GameBoy::setInterrupt(const Byte interrupt) {
	addressSpace.memoryLayout.IF |= 1 << interrupt;
	addressSpace.memoryLayout.IF |= 0xE0;
}

void GameBoy::resetInterrupt(const Byte interrupt) {
	addressSpace.memoryLayout.IF &= ~(1 << interrupt);
	addressSpace.memoryLayout.IF |= 0xE0;
}

void GameBoy::interruptHandler() {
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << VBLANK_INTERRUPT) && testInterruptEnabled(
		VBLANK_INTERRUPT)) {
		if (IME)
			VBlankHandle();
		halted = false;
	}
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << LCD_STAT_INTERRUPT) && testInterruptEnabled(
		LCD_STAT_INTERRUPT)) {
		if (IME)
			LCDStatHandle();
		halted = false;
	}
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << TIMER_INTERRUPT) && testInterruptEnabled(
		TIMER_INTERRUPT)) {
		if (IME)
			timerHandle();
		halted = false;
	}
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << SERIAL_INTERRUPT) && testInterruptEnabled(
		SERIAL_INTERRUPT)) {
		if (IME)
			serialHandle();
		halted = false;
	}
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << JOYPAD_INTERRUPT) && testInterruptEnabled(
		JOYPAD_INTERRUPT)) {
		if (IME)
			joypadHandle();
		halted = false;
	}
}

void GameBoy::VBlankHandle() {
	IME = 0;
	push(PC);
	addCycles(20);
	PC = 0x40;
	resetInterrupt(VBLANK_INTERRUPT);
}

void GameBoy::LCDStatHandle() {
	IME = 0;
	push(PC);
	addCycles(20);
	PC = 0x48;
	resetInterrupt(LCD_STAT_INTERRUPT);
}

void GameBoy::timerHandle() {
	IME = 0;
	push(PC);
	addCycles(20);
	PC = 0x50;
	resetInterrupt(TIMER_INTERRUPT);
}

void GameBoy::serialHandle() {
	IME = 0;
	push(PC);
	addCycles(20);
	PC = 0x58;
	resetInterrupt(SERIAL_INTERRUPT);
}

void GameBoy::joypadHandle() {
	IME = 0;
	push(PC);
	addCycles(20);
	PC = 0x60;
	resetInterrupt(JOYPAD_INTERRUPT);
}
