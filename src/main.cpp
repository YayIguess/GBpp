#include <string>
#include "gameboy.hpp"

int main(int argc, char** argv) {
	auto* gb = new GameBoy();
	gb->SDL2setup();
	gb->start("../dmg_boot.bin", "../roms/03-op_sp,hl.gb");
	gb->SDL2destroy();
	delete gb;

	return 0;
}
