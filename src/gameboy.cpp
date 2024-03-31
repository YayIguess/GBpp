#include <iostream>
#include "gameboy.hpp"

void GameBoy::addCycles(const uint8_t ticks) {
	cycles += ticks;
	if (ppuEnabled) {
		ppuCycles += ticks;
	}
	lastOpTicks = ticks;
}

void GameBoy::start(std::string bootrom, std::string game) {
	addressSpace.loadBootrom(bootrom);
	addressSpace.loadGame(game);
	addressSpace.determineMBCInfo();

	//init some registers that won't otherwise by set
	(*JOYP) = 0xCF;
	(*SC) = 0x7E;

	bool quit = false;

	bool display = false;

	while (!quit) {
		// Event loop
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		while (!rendered) {
			if (PC > 0xFF && addressSpace.getBootromState()) {
				addressSpace.unmapBootrom();
			}
			ppuEnabled = (*LCDC) & 0x80;

			if (PC >= 0xe0)
				display = true;
			if (display) {
				printf("Cycles: %lu, Opcode: 0x%.2x PPU cycles: %lu, PPMode: %d\n", cycles, addressSpace[PC],
				       cyclesSinceLastScanline(), currentMode);
				printf("PC:0x%.2x, SP:0x%.2x\n", PC, SP);
				printf("AF:0x%.4x, BC:0x%.4x\n", AF.reg, BC.reg);
				printf("DE:0x%.4x, HL:0x%.4x\n", DE.reg, HL.reg);
				printf("IME:%d IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
				printf("LCDC:%.2x STAT:0x%.2x LY:%d LYC:%d\n", (*LCDC), (*STAT), (*LY), (*LYC));
				printf("Cart type: 0x%.2x\n", addressSpace.game[0x147]);
				printf("\n");
			}

			opcodeResolver();
			interruptHandler();
			timingHandler();
			if (ppuEnabled) {
				ppuUpdate();
			}
			else {
				ppuCycles = 2;
				lastScanline = 0;
				lastRefresh = 0;
				(*LY) = 0x00;
				(*STAT) &= 0xfc;
			}
		}
		rendered = false;
	}
}
