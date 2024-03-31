#include "gameboy.hpp"
#include "defines.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>

void GameBoy::ppuUpdate() {
	//test for HBlank
	checkPPUMode();

	if ((*LY) == (*LYC) || (*STAT) & (1 << 6)) {
		// Request STAT interrupt if LY matches LYC
		// bug on DMG models triggers a STAT interrupt anytime the STAT register is written
		// Road Rage and Zerd no Denetsu rely on this
		(*STAT) |= (1 << 2);
	}
	else {
		(*STAT) &= ~(1 << 2);
	}

	// Check for STAT interrupts and request if needed (e.g., when entering specific modes)
	bool hBlankInterruptEnabled = (*STAT) & (1 << 3);
	bool vBlankInterruptEnabled = (*STAT) & (1 << 4); /* Determine if VBlank interrupt is enabled */
	bool oamInterruptEnabled = (*STAT) & (1 << 5); /* Determine if OAM Search interrupt is enabled */

	if (currentMode == PPUMode::mode0 && hBlankInterruptEnabled ||
		currentMode == PPUMode::mode1 && vBlankInterruptEnabled ||
		currentMode == PPUMode::mode2 && oamInterruptEnabled) {
		*IF |= 0x2;
	}
}

void GameBoy::checkPPUMode() {
	// Check the PPU mode (HBlank, VBlank, OAM Search, or Pixel Transfer)
	const uint64_t cyclesSinceScanline = cyclesSinceLastScanline();

	switch (currentMode) {
	//hblank and vblank
	case 0:
	case 1:
		if (cyclesSinceScanline > SCANLINE_DURATION) {
			lastScanline = ppuCycles - (cyclesSinceScanline - SCANLINE_DURATION);
			incLY();
		}
		break;
	case 2:
		if (cyclesSinceScanline > MODE2_DURATION) {
			setPPUMode(PPUMode::mode3);
		}
		break;
	case 3:
		if (cyclesSinceScanline > MODE2_DURATION + MODE3_MIN_DURATION) {
			drawLine();
			setPPUMode(PPUMode::mode0);
		}
		break;
	}
}

void GameBoy::incLY() {
	(*LY)++;
	setPPUMode(PPUMode::mode2);
	if ((*LY) > SCANLINES_PER_FRAME - 1) {
		(*LY) = 0;
	}
	else if ((*LY) == 144) {
		// VBlank Period
		SDL2present();
		setPPUMode(PPUMode::mode1);
		*IF |= 0x1;
	}
}

uint64_t GameBoy::cyclesSinceLastScanline() const {
	const uint64_t difference = ppuCycles - lastScanline;
	return difference;
}

uint64_t GameBoy::cyclesSinceLastRefresh() const {
	const uint64_t difference = ppuCycles - lastRefresh;
	return difference;
}

void GameBoy::setPPUMode(const PPUMode mode) {
	switch (mode) {
	case PPUMode::mode0:
		(*STAT) &= ~0x03;
		currentMode = PPUMode::mode0;
		break;
	case PPUMode::mode1:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x01;
		currentMode = PPUMode::mode1;
		break;
	case PPUMode::mode2:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x02;
		currentMode = PPUMode::mode2;
		break;
	case PPUMode::mode3:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x03;
		currentMode = PPUMode::mode3;
		break;
	}
	//7th bit is unused but always set
	(*STAT) |= 0x80;
}

void GameBoy::drawLine() {
	const uint8_t line = (*LY);

	// Calculate the starting index of the current scanline in the framebuffer
	const uint32_t lineStartIndex = line * RESOLUTION_X;

	// Pointer to the current line's pixel data in the framebuffer
	uint32_t* currentLinePixels = framebuffer + lineStartIndex;

	if (!(*LCDC & 0x1)) {
		std::fill_n(currentLinePixels, RESOLUTION_X, 0xFFFFFFFF); // Fill line with white if BG display is off.
		return;
	}

	const uint16_t backgroundMapAddr = (*LCDC & 0x08) ? 0x9C00 : 0x9800;
	const uint16_t tileDataTableAddr = (*LCDC & 0x10) ? 0x8000 : 0x8800;
	const bool signedIndex = !(*LCDC & 0x10);

	for (int pixel = 0; pixel < RESOLUTION_X; pixel++) {
		const uint8_t xPos = (pixel + (*SCX)) % 256; // 256 pixels in total BG width
		const uint8_t yPos = (line + (*SCY)) % 256; // 256 pixels in total BG height

		const uint16_t tileRow = (yPos / 8) * 32;
		const uint16_t tileCol = xPos / 8;
		const uint16_t tileIndex = tileRow + tileCol;

		const uint16_t tileAddr = backgroundMapAddr + tileIndex;
		const int8_t tileID = signedIndex ? static_cast<int8_t>(addressSpace[tileAddr]) : addressSpace[tileAddr];

		uint16_t tileDataAddr;
		if (signedIndex) {
			tileDataAddr = tileDataTableAddr + (((tileID + 128) % 256) * 16); // For signed, wrap around
		}
		else {
			tileDataAddr = tileDataTableAddr + (tileID * 16);
		}

		const uint8_t lineOffset = yPos % 8;
		const uint8_t tileRowData1 = addressSpace[tileDataAddr + (lineOffset * 2)];
		const uint8_t tileRowData2 = addressSpace[tileDataAddr + (lineOffset * 2) + 1];

		const uint8_t colourBit = 7 - (xPos % 8);
		const uint8_t colourNum = ((tileRowData2 >> colourBit) & 0x1) << 1 | ((tileRowData1 >> colourBit) & 0x1);

		// Apply the BGP register for palette mapping
		uint8_t palette = (*BGP >> (colourNum * 2)) & 0x3;
		if (xPos == 0)
			palette = 0x3;
		switch (palette) {
		case 0: currentLinePixels[pixel] = 0xFFFFFFFF;
			break; // White
		case 1: currentLinePixels[pixel] = 0xAAAAAAFF;
			break; // Light gray
		case 2: currentLinePixels[pixel] = 0x555555FF;
			break; // Dark gray
		case 3: currentLinePixels[pixel] = 0x000000FF;
			break; // Black
		default: break; // Default case for safety, should not be reached
		}
	}
}

void GameBoy::SDL2setup() {
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_CreateWindow("GBpp",
	                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          RESOLUTION_X, RESOLUTION_Y,
	                          0);

	// Create an SDL renderer to draw on the window
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	// Create an SDL texture to hold the framebuffer data
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                            SDL_TEXTUREACCESS_STREAMING, RESOLUTION_X, RESOLUTION_Y);
}

void GameBoy::SDL2destroy() const {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	SDL_Quit();
}

void GameBoy::SDL2present() {
	SDL_UpdateTexture(texture, nullptr, framebuffer, RESOLUTION_X * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);


	frameTime = SDL_GetTicks() - frameStart;
	std::cout << SDL_GetTicks() << " " << frameTime << std::endl;

	if (frameDelay > frameTime) {
		SDL_Delay(frameDelay - frameTime);
	}

	SDL_RenderPresent(renderer);
	frameStart = SDL_GetTicks();
	rendered = true;
}
