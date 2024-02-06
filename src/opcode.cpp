#include "gameboy.hpp"

void GameBoy::setFlag(Byte bit) {
	AF.lo |= (1 << bit);
}

void GameBoy::resetFlag(Byte bit) {
	AF.lo &= ~(1 << bit);
}

bool GameBoy::getFlag(Byte bit) const {
	return (AF.lo >> bit) & 1;
}

Word GameBoy::getWordPC() {
	RegisterPair word;

	//remember little endianness
	word.lo = addressSpace[PC + 1];
	word.hi = addressSpace[PC + 2];

	return word.reg;
}

Byte GameBoy::getBytePC() {
	return addressSpace[PC + 1];
}

Word GameBoy::getWordSP() {
	RegisterPair word;

	//remember little endianness
	word.lo = addressSpace[SP++];
	word.hi = addressSpace[SP++];

	return word.reg;
}

Byte GameBoy::getByteSP() {
	return addressSpace[SP++];
}

void GameBoy::ret() {
	pop(PC);
}

template <typename T>
void GameBoy::ld(T& dest, T src) {
	dest = src;
}

template <typename T>
void GameBoy::ldW(T dest, T src) {
	if (sizeof(src) == sizeof(Word)) {
		addressSpace[dest] = (Byte)(src & 0xFF00) >> 8;
		addressSpace[dest + 1] = (Byte)(src & 0xFF);
	}
}

