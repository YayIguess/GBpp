#ifndef GBPP_SRC_DEFINES_HPP_
#define GBPP_SRC_DEFINES_HPP_

#define Word uint16_t
#define Byte uint8_t

// Bit  Name  Set Clr  Expl.
// 7    zf    Z   NZ   Zero Flag
// 6    n     -   -    Add/Sub-Flag (BCD)
// 5    h     -   -    Half Carry Flag (BCD)
// 4    cy    C   NC   Carry Flag
// 3-0  -     -   -    Not used
#define CARRY_FLAG 4 //'C'
#define HALFCARRY_FLAG 5 //'H'
#define SUBTRACT_FLAG 6 //'N'
#define ZERO_FLAG 7 //'Z'

#define VBLANK_INTERRUPT 0
#define LCD_STAT_INTERRUPT 1
#define TIMER_INTERRUPT 2
#define SERIAL_INTERRUPT 3
#define JOYPAD_INTERRUPT 4

#define T_CLOCK_FREQ 4194304 //2^22

#define DIVIDER_REGISTER_FREQ (4194304/16384)

#define BOOTROM_SIZE 0x100

#define RESOLUTION_X 160
#define RESOLUTION_Y 144
#define SCREEN_BPP 3

#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

#define SCANLINES_PER_FRAME 154
#define SCANLINE_DURATION 456
#define FRAME_DURATION 70224
#define MODE2_DURATION 80
#define MODE3_MIN_DURATION 172

#define H_SYNC 9198
#define V_SYNC 59.73
#define HBLANK_DURATION 204 //PPU_MODE 0
#define VBLANK_DURATION 4560
#define SCANLINE_OAM_FREQ 80 //PPU_MODE 2
#define SCANLINE_VRAM_FREQ 80 //PPU_MODE 3

//two bits per colour
enum Colour {
	black = 0b11,
	darkGray = 0b10,
	lightGray = 0b01,
	white = 0b00
};

enum MBCType {
	romOnly = 0x00,
	MBC1 = 0x01,
	MBC1Ram = 0x02,
	MBC1RamBattery = 0x03,
	MBC2 = 0x05,
	MBC2Battery = 0x06,
	RomRam = 0x08, //unused
	RomRamBattery = 0x09, //unused
	MMM01 = 0x0B, //multigame roms only
	MMM01Ram = 0x0C,
	MMM01RamBattery = 0x0D,
	MBC3TimerBattery = 0x0F,
	MBC3TimerRamBattery = 0x10,
	MBC3 = 0x11,
	MBC3Ram = 0x12, // MBC3 with 64 KiB of SRAM refers to MBC30, used only in Pocket Monsters: Crystal Version.
	MBC3RamBattery = 0x13,
	MBC5 = 0x19,
	MBC5Ram = 0x1A,
	MBC5RamBattery = 0x1B,
	MBC5Rumble = 0x1C,
	MBC5RumbleRam = 0x1D,
	MBC5RumbleRamBattery = 0x1E,
	MBC6 = 0x20,
	MBC7SensorRumbleRamBattery = 0x22,
	PocketCamera = 0xFC,
	BandaiTama5 = 0xFD,
	HuC3 = 0xFE,
	HuC1RamBattery = 0xFF
};

enum PPUMode {
	mode0, // Horizontal Blank (Mode 0): No access to video RAM, occurs during horizontal blanking period.
	mode1, // Vertical Blank (Mode 1): No access to video RAM, occurs during vertical blanking period.
	mode2, // OAM Search (Mode 2): Access to OAM (Object Attribute Memory) only, sprite evaluation.
	mode3 // Pixel Transfer (Mode 3): Access to both OAM and video RAM, actual pixel transfer to the screen.
};

#endif
