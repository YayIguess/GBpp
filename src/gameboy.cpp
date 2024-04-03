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
	addressSpace.createRamBank();

	//init some registers that won't otherwise by set
	addressSpace.memoryLayout.JOYP = 0xCF;
	addressSpace.memoryLayout.SC = 0x7E;

	bool quit = false;

	bool display = false;

	while (!quit) {
		// Event loop
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			if (event.type == SDL_KEYUP) {
				display = true;
			}
		}

		while (!rendered) {
			if (PC > 0xFF && addressSpace.getBootromState()) {
				addressSpace.unmapBootrom();
			}
			ppuEnabled = addressSpace.memoryLayout.LCDC & 0x80;

			// if (PC == 0x100)
			// 	display = true;
			// if (display) {
			// 	printf("Cycles: %lu, Opcode: 0x%.2x PPU cycles: %lu, PPMode: %d\n", cycles, readOnlyAddressSpace[PC],
			// 	       cyclesSinceLastScanline(), currentMode);
			// 	printf("AF:0x%.4x, BC:0x%.4x\n", AF.reg, BC.reg);
			// 	printf("DE:0x%.4x, HL:0x%.4x\n", DE.reg, HL.reg);
			// 	printf("IME:%d IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
			// 	printf("PC:0x%.4x, SP:0x%.4x\n", PC, SP);
			// 	printf("LCDC:%.2x STAT:0x%.2x LY:%d LYC:%d\n", (*LCDC), (*STAT), (*LY), (*LYC));
			// 	printf("\n");
			// }
			// if (PC >= 0xf000)
			// 	exit(1);

			opcodeResolver();
			addressSpace.MBCUpdate();
			interruptHandler();
			timingHandler();
			if (ppuEnabled) {
				ppuUpdate();
			}
			else {
				ppuCycles = 2;
				lastScanline = 0;
				lastRefresh = 0;
				addressSpace.memoryLayout.LY = 0x00;
				addressSpace.memoryLayout.STAT &= 0xfc;
			}
		}
		rendered = false;
	}
}
