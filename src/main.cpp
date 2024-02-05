#include <iostream>
#include <filesystem>
#include <string>
#include <optional>
#include "gameboy.hpp"
#include "defines.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
	auto* gb = new GameBoy();
	gb->SDL2setup();
	gb->start("/home/braiden/Code/GBpp/bootrom.bin", "/home/braiden/Code/GBpp/roms/DrMario.gb");
	gb->SDL2destroy();
	delete gb;
	return 0;
}
