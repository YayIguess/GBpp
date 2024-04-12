#include "gameboy.hpp"
#include "defines.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>

bool GameBoy::testLCDCBitEnabled(const Byte bit) const {
	return readOnlyAddressSpace.memoryLayout.LCDC & static_cast<Byte>(1 << bit);
}

void GameBoy::ppuUpdate() {
	//test for HBlank
	checkPPUMode();

	// TODO:
	// bug on DMG models triggers a STAT interrupt anytime the STAT register is written
	// Road Rage and Zerd no Denetsu rely on this

	// Check for STAT interrupts and request if needed (e.g., when entering specific modes)
	const bool hBlankInterruptEnabled = readOnlyAddressSpace.memoryLayout.STAT & (1 << 3);
	const bool drawingInterruptEnabled = readOnlyAddressSpace.memoryLayout.STAT & (1 << 4);
	const bool oamInterruptEnabled = readOnlyAddressSpace.memoryLayout.STAT & (1 << 5);
	const bool previousInterruptLine = statInteruptLine;

	if (currentMode == PPUMode::mode0 && hBlankInterruptEnabled ||
		currentMode == PPUMode::mode3 && drawingInterruptEnabled ||
		currentMode == PPUMode::mode2 && oamInterruptEnabled) {
		statInteruptLine = true;
	}
	else {
		statInteruptLine = false;
	}

	const bool lyLycInterruptEnabled = readOnlyAddressSpace.memoryLayout.STAT & (1 << 6);

	if (readOnlyAddressSpace.memoryLayout.LY == readOnlyAddressSpace.memoryLayout.LYC) {
		addressSpace.memoryLayout.STAT |= (1 << 2);
		if (lyLycInterruptEnabled)
			statInteruptLine = true;
	}
	else {
		addressSpace.memoryLayout.STAT &= ~(1 << 2);
	}
	if (statInteruptLine && !previousInterruptLine)
		addressSpace.memoryLayout.IF |= 1 << LCD_STAT_INTERRUPT;
}

