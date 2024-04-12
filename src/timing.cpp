#include "gameboy.hpp"

//handles most of the behavoir as described here: https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff04--div-divider-register
void GameBoy::timingHandler() {
	//can't do this as we use cycles for PPU timing but this is what should happen
	//addressSpace.memoryLayout.DIV = ((cycles / 4) >> 6) & 0xFF;

	if (cycles - lastDivUpdate >= DIVIDER_REGISTER_FREQ) {
		const uint8_t increments = (cycles - lastDivUpdate) / DIVIDER_REGISTER_FREQ;
		addressSpace.memoryLayout.DIV += increments;
		lastDivUpdate += increments * DIVIDER_REGISTER_FREQ;
	}

	//if enabled
	uint64_t TIMAFrequency = 0;
	if (addressSpace.memoryLayout.TAC & 0x04) {
		switch (addressSpace.memoryLayout.TAC & 0x03) {
		case 0:
			TIMAFrequency = 1024;
			break;
		case 1:
			TIMAFrequency = 16;
			break;
		case 2:
			TIMAFrequency = 64;
			break;
		case 3:
			TIMAFrequency = 256;
			break;
		}
		//if TIMA overflowed and prevTMA != current TMA, use prevTMA (ie use prevTMA regardless)
		const int increments = (cycles - lastTIMAUpdate) / TIMAFrequency;
		if (cycles - lastTIMAUpdate >= TIMAFrequency) {
			if (static_cast<int>(addressSpace.memoryLayout.TIMA) + increments > 255) {
				addressSpace.memoryLayout.TIMA = prevTMA + ((addressSpace.memoryLayout.TIMA + increments) % 256);
				setInterrupt(TIMER_INTERRUPT);
			}
			else
				addressSpace.memoryLayout.TIMA += increments;
			lastTIMAUpdate += increments * TIMAFrequency;
		}
	}
}
