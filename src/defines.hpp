#ifndef GBPP_SRC_DEFINES_HPP_
#define GBPP_SRC_DEFINES_HPP_

#define Word uint16_t
#define Byte uint8_t

// Bit  Name  Set Clr  Expl.
// 7    zf    Z   NZ   Zero Flag
// 6    n     -   -    Add/Sub-Flag (BCD)
// 5    h     -   -    Half Carry Flag (BCD)
// 4    cy    C   NC   Carry Flag
// 3-0  -     -   -    Not used (always zero)
#define CARRY_FLAG 4 //'C'
#define HALFCARRY_FLAG 5 //'H'
#define SUBTRACT_FLAG 6 //'N'
#define ZERO_FLAG 7 //'Z'

#define VBLANK_INTERRUPT 0
#define LCD_STAT_INTERRUPT 1
#define TIMER_INTERRUPT 2
#define SERIAL_INTERRUPT 3
#define JOYPAD_INTERRUPT 4

#define T_CLOCK_FREQ 4194304

#define DIVIDER_REGISTER_FREQ 16384

#define BOOTROM_SIZE 0x100

#define RESOLUTION_X 160
#define RESOLUTION_Y 144
#define SCREEN_BPP 3

#define ROM_BANK_SIZE 0x4000

#define SCANLINES_PER_FRAME 154
#define SCANLINE_DURATION 456
#define FRAME_DURATION 70224
#define MODE2_DURATION 80
#define MODE3_BASE_DURATION 168
#define MODE0_3_DURATION 376 //mode3 is 168 to 291, mode0 85 to 208
#define MODE1_DURATION 4560

#define H_SYNC 9198
#define V_SYNC 59.73
#define HBlank_DURATION 204 //GPU_MODE 0
#define SCANLINE_OAM_FREQ 80 //GPU_MODE 2
#define SCANLINE_VRAM_FREQ 80 //GPU_MODE 3


#endif
