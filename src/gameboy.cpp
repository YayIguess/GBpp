#include <iostream>
#include "gameboy.hpp"

bool AddressSpace::getBootromState() {
	return bootromLoaded;
}

void AddressSpace::unmapBootrom() {
	bootromLoaded = false;
}

void AddressSpace::mapBootrom() {
	bootromLoaded = true;
}

void AddressSpace::loadBootrom(std::string filename) {
	std::ifstream file;
	int size = std::filesystem::file_size(filename);
	if (size != 256) {
		std::cerr << "Bootrom was an unexpected size!\nQuitting!\n" << std::endl;
		exit(1);
	}
	file.open(filename, std::ios::binary);
	file.read(reinterpret_cast<char*>(bootrom), BOOTROM_SIZE);
}

void AddressSpace::loadGame(std::string filename) {
	game.open(filename, std::ios::binary);

	if (!game.is_open()) {
		std::cerr << "Game was not found!\nQuitting!\n" << std::endl;
		exit(1);
	}
	game.read(reinterpret_cast<char*>(memoryLayout.romBank1), ROM_BANK_SIZE * 2);
}

void GameBoy::addCycles(uint8_t ticks) {
	cycles = (cycles + ticks) % T_CLOCK_FREQ;
	lastOpTicks = ticks;
}

void GameBoy::start(std::string bootrom, std::string game) {
	addressSpace.loadBootrom(bootrom);
	addressSpace.loadGame(game);

	bool quit = false;
	while (!quit) {
		// Event loop: Check and handle SDL events
		// if(SDL_PollEvent(&event))
		// {
		// 	if(event.type == SDL_QUIT)
		// 	{
		// 		quit = true; // Set the quit flag when the close button is hit
		// 	}
		// }

		opcodeHandler();
		interruptHandler();
		//timing();
		ppuUpdate();
		if (PC > 0xFF && addressSpace.getBootromState()) {
			addressSpace.unmapBootrom();
		}
		int cyclesSince = cyclesSinceLastRefresh();
		if (cyclesSince > FRAME_DURATION) {
			lastRefresh = cycles;
			SDL2present();
		}
	}
}
