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


void GameBoy::start(const std::string& bootrom, const std::string& game) {
	addressSpace.loadBootrom(bootrom);
	addressSpace.loadGame(game);
	addressSpace.determineMBCInfo();
	addressSpace.createRamBank();

	bool quit = false;
	bool setIME = false;
	bool debug = false;
	bool step = false;

	while (!quit) {
		// Event loop
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_a:
					joypadInput.LEFT = true;
					break;
				case SDLK_d:
					joypadInput.RIGHT = true;
					break;
				case SDLK_w:
					joypadInput.UP = true;
					break;
				case SDLK_s:
					joypadInput.DOWN = true;
					break;
				case SDLK_k:
					joypadInput.A = true;
					break;
				case SDLK_l:
					joypadInput.B = true;
					break;
				case SDLK_o:
					joypadInput.SELECT = true;
					break;
				case SDLK_p:
					joypadInput.START = true;
					break;
				case SDLK_h:
					debug = !debug;
					break;
				case SDLK_n:
					step = true;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
				case SDLK_a:
					joypadInput.LEFT = false;
					break;
				case SDLK_d:
					joypadInput.RIGHT = false;
					break;
				case SDLK_w:
					joypadInput.UP = false;
					break;
				case SDLK_s:
					joypadInput.DOWN = false;
					break;
				case SDLK_k:
					joypadInput.A = false;
					break;
				case SDLK_l:
					joypadInput.B = false;
					break;
				case SDLK_o:
					joypadInput.SELECT = false;
					break;
				case SDLK_p:
					joypadInput.START = false;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		while (!rendered) {
			if (debug == true && step == false)
				break;
			step = false;
			joypadHandler();
			if (PC > 0xFF && addressSpace.getBootromState()) {
				addressSpace.unmapBootrom();
			}
			ppuEnabled = addressSpace.memoryLayout.LCDC & 0x80;
			prevTMA = addressSpace.memoryLayout.TMA;

			if (debug) {
				printf(
					"A: %.2X F: %.2X B: %.2X C: %.2X D: %.2X E: %.2X H: %.2X L: %.2X SP: %.4X PC: 00:%.4X (%.2X %.2X %.2X %.2X)\n",
					AF.hi, AF.lo, BC.hi, BC.lo, DE.hi, DE.lo, HL.hi, HL.lo, SP, PC, readOnlyAddressSpace[PC],
					readOnlyAddressSpace[PC + 1], readOnlyAddressSpace[PC + 2], readOnlyAddressSpace[PC + 3]);


				// printf("Cycles: %lu, Opcode: 0x%.2x PPU cycles: %lu, PPMode: %d\n", cycles, readOnlyAddressSpace[PC],
				//        cyclesSinceLastScanline(), currentMode);
				// printf("AF:0x%.4x, BC:0x%.4x\n", AF.reg, BC.reg);
				// printf("DE:0x%.4x, HL:0x%.4x\n", DE.reg, HL.reg);
				// printf("IME:%d IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
				// printf("PC:0x%.4x, SP:0x%.4x\n", PC, SP);
				// printf("LCDC:%.2x STAT:0x%.2x LY:%d LYC:%d\n", (*LCDC), (*STAT), (*LY), (*LYC));
				// printf("\n");
			}

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
			if (addressSpace.dmaTransferRequested) {
				cyclesUntilDMATransfer -= lastOpTicks;
				if (cyclesUntilDMATransfer <= 0) {
					cyclesUntilDMATransfer = 160;
					addressSpace.dmaTransfer();
				}
			}
		}
		rendered = false;
	}
}