template <typename T>
void GameBoy::rla(T& reg) {
	//printf("0x%.2x\n", REG);
	//printf("%d\n", GET_FLAG(CARRY_FLAG));
	bool carry;

	//printf("\n0x%x\n", REG);
	//printf("0x%x\n\n", REG & ((T)1 << 7));

	if (reg & (1 << 7))
		carry = true;
	else
		carry = false;

	reg <<= 1;

	if (getFlag(CARRY_FLAG))
		reg += 1;

	if (carry)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	resetFlag(ZERO_FLAG);
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::add(T& reg, T value) {
	if (sizeof(reg) == sizeof(Byte)) {
		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if ((((value & 0xF) + (reg & 0xF)) & 0x10) == 0x10)
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if ((((value & 0xFF) + (reg & 0xFF)) & 0x100) == 0x100)
			setFlag(CARRY_FLAG);
		else
			resetFlag(CARRY_FLAG);
	}

	if (sizeof(reg) == sizeof(Word)) {
		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if ((((value & 0xFFF) + (reg & 0xFFF)) & 0x1000) == 0x1000)
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if ((((value & 0xFFFF) + (reg & 0xFFFF)) & 0x10000) == 0x10000)
			setFlag(CARRY_FLAG);
		else
			resetFlag(CARRY_FLAG);
	}

	reg += value;

	if (reg == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
}

template <typename T>
void GameBoy::orBitwise(T& dest, T src) {
	dest |= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
	resetFlag(CARRY_FLAG);
}

template <typename T>
void GameBoy::andBitwise(T& dest, T src) {
	dest &= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
	resetFlag(CARRY_FLAG);
}

template <typename T>
void GameBoy::xorBitwise(T& dest, T src) {
	dest ^= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(CARRY_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::bit(T testBit, T reg) {
	Byte result = reg & (T)(1 << testBit);

	if (result == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
}

template <typename T>
bool GameBoy::jrNZ(T offset) {
	bool jumped = false;
	if (!getFlag(ZERO_FLAG)) //if not set
	{
		PC += (int8_t)offset + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

template <typename T>
void GameBoy::inc(T& reg) {
	reg++;

	if (sizeof(reg) == sizeof(Byte)) {
		if (reg == 0)
			setFlag(ZERO_FLAG);
		else
			resetFlag(ZERO_FLAG);

		resetFlag(SUBTRACT_FLAG);

		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if (((((reg - 1) & 0xf) + (reg & 0xf)) & 0x10) == 0x10)
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
	}
}

template <typename T>
void GameBoy::call(T address) {
	push(PC + 3);
	PC = address;
}

template <typename T>
void GameBoy::cp(T value) //compare
{
	if (AF.hi < value) {
		setFlag(CARRY_FLAG);
		resetFlag(ZERO_FLAG);
	}
	else if (AF.hi == value) {
		setFlag(ZERO_FLAG);
		resetFlag(CARRY_FLAG);
	}

	setFlag(SUBTRACT_FLAG);

	//halfcarry test https://www.reddit.com/r/EmuDev/comments/4clh23/trouble_with_halfcarrycarry_flag/
	if (0 > (((AF.hi) & 0xf) - (value & 0xf)))
		setFlag(HALFCARRY_FLAG);
	else
		resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::dec(T& reg) {
	reg -= 1;

	if (sizeof(reg) == sizeof(Byte)) {
		if (reg == 0)
			setFlag(ZERO_FLAG);
		else
			resetFlag(ZERO_FLAG);

		setFlag(SUBTRACT_FLAG);

		//halfcarry test https://www.reddit.com/r/EmuDev/comments/4clh23/trouble_with_halfcarrycarry_flag/
		if (0 > (((reg + 1) & 0xf) - (reg & 0xf)))
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
	}
}

template <typename T>
bool GameBoy::jrZ(T offset) {
	bool jumped = false;
	if (getFlag(ZERO_FLAG)) //if not set
	{
		PC += (int8_t)offset + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

void GameBoy::swap(Byte& value) {
	// Extract the lower and upper nibbles of the register
	Byte lowerNibble = value & 0x0F;
	Byte upperNibble = (value >> 4) & 0x0F;

	// Swap the lower and upper nibbles
	value = (lowerNibble << 4) | upperNibble;
	if (value == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
	resetFlag(CARRY_FLAG);
}

void GameBoy::halt() {}

template <typename T>
void GameBoy::sub(T value) {
	if (AF.hi < value) {
		setFlag(CARRY_FLAG);
		resetFlag(ZERO_FLAG);
	}
	else if (AF.hi == value) {
		setFlag(ZERO_FLAG);
		resetFlag(CARRY_FLAG);
	}

	AF.hi -= value;

	setFlag(SUBTRACT_FLAG);
	//halfcarry test https://www.reddit.com/r/EmuDev/comments/4clh23/trouble_with_halfcarrycarry_flag/
	if (0 > (((AF.hi) & 0xf) - (value & 0xf)))
		setFlag(HALFCARRY_FLAG);
	else
		resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::jr(T offset) {
	PC += (int8_t)offset + 2; //PC moves 2 from original instruction
}

template <typename T>
void GameBoy::rl(T& reg) {
	bool carry;

	if (reg & (1 << 7))
		carry = true;
	else
		carry = false;

	reg <<= 1;

	if (getFlag(CARRY_FLAG))
		reg += 1;

	if (carry)
		setFlag(CARRY_FLAG);

	else
		resetFlag(CARRY_FLAG);

	if (reg == 0)
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::pop(T& reg) {
	reg = getWordSP();
}

template <typename T>
void GameBoy::push(T reg) {
	//little endian
	RegisterPair temp;
	temp.lo = reg & 0xFF;
	temp.hi = reg >> 8;
	SP--;
	addressSpace[SP] = temp.hi;
	SP--;
	addressSpace[SP] = temp.lo;
}

template <typename T>
void GameBoy::jp(T address) {
	PC = address;
}

template <typename T>
void GameBoy::rst(T address) {
	push(PC);
	PC = address;
}

void GameBoy::cpl() {
	AF.hi = ~AF.hi;
	setFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
}

void GameBoy::ccf() {
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
	if (getFlag(CARRY_FLAG))
		resetFlag(CARRY_FLAG);
	else
		setFlag(CARRY_FLAG);
}

void GameBoy::stop() {}

void GameBoy::opcodeHandler() {
	bool jumped;

	if (addressSpace[PC] != 0xCB) {
		//printf("PC:0x%.2x, Opcode:0x%.2x\n", PC, addressSpace[PC]);
		if (PC == 0x100) {
			printf("LY:0x%.2x\n", (*LY));
			exit(1);
			// printf("PC:0x%.2x, Opcode:0x%.2x\n", PC, addressSpace[PC]);
			// printf("IME:%b IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));
		}
		//printf("IME:%b IF:0x%.2x IE:0x%.2x\n", IME, (*IF), (*IE));

		switch (addressSpace[PC]) {
		case 0x00:
			//NOP
			PC += 1;
			addCycles(4);
			break;

		case 0x01:
			ld(BC.reg, getWordPC());
			PC += 3;
			addCycles(12);
			break;

		case 0x02:
			ld(addressSpace[BC.reg], AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x03:
			BC.reg += 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x04:
			inc(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x05:
			dec(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x06:
			ld(BC.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x08:
			ldW(getWordPC(), SP);
			PC += 3;
			addCycles(20);
			break;

		case 0x09:
			add(HL.reg, BC.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x0A:
			ld(AF.hi, addressSpace[BC.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x0B:
			BC.reg -= 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x0C:
			inc(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x0D:
			dec(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x0E:
			ld(BC.lo, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x10:
			stop();
			PC += 2;
			addCycles(4);
			break;

		case 0x11:
			ld(DE.reg, getWordPC());
			PC += 3;
			addCycles(12);
			break;

		case 0x12:
			ld(addressSpace[BC.reg], AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x13:
			DE.reg++; //no flags change no just inc it manually
			PC += 1;
			addCycles(8);
			break;

		case 0x14:
			inc(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x15:
			dec(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x16:
			ld(DE.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x17:
			rla(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x18:
			jr(getBytePC());
			PC += 1;
			addCycles(4);
			break;

		case 0x19:
			add(HL.reg, BC.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x1A:
			ld(AF.hi, addressSpace[DE.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x1B:
			DE.reg -= 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x1C:
			inc(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x1D:
			dec(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x1E:
			ld(DE.lo, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x20:
			jumped = jrNZ(getBytePC());
			if (jumped) {
				addCycles(12);
			}
			else {
				PC += 2;
				addCycles(8);
			}
			break;

		case 0x21:
			ld(HL.reg, getWordPC());
			PC += 3;
			addCycles(12);
			break;

		case 0x22:
			ld(addressSpace[HL.reg], AF.hi);
			HL.reg++;
			PC += 1;
			addCycles(8);
			break;

		case 0x23:
			inc(HL.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x24:
			inc(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x25:
			dec(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x28:
			jumped = jrZ(getBytePC());
			if (jumped) {
				addCycles(12);
			}
			else {
				PC += 2;
				addCycles(8);
			}
			break;

		case 0x2A:
			ld(AF.hi, addressSpace[HL.reg]);
			HL.reg += 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x2E:
			ld(HL.lo, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x2F:
			cpl();
			PC += 1;
			addCycles(4);
			break;

		case 0x31:
			ld(SP, getWordPC());
			PC += 3;
			addCycles(12);
			break;

		case 0x32:
			ld(addressSpace[HL.reg], AF.hi);
			HL.reg--;
			PC += 1;
			addCycles(8);
			break;

		case 0x33:
			SP += 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x34:
			inc(addressSpace[HL.reg]);
			PC += 1;
			addCycles(12);
			break;

		case 0x35:
			dec(addressSpace[HL.reg]);
			PC += 1;
			addCycles(12);
			break;

		case 0x36:
			ld(addressSpace[HL.reg], getBytePC());
			PC += 2;
			addCycles(12);
			break;

		case 0x39:
			add(HL.reg, SP);
			PC += 1;
			addCycles(8);
			break;

		case 0x3A:
			ld(AF.hi, addressSpace[HL.reg]);
			HL.reg -= 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x3B:
			SP--;
			PC += 1;
			addCycles(8);
			break;

		case 0x3C:
			inc(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x3D:
			dec(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x3E:
			ld(AF.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x3F:
			ccf();
			PC += 1;
			addCycles(4);
			break;

		case 0x40:
			ld(BC.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x41:
			ld(BC.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x42:
			ld(BC.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x43:
			ld(BC.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x44:
			ld(BC.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x45:
			ld(BC.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x46:
			ld(BC.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x47:
			ld(BC.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x48:
			ld(BC.lo, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x49:
			ld(BC.lo, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x4A:
			ld(BC.lo, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x4B:
			ld(BC.lo, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x4C:
			ld(BC.lo, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x4D:
			ld(BC.lo, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x4E:
			ld(BC.lo, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x4F:
			ld(BC.lo, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x50:
			ld(DE.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x51:
			ld(DE.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x52:
			ld(DE.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x53:
			ld(DE.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x54:
			ld(DE.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x55:
			ld(DE.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x56:
			ld(DE.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x57:
			ld(DE.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x58:
			ld(DE.lo, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x59:
			ld(DE.lo, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x5A:
			ld(DE.lo, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x5B:
			ld(DE.lo, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x5C:
			ld(DE.lo, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x5D:
			ld(DE.lo, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x5E:
			ld(DE.lo, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x5F:
			ld(DE.lo, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x60:
			ld(HL.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x61:
			ld(HL.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x62:
			ld(HL.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x63:
			ld(HL.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x64:
			ld(HL.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x65:
			ld(HL.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x66:
			ld(HL.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x67:
			ld(HL.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x68:
			ld(HL.lo, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x69:
			ld(HL.lo, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x6A:
			ld(HL.lo, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x6B:
			ld(HL.lo, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x6C:
			ld(HL.lo, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x6D:
			ld(HL.lo, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x6E:
			ld(HL.lo, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x6F:
			ld(HL.lo, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x70:
			ld(addressSpace[HL.reg], BC.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x71:
			ld(addressSpace[HL.reg], BC.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x72:
			ld(addressSpace[HL.reg], DE.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x73:
			ld(addressSpace[HL.reg], DE.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x74:
			ld(addressSpace[HL.reg], HL.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x75:
			ld(addressSpace[HL.reg], HL.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x76:
			halt();
			PC += 1;
			addCycles(4);
			break;

		case 0x77:
			ld(addressSpace[HL.reg], AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x78:
			ld(AF.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x79:
			ld(AF.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x7A:
			ld(AF.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x7B:
			ld(AF.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x7C:
			ld(AF.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x7D:
			ld(AF.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x7E:
			ld(AF.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x7F:
			ld(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x80:
			add(AF.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x81:
			add(AF.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x82:
			add(AF.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x83:
			add(AF.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x84:
			add(AF.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x85:
			add(AF.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x86:
			add(AF.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x87:
			add(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x90:
			sub(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x91:
			sub(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x92:
			sub(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x93:
			sub(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x94:
			sub(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x95:
			sub(HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x96:
			sub(addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x97:
			sub(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA0:
			andBitwise(AF.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA1:
			andBitwise(AF.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xA2:
			andBitwise(AF.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA3:
			andBitwise(AF.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xA4:
			andBitwise(AF.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA5:
			andBitwise(AF.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xA6:
			andBitwise(AF.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0xA7:
			andBitwise(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA8:
			xorBitwise(AF.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xA9:
			xorBitwise(AF.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xAA:
			xorBitwise(AF.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xAB:
			xorBitwise(AF.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xAC:
			xorBitwise(AF.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xAD:
			xorBitwise(AF.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xAE:
			xorBitwise(AF.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0xAF:
			xorBitwise(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB0:
			orBitwise(AF.hi, BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB1:
			orBitwise(AF.hi, BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xB2:
			orBitwise(AF.hi, DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB3:
			orBitwise(AF.hi, DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xB4:
			orBitwise(AF.hi, HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB5:
			orBitwise(AF.hi, HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xB6:
			orBitwise(AF.hi, addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0xB7:
			orBitwise(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB8:
			cp(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xB9:
			cp(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xBA:
			cp(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xBB:
			cp(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xBC:
			cp(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xBD:
			cp(HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0xBE:
			cp(addressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0xBF:
			cp(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0xC0: //RET NZ
			if (!getFlag(ZERO_FLAG)) {
				ret();
				addCycles(20);
			}
			else {
				addCycles(8);
				PC += 1;
			}
			break;

		case 0xC1:
			pop(BC.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xC2:
			if (!getFlag(ZERO_FLAG)) {
				jp(getWordPC());
				addCycles(16);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xC3:
			jp(getWordPC());
			addCycles(16);
			break;

		case 0xC4:
			if (!getFlag(ZERO_FLAG)) {
				call(getWordPC());
				addCycles(24);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xC5:
			push(BC.reg);
			PC += 1;
			addCycles(16);
			break;

		case 0xC8:
			if (getFlag(ZERO_FLAG)) {
				ret();
				addCycles(20);
			}
			else {
				addCycles(8);
				PC += 1;
			}
			break;

		case 0xC9:
			ret();
			addCycles(16);
			break;

		case 0xCA:
			if (getFlag(ZERO_FLAG)) {
				jp(getWordPC());
				addCycles(16);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xCC:
			if (getFlag(ZERO_FLAG)) {
				call(getWordPC());
				addCycles(24);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xCD:
			call(getWordPC());
			addCycles(8);
			break;

		case 0xCF:
			rst(0x08);
			addCycles(16);
			break;

		case 0xD0: //RET NC
			if (!getFlag(CARRY_FLAG)) {
				ret();
				addCycles(20);
			}
			else {
				addCycles(8);
				PC += 3;
			}
			break;

		case 0xD1:
			pop(DE.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xD2:
			if (!getFlag(CARRY_FLAG)) {
				jp(getWordPC());
				addCycles(24);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xD4:
			if (!getFlag(CARRY_FLAG)) {
				call(getWordPC());
				addCycles(24);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xD5:
			push(DE.reg);
			PC += 1;
			addCycles(16);
			break;

		case 0xD8:
			if (getFlag(CARRY_FLAG)) {
				ret();
				addCycles(20);
			}
			else {
				addCycles(8);
				PC += 1;
			}
			break;

		//reti
		case 0xD9:
			IME = 1;
			ret();
			addCycles(16);
			break;

		case 0xDA:
			if (getFlag(CARRY_FLAG)) {
				jp(getWordPC());
				addCycles(16);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xDC:
			if (getFlag(CARRY_FLAG)) {
				call(getWordPC());
				addCycles(24);
			}
			else {
				addCycles(12);
				PC += 3;
			}
			break;

		case 0xDF:
			rst(0x18);
			addCycles(16);
			break;

		case 0xE0:
			ld(addressSpace[0xFF00 + getBytePC()], AF.hi);
			PC += 2;
			addCycles(8);
			break;

		case 0xE1:
			pop(HL.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xE2:
			ld(addressSpace[BC.lo + 0xFF00], AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0xE5:
			push(HL.reg);
			PC += 1;
			addCycles(16);
			break;

		case 0xE6:
			andBitwise(AF.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		//		case 0xE8:
		//			SP += (int8_t)getByte();
		//			PC += 2;
		//			addCycles(16);
		//			break;

		case 0xE9:
			jp(HL.reg);
			addCycles(16);
			break;

		case 0xEA:
			ld(addressSpace[getWordPC()], AF.hi);
			PC += 3;
			addCycles(16);
			break;

		case 0xEF:
			rst(0x28);
			addCycles(16);
			break;

		case 0xF0:
			ld(AF.hi, addressSpace[0xFF00 + getBytePC()]);
			PC += 2;
			addCycles(12);
			break;

		case 0xF1:
			pop(AF.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xF3:
			IME = 0;
			PC += 1;
			addCycles(4);
			break;

		case 0xF5:
			push(AF.reg);
			PC += 1;
			addCycles(16);
			break;

		case 0xFA:
			ldW(AF.hi, addressSpace[getWordPC()]);
			PC += 3;
			addCycles(16);
			break;

		//should not actually enable until the next opcode
		//EI 0xFB then DI 0xF3 does not allow interrupts to happen
		case 0xFB:
			IME = 1;
			PC += 1;
			addCycles(4);
			break;

		case 0xFE:
			cp(getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xFF:
			rst(0x38);
			addCycles(16);
			break;

		default:
			printf("Unimplemented opcode found: PC:0x%.2x, Opcode:0x%.2x\n", PC, addressSpace[PC]);
			exit(1);
		}
	}
	else //extension
	{
		//printf("PC:0x%.2x, Opcode:0x%x%.2x\n", PC, addressSpace[PC], addressSpace[PC + 1]);
		PC += 1;
		addCycles(4);

		//extension handler
		switch (addressSpace[PC]) {
		case 0x11:
			rl(BC.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x30:
			swap(BC.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x31:
			swap(BC.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x32:
			swap(DE.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x33:
			swap(DE.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x34:
			swap(HL.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x35:
			swap(HL.lo);
			PC += 1;
			addCycles(8);
			break;

		case 0x36:
			swap(addressSpace[HL.reg]);
			PC += 1;
			addCycles(16);
			break;

		case 0x37:
			swap(AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x7C:
			bit((Byte)7, HL.hi);
			PC += 1;
			addCycles(8);
			break;

		default:
			printf("Unimplemented extended opcode found: PC:0x%.2x, Opcode:0xcb%.2x\n", PC, addressSpace[PC]);
			exit(1);
		}
	}
}
