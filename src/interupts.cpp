#include "defines.hpp"
#include "gameboy.hpp"

bool GameBoy::testInterruptEnabled(const Byte interrupt) const {
	return readOnlyAddressSpace.memoryLayout.IE & static_cast<Byte>(1 << interrupt);
}

void GameBoy::resetInterrupt(const Byte interrupt) {
	addressSpace.memoryLayout.IF &= ~(1 << interrupt);
	addressSpace.memoryLayout.IF |= 0xE0;
}

void GameBoy::interruptHandler() {
	if (!IME)
		return;

	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << VBLANK_INTERRUPT) && testInterruptEnabled(
		VBLANK_INTERRUPT))
		VBlankHandle();
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << LCD_STAT_INTERRUPT) && testInterruptEnabled(
		LCD_STAT_INTERRUPT))
		LCDStatHandle();
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << TIMER_INTERRUPT) && testInterruptEnabled(
		TIMER_INTERRUPT))
		timerHandle();
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << SERIAL_INTERRUPT) && testInterruptEnabled(
		SERIAL_INTERRUPT))
		serialHandle();
	if (readOnlyAddressSpace.memoryLayout.IF & static_cast<Byte>(1 << JOYPAD_INTERRUPT) && testInterruptEnabled(
		JOYPAD_INTERRUPT))
		joypadHandle();
}

void GameBoy::VBlankHandle() {
	//printf("VBlank interrupt\n");
	IME = 0;
	push(PC);
	PC = 0x40;
	resetInterrupt(VBLANK_INTERRUPT);
}

void GameBoy::LCDStatHandle() {
	//printf("LCD stat interrupt\n");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x48;
	resetInterrupt(LCD_STAT_INTERRUPT);
}

void GameBoy::timerHandle() {
	//printf("timer interrupt\n");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x50;
	resetInterrupt(TIMER_INTERRUPT);
}

void GameBoy::serialHandle() {
	//printf("serial interrupt\n");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x58;
	resetInterrupt(SERIAL_INTERRUPT);
}

void GameBoy::joypadHandle() {
	printf("joypad interrupt\n");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x60;
	resetInterrupt(JOYPAD_INTERRUPT);
}
