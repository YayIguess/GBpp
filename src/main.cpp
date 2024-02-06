#include <string>
#include "gameboy.hpp"

int main(int argc, char** argv) {
	auto* gb = new GameBoy();
	gb->SDL2setup();
	gb->start("../bootrom.bin", "../roms/DrMario.gb");
	gb->SDL2destroy();
	delete gb;
	return 0;
}
