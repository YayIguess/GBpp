#include "gameboy.hpp"
#include "defines.hpp"

void GameBoy::joypadHandler() {
	const Byte joyP = addressSpace.memoryLayout.JOYP;
	const bool buttons = (joyP & 0x20) == 0;
	const bool dpad = (joyP & 0x10) == 0;

	addressSpace.memoryLayout.JOYP |= 0xCF;

	if (buttons && !dpad) {
		if (joypadInput.A)
			addressSpace.memoryLayout.JOYP &= ~0x1;
		if (joypadInput.B)
			addressSpace.memoryLayout.JOYP &= ~0x2;
		if (joypadInput.SELECT)
			addressSpace.memoryLayout.JOYP &= ~0x4;
		if (joypadInput.START)
			addressSpace.memoryLayout.JOYP &= ~0x8;
	}
	if (!buttons && dpad) {
		if (joypadInput.RIGHT)
			addressSpace.memoryLayout.JOYP &= ~0x1;
		if (joypadInput.LEFT)
			addressSpace.memoryLayout.JOYP &= ~0x2;
		if (joypadInput.UP)
			addressSpace.memoryLayout.JOYP &= ~0x4;
		if (joypadInput.DOWN)
			addressSpace.memoryLayout.JOYP &= ~0x8;
	}

	if (buttons && dpad) {
		addressSpace.memoryLayout.JOYP |= 0xCF;

		if (joypadInput.RIGHT && joypadInput.A)
			addressSpace.memoryLayout.JOYP &= ~0x1;
		if (joypadInput.LEFT && joypadInput.B)
			addressSpace.memoryLayout.JOYP &= ~0x2;
		if (joypadInput.UP && joypadInput.SELECT)
			addressSpace.memoryLayout.JOYP &= ~0x4;
		if (joypadInput.DOWN && joypadInput.START)
			addressSpace.memoryLayout.JOYP &= ~0x8;
	}
}
