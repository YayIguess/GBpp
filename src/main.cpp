#include <string>
#include "gameboy.hpp"

int main(int argc, char** argv) {
	auto* gb = new GameBoy();
	gb->SDL2setup();
	gb->start("../dmg_boot.bin", "../roms/cpu_instrs.gb");
	gb->SDL2destroy();
	delete gb;

	return 0;
}
