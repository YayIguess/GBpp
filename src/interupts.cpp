#include "defines.hpp"
#include "gameboy.hpp"

bool GameBoy::testInterruptEnabled(Byte interrupt)
{
	return (*IE) & (Byte) (1 << interrupt);
}

void GameBoy::resetInterrupt(Byte interrupt)
{
	*IF &= ~(1 << interrupt);
}

void GameBoy::interruptHandler()
{
	if(!IME)
		return;

	if(*IF & (Byte) (1 << VBLANK_INTERRUPT) && testInterruptEnabled(VBLANK_INTERRUPT))
		VBlankHandle();
	if(*IF & (Byte) (1 << LCD_STAT_INTERRUPT) && testInterruptEnabled(LCD_STAT_INTERRUPT))
		LCDStatHandle();
	if(*IF & (Byte) (1 << TIMER_INTERRUPT) && testInterruptEnabled(TIMER_INTERRUPT))
		timerHandle();
	if(*IF & (Byte) (1 << SERIAL_INTERRUPT)	&& testInterruptEnabled(SERIAL_INTERRUPT))
		serialHandle();
	if(*IF & (Byte) (1 << JOYPAD_INTERRUPT)	&& testInterruptEnabled(JOYPAD_INTERRUPT))
		joypadHandle();
}

void GameBoy::VBlankHandle()
{
	printf("VBlank interrupt");
	IME = 0;
	push(PC);
	PC = 0x40;
	resetInterrupt(VBLANK_INTERRUPT);
}

void GameBoy::LCDStatHandle()
{
	printf("LCD stat interrupt");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x48;
	resetInterrupt(LCD_STAT_INTERRUPT);
}

void GameBoy::timerHandle()
{
	printf("timer interrupt");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x50;
	resetInterrupt(TIMER_INTERRUPT);
}

void GameBoy::serialHandle()
{
	printf("serial interrupt");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x58;
	resetInterrupt(SERIAL_INTERRUPT);
}

void GameBoy::joypadHandle()
{
	printf("joypad interrupt");
	IME = 0;
	push(PC);
	addCycles(16);
	PC = 0x60;
	resetInterrupt(JOYPAD_INTERRUPT);
}