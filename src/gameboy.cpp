#include <iostream>
#include "gameboy.hpp"

bool AddressSpace::getBootromState() const {
	return bootromLoaded;
}

void AddressSpace::unmapBootrom() {
	bootromLoaded = false;
}

void AddressSpace::mapBootrom() {
	bootromLoaded = true;
}

void AddressSpace::loadBootrom(const std::string& filename) {
	std::ifstream file;
	if (const uintmax_t size = std::filesystem::file_size(filename); size != 256) {
		std::cerr << "Bootrom was an unexpected size!\nQuitting!\n" << std::endl;
		exit(1);
	}
	file.open(filename, std::ios::binary);
	file.read(reinterpret_cast<char*>(bootrom), BOOTROM_SIZE);
}

void AddressSpace::loadGame(const std::string& filename) {
	game.open(filename, std::ios::binary);

	if (!game.is_open()) {
		std::cerr << "Game was not found!\nQuitting!\n" << std::endl;
		exit(1);
	}
	game.read(reinterpret_cast<char*>(memoryLayout.romBank1), ROM_BANK_SIZE * 2);
}

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

	//init some registers that won't otherwise by set
	(*JOYP) = 0xCF;
	(*SC) = 0x7E;

	bool quit = false;

	bool display = false;

	while (!quit) {
		// Event loop: Check and handle SDL events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true; // Set the quit flag when the close button is hit
			}
		}

		if (PC > 0xFF && addressSpace.getBootromState()) {
			addressSpace.unmapBootrom();
		}
		ppuEnabled = (*LCDC) & 0x80;

		// if (PC == 0x100)
		// 	display = true;
		// if (display) {
		// 	printf("Cycles: %lu, Opcode: 0x%.2x PPU cycles: %lu, PPMode: %d\n", cycles, addressSpace[PC],
		// 	       cyclesSinceLastScanline(), currentMode);
		// 	printf("PC:0x%.2x, SP:0x%.2x\n", PC, SP);
		// 	printf("AF:0x%.4x, BC:0x%.4x\n", AF.reg, BC.reg);
		// 	printf("DE:0x%.4x, HL:0x%.4x\n", DE.reg, HL.reg);
		// 	printf("IME:%d IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
		// 	printf("LCDC:%.2x STAT:0x%.2x LY:%d LYC:%d\n", (*LCDC), (*STAT), (*LY), (*LYC));
		// 	printf("\n");
		// }

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
}
