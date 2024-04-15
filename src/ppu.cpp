#include "gameboy.hpp"
#include "defines.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>

bool GameBoy::LCDCBitEnabled(const Byte bit) const {
	return readOnlyAddressSpace.memoryLayout.LCDC & static_cast<Byte>(1 << bit);
}

bool GameBoy::oamBitEnabled(const Byte oamAttributeByte, const Byte bit) {
	return oamAttributeByte & static_cast<Byte>(1 << bit);
}

unsigned int GameBoy::getColourFromPalette(const Byte palette) {
	switch (palette & 0x3) {
	case 0:
		return WHITE;
	case 1:
		return LIGHT_GRAY;
	case 2:
		return DARK_GRAY;
	case 3:
		return BLACK;
	default:
		std::unreachable();
	}
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
	case mode0:
		addressSpace.memoryLayout.STAT &= ~0x03;
		currentMode = mode0;
		break;
	case mode1:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x01;
		currentMode = mode1;
		break;
	case mode2:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x02;
		currentMode = mode2;
		break;
	case mode3:
		addressSpace.memoryLayout.STAT &= ~0x03;
		addressSpace.memoryLayout.STAT |= 0x03;
		currentMode = mode3;
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


	const uint16_t backgroundMapAddr = LCDCBitEnabled(BG_TILE_MAP_AREA) ? 0x9C00 : 0x9800;
	const uint16_t windowMapAddr = LCDCBitEnabled(WINDOW_TILE_MAP_AREA) ? 0x9C00 : 0x9800;
	const uint16_t tileDataTableAddr = LCDCBitEnabled(BG_WINDOW_TILE_DATA_AREA) ? 0x8000 : 0x8800;
	const bool signedIndex = !LCDCBitEnabled(BG_WINDOW_TILE_DATA_AREA);

	//BG
	if (LCDCBitEnabled(BG_WINDOW_ENABLE)) {
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

			currentLinePixels[pixel] = getColourFromPalette(palette);
		}

		// 	For the window to be displayed on a scanline, the following conditions must be met:
		// WY condition was triggered: i.e. at some point in this frame the value of WY was equal to LY (checked at the start of Mode 2 only)
		// WX condition was triggered: i.e. the current X coordinate being rendered + 7 was equal to WX
		// Window enable bit in LCDC is set
		//Window
		const uint8_t windowY = readOnlyAddressSpace.memoryLayout.WY;
		const int16_t windowX = static_cast<int16_t>(readOnlyAddressSpace.memoryLayout.WX - 7);
		if (LCDCBitEnabled(WINDOW_ENABLE) && windowX >= 0 && windowX < RESOLUTION_X && line >= windowY) {
			for (int pixel = windowX; pixel < RESOLUTION_X; pixel++) {
				const uint16_t yIndex = windowLineCounter;
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

				currentLinePixels[pixel] = getColourFromPalette(palette);
			}
			windowLineCounter += 1;
		}
	}
	// oam/sprites
	if (LCDCBitEnabled(OBJ_ENABLE)) {
		uint32_t oamPixels[RESOLUTION_X];
		std::fill_n(oamPixels, RESOLUTION_X, 0);

		constexpr
			Word oamAddrStart = 0xFE00;
		const int spriteHeight = LCDCBitEnabled(OBJ_SIZE) ? 16 : 8;

		int objects[10] = {0};
		int found = 0;

		//oam hold 40 objects
		//find first 10
		for (int i = 0; i < 40 && found < 10; i++) {
			const int offset = i * 4;
			const int yPos = readOnlyAddressSpace[oamAddrStart + offset] - 16;

			if (line >= yPos && line <= yPos + spriteHeight - 1) {
				objects[found] = oamAddrStart + offset;
				found += 1;
			}
		}

		//sort by xPos (lower has higher priority when rendering) and then earlier objects
		for (int i = 0; i < found; i++) {
			for (int j = 0; j < found - i - 1; j++) {
				const int xPos1 = readOnlyAddressSpace[objects[j] + 1];
				const int xPos2 = readOnlyAddressSpace[objects[j + 1] + 1];

				if (xPos1 > xPos2) {
					const int tmp = objects[j];
					objects[j] = objects[j + 1];
					objects[j + 1] = tmp;
				}
			}
		}


		for (int objectIndex = found - 1; objectIndex >= 0; objectIndex--) {
			const int yPos = readOnlyAddressSpace[objects[objectIndex]] - 16;
			const int xPos = readOnlyAddressSpace[objects[objectIndex] + 1] - 8;
			const int tileIndex = readOnlyAddressSpace[objects[objectIndex] + 2];
			const Byte attributes = readOnlyAddressSpace[objects[objectIndex] + 3];

			const bool priority = oamBitEnabled(attributes, PRIORITY);
			const bool yFlip = oamBitEnabled(attributes, Y_FLIP);
			const bool xFlip = oamBitEnabled(attributes, X_FLIP);
			//get obp0 or obj1
			const Byte objPalette = oamBitEnabled(attributes, OBJ_PALETTE)
				                        ? addressSpace.memoryLayout.OBP1
				                        : addressSpace.memoryLayout.OBP0;

			for (int pixel = xPos; pixel < RESOLUTION_X && pixel < xPos + 8; pixel++) {
				constexpr
					Word objectTileAddr = 0x8000;
				if (pixel < 0)
					continue;

				const uint32_t colour = currentLinePixels[pixel];
				const Byte BGP = readOnlyAddressSpace.memoryLayout.BGP;
				if (priority && (colour == getColourFromPalette((BGP >> 2) & 0x3) ||
					colour == getColourFromPalette((BGP >> 4) & 0x3) ||
					colour == getColourFromPalette((BGP >> 6) & 0x3)
				)) {
					oamPixels[pixel] = colour;
					continue;
				}

				Byte objectX = pixel - xPos;
				Byte objectY = line - yPos;
				if (xFlip)
					objectX = 7 - objectX;
				if (yFlip)
					objectY = (spriteHeight - 1) - objectY;

				const Word objTileDataAddr = spriteHeight == 8
					                             ? objectTileAddr + (tileIndex * 16)
					                             : objectTileAddr + ((tileIndex & 0xFE) * 16);

				const Byte tileRow = objectY * 2;
				const Byte tileRowData1 = readOnlyAddressSpace[objTileDataAddr + tileRow];
				const Byte tileRowData2 = readOnlyAddressSpace[objTileDataAddr + tileRow + 1];

				const int bit = 7 - objectX;
				const int colorIndex = ((tileRowData2 >> bit) & 1) << 1 | ((tileRowData1 >> bit) & 1);

				// 0 is always transparent
				if (colorIndex != 0) {
					const uint8_t paletteColor = (objPalette >> (colorIndex * 2)) & 0x3;
					const uint32_t finalColor = getColourFromPalette(paletteColor);
					oamPixels[pixel] = finalColor;
				}
				else if (oamPixels[pixel] == 0) {
					oamPixels[pixel] = currentLinePixels[pixel];
				}
			}
		}

		//help ensure correct interaction with "BG OVER OBJ" flag
		for (int i = 0; i < RESOLUTION_X; i++) {
			if (oamPixels[i] != currentLinePixels[i] && oamPixels[i] != 0)
				currentLinePixels[i] = oamPixels[i];
		}
	}
}

void GameBoy::SDL2setup() {
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_CreateWindow("GameBoy++",
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
