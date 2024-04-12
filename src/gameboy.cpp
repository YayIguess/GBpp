#include <iostream>
#include "gameboy.hpp"

void GameBoy::addCycles(const uint8_t ticks) {
	cycles += ticks;
	if (ppuEnabled) {
		ppuCycles += ticks;
	}
	lastOpTicks = ticks;
}

GameboyTestState GameBoy::runTest(GameboyTestState initial) {
	addressSpace.setTesting(true);

	PC = initial.PC;
	SP = initial.SP;
	AF.hi = initial.A;
	AF.lo = initial.F;
	BC.hi = initial.B;
	BC.lo = initial.C;
	DE.hi = initial.D;
	DE.lo = initial.E;
	HL.hi = initial.H;
	HL.lo = initial.L;
	addressSpace.memoryLayout.IE = 1;

	for (const auto& [addr, val] : initial.RAM) {
		addressSpace[addr] = val;
	}

	opcodeResolver();

	std::vector<std::tuple<Word, Byte>> returnRAM;
	for (const auto& [addr, val] : initial.RAM) {
		returnRAM.emplace_back(addr, addressSpace[addr]);
	}
	return {
		PC, SP,
		AF.hi, AF.lo,
		BC.hi, BC.lo,
		DE.hi, DE.lo,
		HL.hi, HL.lo,
		returnRAM
	};
}


void GameBoy::start(std::string bootrom, std::string game) {
	addressSpace.loadBootrom(bootrom);
	addressSpace.loadGame(game);
	addressSpace.determineMBCInfo();
	addressSpace.createRamBank();

	//init some registers that won't otherwise by set
	addressSpace.memoryLayout.JOYP = 0xDF;
	addressSpace.memoryLayout.SC = 0x7E;

	bool quit = false;
	bool setIME = false;
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
			prevTMA = addressSpace.memoryLayout.TMA;

			if (PC == 0x100)
				display = true;
			if (display) {
				// printf("A: %.2X F: %.2X B: %.2X C: %.2X D: %.2X E: %.2X H: %.2X L: %.2X SP: %.4X PC: 00:%.4X (%.2X %.2X %.2X %.2X)\n",
				// AF.hi, AF.lo, BC.hi, BC.lo, DE.hi, DE.lo, HL.hi, HL.lo, SP, PC, readOnlyAddressSpace[PC],
				// readOnlyAddressSpace[PC + 1], readOnlyAddressSpace[PC + 2], readOnlyAddressSpace[PC + 3]);


				// 	printf("Cycles: %lu, Opcode: 0x%.2x PPU cycles: %lu, PPMode: %d\n", cycles, readOnlyAddressSpace[PC],
				// 	       cyclesSinceLastScanline(), currentMode);
				// 	printf("AF:0x%.4x, BC:0x%.4x\n", AF.reg, BC.reg);
				// 	printf("DE:0x%.4x, HL:0x%.4x\n", DE.reg, HL.reg);
				// 	printf("IME:%d IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
				// 	printf("PC:0x%.4x, SP:0x%.4x\n", PC, SP);
				// 	printf("LCDC:%.2x STAT:0x%.2x LY:%d LYC:%d\n", (*LCDC), (*STAT), (*LY), (*LYC));
				// 	printf("\n");
			}
			// if (PC >= 0xf000)
			// 	exit(1);


			if (!halted) {
				opcodeResolver();
				addressSpace.MBCUpdate();
			}
			else {
				addCycles(4);
			}
			timingHandler();
			interruptHandler();
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
			if (setIME) {
				IME = 1;
				setIME = false;
			}
			if (IME_togge) {
				setIME = true;
				IME_togge = false;
			}
		}
		rendered = false;
	}
}
