#include "gameboy.hpp"
#include "defines.hpp"



void GameBoy::ppuUpdate()
{
	//test HBlank
	checkPPUMode();

	if(cyclesToStayInHblank != -1)
	{
		if (cyclesToStayInHblank < cyclesSinceLastScanline())
		{
			return;
		}
		if(cyclesToStayInHblank >= cyclesSinceLastScanline())
		{
			lastScanline = cycles;
			cyclesToStayInHblank = -1;
		}
	}

	// Check the PPU mode (HBlank, VBlank, OAM Search, or Pixel Transfer)
	Byte mode = (*STAT)&0x03;
	switch(mode)
	{
	case 0:
		if(cyclesSinceLastScanline() > MODE2_DURATION + MODE3_BASE_DURATION)
		{
			drawLine();
			cyclesToStayInHblank = SCANLINE_DURATION - cyclesSinceLastScanline();
			lastScanline = cycles;
			(*LY)++;
			if((*LY) > 153)
				(*LY) = 0;
		}
		currentMode = PPUMode::mode0;
		break;

	//vblank
	case 1:
		if(currentMode != PPUMode::mode1)
		{
			drawLine();
		}
		if(cyclesSinceLastScanline() > SCANLINE_DURATION)
		{
			lastScanline = cycles;
			(*LY)++;
			if((*LY) > 153)
				(*LY) = 0;
		}
		currentMode = PPUMode::mode1;
		break;
	case 2:
		currentMode = PPUMode::mode2;
		break;
	case 3:
		currentMode = PPUMode::mode3;
		break;
	}

	if ((*LY) == (*LYC) || (*STAT)&(1 << 6))
	{
		// Request STAT interrupt if LY matches LYC
		//bug on DMG models triggers a STAT interrupt anytime the STAT register is written
		//Road Rage and Zerd no Denetsu rely on this
		(*STAT) |= (1 << 2);
	}

	// Check for STAT interrupts and request if needed (e.g., when entering specific modes)
	bool hBlankInterruptEnabled = (*STAT)&(1 << 3);
	bool vBlankInterruptEnabled = (*STAT)&(1 << 4);/* Determine if VBlank interrupt is enabled */
	bool oamInterruptEnabled = (*STAT)&(1 << 5);/* Determine if OAM Search interrupt is enabled */

	if (currentMode == PPUMode::mode0 && hBlankInterruptEnabled)
	{
		// Request HBlank interrupt
	}
	else if (currentMode == PPUMode::mode1 && vBlankInterruptEnabled)
	{
		// Request VBlank interrupt
	}
	else if (currentMode == PPUMode::mode2 && oamInterruptEnabled)
	{
		// Request OAM Search interrupt
	}
}

int GameBoy::cyclesSinceLastScanline()
{
	int difference = cycles - lastScanline;
	if (difference < 0)
	{
		// Handle the case when cycles has wrapped around (overflowed)
		difference += T_CLOCK_FREQ;
	}
	return difference;
}

int GameBoy::cyclesSinceLastRefresh()
{
	int difference = cycles - lastRefresh;
	if (difference < 0)
	{
		// Handle the case when cycles has wrapped around (overflowed)
		difference += T_CLOCK_FREQ;
	}
	return difference;
}

void GameBoy::checkPPUMode()
{
	int oamFetchTime = 0;
	if ((*LY) < 144)
	{
		int currentDuration = cyclesSinceLastScanline();
		// Active Display Period (HBlank, OAM Search, and Pixel Transfer)
		if(currentDuration < MODE2_DURATION)
			setPPUMode(PPUMode::mode2);
		else if(currentDuration < MODE2_DURATION + MODE3_BASE_DURATION + oamFetchTime)
			setPPUMode(PPUMode::mode3);
		else
			setPPUMode(PPUMode::mode0);
	}
	// VBlank Period
	else
		setPPUMode(PPUMode::mode1);
}

void GameBoy::setPPUMode(PPUMode mode)
{
	switch(mode)
	{
	case PPUMode::mode0:
		(*STAT) &= ~0x03;
		break;
	case PPUMode::mode1:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x01;
		//set vblank interrupt flag
		(*IF) |= 0x01;
		break;
	case PPUMode::mode2:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x02;
		break;
	case PPUMode::mode3:
		(*STAT) &= ~0x03;
		(*STAT) |= 0x03;
		break;
	}
}

void GameBoy::drawLine()
{
	uint8_t line = (*LY);

	// Calculate the starting index of the current scanline in the framebuffer
	uint32_t lineStartIndex = line * RESOLUTION_X;

	// Pointer to the current line's pixel data in the framebuffer
	uint32_t* currentLinePixels = framebuffer + lineStartIndex;

	// For example, if you are setting the entire scanline to a color (e.g., white):
	for (int i = 0; i < RESOLUTION_X; i++)
	{
		// Assuming white color is represented as 0xFFFFFFFF (ARGB format)
//		if(currentLinePixels[i] == 0xFFFFFFFF)
//			currentLinePixels[i] = 0xFF000000;
//		else
			currentLinePixels[i] = 0xFFFFFFFF;
	}
}

void GameBoy::SDL2setup()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_CreateWindow("GBpp",
							SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							RESOLUTION_X, RESOLUTION_Y,
							SDL_WINDOW_OPENGL);

	// Create an SDL renderer to draw on the window
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

	// Create an SDL texture to hold the framebuffer data
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, RESOLUTION_X, RESOLUTION_Y);
}

void GameBoy::SDL2destroy()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	SDL_Quit();
}

void GameBoy::SDL2present()
{
	// Update the SDL texture with the framebuffer data
	SDL_UpdateTexture(texture, NULL, framebuffer, RESOLUTION_X * sizeof(uint32_t));

	// Clear the renderer and render the texture
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	// Present the renderer on the screen
	SDL_RenderPresent(renderer);
}