#include "addressSpace.hpp"
#include <iostream>
#include <iterator>

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
	std::ifstream rom(filename, std::ios::binary);
	rom.unsetf(std::ios::skipws);

	if (!rom.is_open()) {
		std::cerr << "Game was not found!\nQuitting!\n" << std::endl;
		exit(1);
	}

	rom.seekg(0, std::ios::end);
	const std::streampos rom_size = rom.tellg();
	rom.seekg(0, std::ios::beg);

	game.reserve(rom_size);
	game.insert(game.begin(),
	            std::istream_iterator<Byte>(rom),
	            std::istream_iterator<Byte>());

	memoryLayout.romBank0 = game.data();
	memoryLayout.romBankSwitch = game.data() + ROM_BANK_SIZE;
}
