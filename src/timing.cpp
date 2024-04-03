#include "gameboy.hpp"

//handles most of the behavoir as described here: https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff04--div-divider-register
void GameBoy::timingHandler() {
	if (cycles - lastDivUpdate >= DIVIDER_REGISTER_FREQ) {
		const uint8_t increments = (cycles - lastDivUpdate) / DIVIDER_REGISTER_FREQ;
		addressSpace.memoryLayout.DIV += increments;
		lastDivUpdate += increments * DIVIDER_REGISTER_FREQ;
	}
}