void GameBoy::checkPPUMode() {
	// Check the PPU mode (HBlank, VBlank, OAM Search, or Pixel Transfer)
	const uint64_t cyclesSinceScanline = cyclesSinceLastScanline();

	switch (currentMode) {
	//hblank AND vblank
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
	addressSpace.memoryLayout.LY += 1;
	setPPUMode(PPUMode::mode2);
	if (addressSpace.memoryLayout.LY > SCANLINES_PER_FRAME - 1) {
		addressSpace.memoryLayout.LY = 0;
		windowLineCounter = 0;
	}
	else if (addressSpace.memoryLayout.LY == 144) {
		// VBlank Period
		SDL2present();
		setPPUMode(PPUMode::mode1);
		addressSpace.memoryLayout.IF |= 0x1;
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
		addressSpace.memoryLayout.STAT &= ~0x03;
		currentMode = PPUMode::mode0;
		break;
	case PPUMode::mode1:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x01;
		currentMode = PPUMode::mode1;
		break;
	case PPUMode::mode2:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x02;
		currentMode = PPUMode::mode2;
		break;
	case PPUMode::mode3:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x03;
		currentMode = PPUMode::mode3;
		break;
	}
	//7th bit is unused but always set
	addressSpace.memoryLayout.STAT |= 0x80;
}

void GameBoy::drawLine() {
	const uint8_t line = readOnlyAddressSpace.memoryLayout.LY;

	// Calculate the starting index of the current scanline in the framebuffer
	const uint32_t lineStartIndex = line * RESOLUTION_X;

	// Pointer to the current line's pixel data in the framebuffer
	uint32_t* currentLinePixels = framebuffer + lineStartIndex;
	std::fill_n(currentLinePixels, RESOLUTION_X, 0xFFFFFFFF);


	const uint16_t backgroundMapAddr = testLCDCBitEnabled(BG_TILE_MAP_AREA) ? 0x9C00 : 0x9800;
	const uint16_t windowMapAddr = testLCDCBitEnabled(WINDOW_TILE_MAP_AREA) ? 0x9C00 : 0x9800;
	const uint16_t tileDataTableAddr = testLCDCBitEnabled(BG_WINDOW_TILE_DATA_AREA) ? 0x8000 : 0x8800;
	const bool signedIndex = !testLCDCBitEnabled(BG_WINDOW_TILE_DATA_AREA);

	//BG
	if (testLCDCBitEnabled(BG_WINDOW_ENABLE)) {
		for (int pixel = 0; pixel < RESOLUTION_X; pixel++) {
			const uint16_t xIndex = (pixel + readOnlyAddressSpace.memoryLayout.SCX) % 256;
			// 256 pixels in total BG width
			const uint16_t yIndex = (line + readOnlyAddressSpace.memoryLayout.SCY) % 256;
			// 256 pixels in total BG height

			const uint16_t tileUpper = (yIndex / 8) << 5;
			const uint16_t tileLower = xIndex / 8 & 0x1F;
			const uint16_t tileIndex = tileUpper + tileLower;

			const uint16_t tileAddr = backgroundMapAddr + tileIndex;
			const int16_t tileID = signedIndex
				                       ? static_cast<int16_t>(readOnlyAddressSpace[tileAddr])
				                       : readOnlyAddressSpace[tileAddr];

			uint16_t tileDataAddr;
			if (signedIndex)
				tileDataAddr = tileDataTableAddr + (((tileID + 128) % 256) << 4); // For signed, wrap around
			else
				tileDataAddr = tileDataTableAddr + (tileID * 16);


			const uint8_t lineOffset = yIndex % 8;
			const uint8_t tileRowData1 = readOnlyAddressSpace[tileDataAddr + (lineOffset * 2)];
			const uint8_t tileRowData2 = readOnlyAddressSpace[tileDataAddr + (lineOffset * 2) + 1];

			//get pixel data (2 bits)
			const uint8_t colourBit = 7 - (xIndex % 8);
			const uint8_t colourNum = ((tileRowData2 >> colourBit) & 0x1) << 1 | ((tileRowData1 >> colourBit) & 0x1);

			// Apply the BGP register for palette mapping
			const uint8_t palette = (readOnlyAddressSpace.memoryLayout.BGP >> (colourNum * 2)) & 0x3;

			switch (palette) {
			case 0:
				currentLinePixels[pixel] = 0xFFFFFFFF;
				break; // White
			case 1:
				currentLinePixels[pixel] = 0xFFAAAAAA;
				break; // Light gray
			case 2:
				currentLinePixels[pixel] = 0xFF555555;
				break; // Dark gray
			case 3:
				currentLinePixels[pixel] = 0xFF000000;
				break; // Black
			default:
				break; // Default case for safety, should not be reached
			}
		}

		// 	For the window to be displayed on a scanline, the following conditions must be met:
		// WY condition was triggered: i.e. at some point in this frame the value of WY was equal to LY (checked at the start of Mode 2 only)
		// WX condition was triggered: i.e. the current X coordinate being rendered + 7 was equal to WX
		// Window enable bit in LCDC is set
		//Window
		const uint8_t windowY = readOnlyAddressSpace.memoryLayout.WY;
		const int16_t windowX = readOnlyAddressSpace.memoryLayout.WX - 7;
		if (testLCDCBitEnabled(WINDOW_ENABLE) && windowX >= 0 && line >= windowY) {
			for (int pixel = windowX; pixel < RESOLUTION_X; pixel++) {
				const uint16_t yIndex = line - windowY;
				const uint16_t windowTileUpper = (yIndex / 8) << 5;

				const uint16_t xIndex = pixel - windowX;
				const uint16_t windowTileLower = (xIndex / 8) & 0x1F;

				const uint16_t tileIndex = windowTileUpper + windowTileLower;
				const uint16_t tileAddr = windowMapAddr + tileIndex;

				const int16_t tileID = signedIndex
					                       ? static_cast<int16_t>(readOnlyAddressSpace[tileAddr])
					                       : readOnlyAddressSpace[tileAddr];

				uint16_t tileDataAddr;
				if (signedIndex)
					tileDataAddr = tileDataTableAddr + (((tileID + 128) % 256) * 16); // For signed, wrap around
				else
					tileDataAddr = tileDataTableAddr + (tileID * 16);


				const uint8_t lineOffset = yIndex % 8;
				const uint8_t tileRowData1 = readOnlyAddressSpace[tileDataAddr + (lineOffset * 2)];
				const uint8_t tileRowData2 = readOnlyAddressSpace[tileDataAddr + (lineOffset * 2) + 1];

				//get pixel data (2 bits)
				const uint8_t colourBit = 7 - (xIndex % 8);
				const uint8_t colourNum = ((tileRowData2 >> colourBit) & 0x1) << 1 | ((tileRowData1 >> colourBit) &
					0x1);

				// Apply the BGP register for palette mapping
				const uint8_t palette = (readOnlyAddressSpace.memoryLayout.BGP >> (colourNum * 2)) & 0x3;

				switch (palette) {
				case 0:
					currentLinePixels[pixel] = 0xFFFFFFFF;
					break; // White
				case 1:
					currentLinePixels[pixel] = 0xFFAAAAAA;
					break; // Light gray
				case 2:
					currentLinePixels[pixel] = 0xFF555555;
					break; // Dark gray
				case 3:
					currentLinePixels[pixel] = 0xFF000000;
					break; // Black
				default:
					break; // Default case for safety, should not be reached
				}
				windowLineCounter += 1;
			}
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

	if (frameDelay > frameTime) {
		SDL_Delay(frameDelay - frameTime);
	}

	SDL_RenderPresent(renderer);
	frameStart = SDL_GetTicks();
	rendered = true;
}
