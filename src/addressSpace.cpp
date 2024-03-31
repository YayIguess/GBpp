#include "addressSpace.hpp"
#include <iostream>

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
