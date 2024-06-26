#include "gameboy.hpp"

void GameBoy::setFlag(const Byte bit) {
	AF.lo |= (1 << bit);
}

void GameBoy::resetFlag(const Byte bit) {
	AF.lo &= ~(1 << bit);
}

bool GameBoy::getFlag(const Byte bit) const {
	return (AF.lo >> bit) & 1;
}

Word GameBoy::getWordPC() {
	RegisterPair word = {0};

	//remember little endianness
	word.lo = readOnlyAddressSpace[PC + 1];
	word.hi = readOnlyAddressSpace[PC + 2];

	return word.reg;
}

Byte GameBoy::getBytePC() {
	return readOnlyAddressSpace[PC + 1];
}

Word GameBoy::getWordSP() {
	RegisterPair word = {0};

	//remember little endianness
	word.lo = readOnlyAddressSpace[SP++];
	word.hi = readOnlyAddressSpace[SP++];

	return word.reg;
}

Byte GameBoy::getByteSP() {
	return readOnlyAddressSpace[SP++];
}

void GameBoy::ret() {
	PC = readOnlyAddressSpace[SP++];
	PC |= readOnlyAddressSpace[SP++] << 8;
}

template <typename T>
void GameBoy::ld(T& dest, T src) {
	if constexpr (std::is_same_v<T, Byte>) {
		if (&dest == &addressSpace.memoryLayout.DIV) {
			addressSpace.memoryLayout.DIV = 0x00;
			lastDivUpdate = cycles;
		}
		else {
			dest = src;
		}
	}
	else {
		//16-bit register pair write
		dest = src;
	}
}

void GameBoy::ldW(const Word destAddr, const Word src) {
	addressSpace[destAddr] = static_cast<Byte>(src & 0xFF);
	addressSpace[destAddr + 1] = static_cast<Byte>((src & 0xFF00) >> 8);
}

template <typename T>
void GameBoy::add(T& reg, T value) {
	if (sizeof(reg) == sizeof(Byte)) {
		if ((((value & 0xF) + (reg & 0xF)) & 0x10) == 0x10)
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
		if ((((value & 0xFF) + (reg & 0xFF)) & 0x100) == 0x100)
			setFlag(CARRY_FLAG);
		else
			resetFlag(CARRY_FLAG);
	}

	if (sizeof(reg) == sizeof(Word)) {
		if (((value & 0xFFF) + (reg & 0xFFF)) & 0x1000)
			setFlag(HALFCARRY_FLAG);
		else
			resetFlag(HALFCARRY_FLAG);
		if ((static_cast<unsigned>(value) + static_cast<unsigned>(reg)) & 0x10000)
			setFlag(CARRY_FLAG);
		else
			resetFlag(CARRY_FLAG);
	}

	reg += value;

	if (sizeof(reg) == sizeof(Byte)) {
		if (reg == 0)
			setFlag(ZERO_FLAG);
		else
			resetFlag(ZERO_FLAG);
	}

	resetFlag(SUBTRACT_FLAG);
}

void GameBoy::adc(const Byte value) {
	Byte carry = getFlag(CARRY_FLAG) ? 1 : 0;

	if ((AF.hi & 0xF) + (value & 0xF) + carry > 0xF)
		setFlag(HALFCARRY_FLAG);
	else
		resetFlag(HALFCARRY_FLAG);
	if ((value & 0xFF) + (AF.hi & 0xFF) + carry > 0xFF)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);


	AF.hi += value + carry;

	if (AF.hi == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
}

void GameBoy::sub(const Byte value) {
	if (AF.hi < value)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);
	if (AF.hi == value)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);
	if ((AF.hi & 0xf) < (value & 0xf))
		setFlag(HALFCARRY_FLAG);
	else
		resetFlag(HALFCARRY_FLAG);

	AF.hi -= value;

	setFlag(SUBTRACT_FLAG);
}

void GameBoy::sbc(const Byte value) {
	const Byte carry = getFlag(CARRY_FLAG) ? 1 : 0;
	const Byte result = AF.hi - value - carry;

	if ((static_cast<unsigned>(AF.hi) - static_cast<unsigned>(value) - carry) > 0xFF)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (result == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	if ((AF.hi & 0xF) < (value & 0xF) + carry)
		setFlag(HALFCARRY_FLAG);
	else
		resetFlag(HALFCARRY_FLAG);

	AF.hi = result;

	setFlag(SUBTRACT_FLAG);
}

//https://gbdev.gg8.se/wiki/articles/DAA
void GameBoy::daa() {
	if (getFlag(SUBTRACT_FLAG)) {
		if (getFlag(CARRY_FLAG)) {
			AF.hi -= 0x60;
		}
		if (getFlag(HALFCARRY_FLAG)) {
			AF.hi -= 0x06;
		}
	}
	else {
		if (getFlag(CARRY_FLAG) || (AF.hi & 0xFF) > 0x99) {
			AF.hi += 0x60;
			setFlag(CARRY_FLAG);
		}
		if (getFlag(HALFCARRY_FLAG) || (AF.hi & 0x0F) > 0x09) {
			AF.hi += 0x06;
		}
	}

	if (AF.hi == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::orBitwise(T& dest, T src) {
	dest |= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
	resetFlag(CARRY_FLAG);
}

template <typename T>
void GameBoy::andBitwise(T& dest, T src) {
	dest &= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
	resetFlag(CARRY_FLAG);
}

template <typename T>
void GameBoy::xorBitwise(T& dest, T src) {
	dest ^= src;

	if (dest == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(CARRY_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::bit(Byte testBit, Byte reg) {
	if (const Byte result = reg & (1 << testBit); result == 0)
		setFlag(ZERO_FLAG);
	else
		resetFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
}

void GameBoy::set(const Byte testBit, Byte& reg) {
	reg |= (1 << testBit);
}

void GameBoy::res(const Byte testBit, Byte& reg) {
	reg &= ~(1 << testBit);
}


template <typename T>
void GameBoy::jr(T offset) {
	PC += static_cast<int8_t>(offset) + 2; //PC moves 2 from original instruction
}

template <typename T>
bool GameBoy::jrNZ(T offset) {
	bool jumped = false;
	if (!getFlag(ZERO_FLAG)) //if not set
	{
		PC += static_cast<int8_t>(offset) + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

template <typename T>
bool GameBoy::jrZ(T offset) {
	bool jumped = false;
	if (getFlag(ZERO_FLAG)) //if not set
	{
		PC += static_cast<int8_t>(offset) + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

template <typename T>
bool GameBoy::jrNC(T offset) {
	bool jumped = false;
	if (!getFlag(CARRY_FLAG)) //if not set
	{
		PC += static_cast<int8_t>(offset) + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

template <typename T>
bool GameBoy::jrC(T offset) {
	bool jumped = false;
	if (getFlag(CARRY_FLAG)) //if not set
	{
		PC += static_cast<int8_t>(offset) + 2; //PC moves 2 from the original instruction
		jumped = true;
	}

	return jumped;
}

template <typename T>
void GameBoy::inc(T& reg) {
	reg += 1;

	if (sizeof(reg) == sizeof(Byte)) {
		if (reg == 0)
			setFlag(ZERO_FLAG);
		else
			resetFlag(ZERO_FLAG);

		resetFlag(SUBTRACT_FLAG);

		//halfcarry test https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
		if ((reg & 0x0F) == 0)
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

void GameBoy::cp(const Byte value) //compare
{
	resetFlag(ZERO_FLAG);
	resetFlag(CARRY_FLAG);
	resetFlag(HALFCARRY_FLAG);

	if (AF.hi == value) {
		setFlag(ZERO_FLAG);
	}
	if ((AF.hi & 0xF) < (value & 0xF)) {
		setFlag(HALFCARRY_FLAG);
	}
	if (AF.hi < value) {
		setFlag(CARRY_FLAG);
	}

	setFlag(SUBTRACT_FLAG);
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

void GameBoy::swap(Byte& value) {
	const Byte lowerNibble = value & 0x0F;
	const Byte upperNibble = (value >> 4) & 0x0F;

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

void GameBoy::halt() {
	halted = true;
	if (!IME && addressSpace.memoryLayout.IE & addressSpace.memoryLayout.IF)
		haltBug = true;
}

void GameBoy::rrc(Byte& reg) {
	const Byte lsb = reg & 0x01;
	reg >>= 1;

	if (lsb)
		reg |= 0x80;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rrca() {
	const Byte lsb = AF.hi & 0x01;
	AF.hi >>= 1;

	if (lsb)
		AF.hi |= 0x80;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	resetFlag(ZERO_FLAG);
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rra() {
	const Byte lsb = AF.hi & 0x01;
	AF.hi >>= 1;

	if (getFlag(CARRY_FLAG))
		AF.hi |= 0x80;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	resetFlag(ZERO_FLAG);
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rr(Byte& reg) {
	const Byte lsb = reg & 0x01;

	reg >>= 1;

	if (getFlag(CARRY_FLAG))
		reg |= 0x80;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rlc(Byte& reg) {
	const Byte msb = (reg & 0x80) >> 7;
	reg <<= 1;

	reg |= msb;

	if (msb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rlca() {
	const Byte msb = (AF.hi & 0x80) >> 7;
	AF.hi <<= 1;

	AF.hi |= msb;

	if (msb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	resetFlag(ZERO_FLAG);
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rla() {
	const Byte msb = (AF.hi & 0x80) >> 7;
	AF.hi <<= 1;

	if (getFlag(CARRY_FLAG))
		AF.hi |= 0x01;

	if (msb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	resetFlag(ZERO_FLAG);
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::rl(Byte& reg) {
	const Byte msb = (reg & 0x80) >> 7;

	reg <<= 1;

	if (getFlag(CARRY_FLAG))
		reg |= 1;

	if (msb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::sla(Byte& reg) {
	const Byte msb = (reg & 0x80) >> 7;

	reg <<= 1;

	if (msb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::sra(Byte& reg) {
	const Byte msb = (reg & 0x80) >> 7;
	const Byte lsb = reg & 0x1;

	reg >>= 1;

	if (msb)
		reg |= 0x80;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::srl(Byte& reg) {
	const Byte lsb = reg & 0x1;

	reg >>= 1;

	if (lsb)
		setFlag(CARRY_FLAG);
	else
		resetFlag(CARRY_FLAG);

	if (reg)
		resetFlag(ZERO_FLAG);
	else
		setFlag(ZERO_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

template <typename T>
void GameBoy::pop(T& reg) {
	reg = getWordSP();
	AF.reg &= 0xFFF0;
}

void GameBoy::push(const Word reg) {
	//little endian
	addressSpace[--SP] = reg >> 8;
	addressSpace[--SP] = reg & 0xFF;
}

template <typename T>
void GameBoy::jp(T address) {
	PC = address;
}

template <typename T>
void GameBoy::rst(T address) {
	PC += 1;
	push(PC);
	PC = address;
}

void GameBoy::cpl() {
	AF.hi = ~AF.hi;
	setFlag(SUBTRACT_FLAG);
	setFlag(HALFCARRY_FLAG);
}

void GameBoy::scf() {
	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
	setFlag(CARRY_FLAG);
}

void GameBoy::ccf() {
	if (getFlag(CARRY_FLAG))
		resetFlag(CARRY_FLAG);
	else
		setFlag(CARRY_FLAG);

	resetFlag(SUBTRACT_FLAG);
	resetFlag(HALFCARRY_FLAG);
}

void GameBoy::stop() {
	stopped = true;
}

void GameBoy::opcodeResolver() {
	if (readOnlyAddressSpace[PC] != 0xCB) {
		bool jumped;
		switch (readOnlyAddressSpace[PC]) {
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

		case 0x07:
			rlca();
			PC += 1;
			addCycles(4);
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
			ld(AF.hi, readOnlyAddressSpace[BC.reg]);
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

		case 0x0F:
			rrca();
			PC += 1;
			addCycles(4);
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
			ld(addressSpace[DE.reg], AF.hi);
			PC += 1;
			addCycles(8);
			break;

		case 0x13:
			DE.reg += 1; //no flags change no just inc it manually
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
			rla();
			PC += 1;
			addCycles(4);
			break;

		case 0x18:
			jr(getBytePC());
			addCycles(12);
			break;

		case 0x19:
			add(HL.reg, DE.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x1A:
			ld(AF.hi, readOnlyAddressSpace[DE.reg]);
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

		case 0x1F:
			rra();
			PC += 1;
			addCycles(4);
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
			HL.reg += 1;
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

		case 0x26:
			ld(HL.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0x27:
			daa();
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

		case 0x29:
			add(HL.reg, HL.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x2A:
			ld(AF.hi, readOnlyAddressSpace[HL.reg]);
			HL.reg += 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x2B:
			dec(HL.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0x2C:
			inc(HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x2D:
			dec(HL.lo);
			PC += 1;
			addCycles(4);
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

		case 0x30:
			jumped = jrNC(getBytePC());
			if (jumped) {
				addCycles(12);
			}
			else {
				PC += 2;
				addCycles(8);
			}
			break;

		case 0x31:
			ld(SP, getWordPC());
			PC += 3;
			addCycles(12);
			break;

		case 0x32:
			ld(addressSpace[HL.reg], AF.hi);
			HL.reg -= 1;
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

		case 0x37:
			scf();
			PC += 1;
			addCycles(4);
			break;

		case 0x38:
			jumped = jrC(getBytePC());
			if (jumped) {
				addCycles(12);
			}
			else {
				PC += 2;
				addCycles(8);
			}
			break;

		case 0x39:
			add(HL.reg, SP);
			PC += 1;
			addCycles(8);
			break;

		case 0x3A:
			ld(AF.hi, readOnlyAddressSpace[HL.reg]);
			HL.reg -= 1;
			PC += 1;
			addCycles(8);
			break;

		case 0x3B:
			SP -= 1;
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
			ld(BC.hi, readOnlyAddressSpace[HL.reg]);
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
			ld(BC.lo, readOnlyAddressSpace[HL.reg]);
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
			ld(DE.hi, readOnlyAddressSpace[HL.reg]);
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
			ld(DE.lo, readOnlyAddressSpace[HL.reg]);
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
			ld(HL.hi, readOnlyAddressSpace[HL.reg]);
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
			ld(HL.lo, readOnlyAddressSpace[HL.reg]);
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
			ld(AF.hi, readOnlyAddressSpace[HL.reg]);
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
			add(AF.hi, readOnlyAddressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x87:
			add(AF.hi, AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x88:
			adc(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x89:
			adc(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x8A:
			adc(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x8B:
			adc(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x8C:
			adc(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x8D:
			adc(HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x8E:
			adc(readOnlyAddressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x8F:
			adc(AF.hi);
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
			sub(readOnlyAddressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x97:
			sub(AF.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x98:
			sbc(BC.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x99:
			sbc(BC.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x9A:
			sbc(DE.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x9B:
			sbc(DE.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x9C:
			sbc(HL.hi);
			PC += 1;
			addCycles(4);
			break;

		case 0x9D:
			sbc(HL.lo);
			PC += 1;
			addCycles(4);
			break;

		case 0x9E:
			sbc(readOnlyAddressSpace[HL.reg]);
			PC += 1;
			addCycles(8);
			break;

		case 0x9F:
			sbc(AF.hi);
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
			andBitwise(AF.hi, readOnlyAddressSpace[HL.reg]);
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
			xorBitwise(AF.hi, readOnlyAddressSpace[HL.reg]);
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
			orBitwise(AF.hi, readOnlyAddressSpace[HL.reg]);
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
			cp(readOnlyAddressSpace[HL.reg]);
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

		case 0xC6:
			add(AF.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xC7:
			rst(0x0000);
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
			addCycles(24);
			break;

		case 0xCE:
			adc(getBytePC());
			PC += 2;
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
				PC += 1;
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

		case 0xD6:
			sub(getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xD7:
			rst(0x0010);
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

		case 0xDE:
			sbc(getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xDF:
			rst(0x18);
			addCycles(16);
			break;

		case 0xE0:
			ld(addressSpace[0xFF00 + getBytePC()], AF.hi);
			PC += 2;
			addCycles(12);
			break;

		case 0xE1:
			pop(HL.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xE2:
			ld(addressSpace[0xFF00 + BC.lo], AF.hi);
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

		case 0xE7:
			rst(0x0020);
			addCycles(16);
			break;

		case 0xE8:
			{
				const int16_t immediate = static_cast<int8_t>(getBytePC());

				if ((SP & 0xF) + (immediate & 0xF) > 0xF)
					setFlag(HALFCARRY_FLAG);
				else
					resetFlag(HALFCARRY_FLAG);

				if ((SP & 0xFF) + (immediate & 0xFF) > 0xFF)
					setFlag(CARRY_FLAG);
				else
					resetFlag(CARRY_FLAG);

				SP += immediate;

				resetFlag(ZERO_FLAG);
				resetFlag(SUBTRACT_FLAG);

				PC += 2;
				addCycles(16);
			}
			break;

		case 0xE9:
			jp(HL.reg);
			addCycles(16);
			break;

		case 0xEA:
			ld(addressSpace[getWordPC()], AF.hi);
			PC += 3;
			addCycles(16);
			break;

		case 0xEE:
			xorBitwise(AF.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xEF:
			rst(0x28);
			addCycles(16);
			break;

		case 0xF0:
			ld(AF.hi, readOnlyAddressSpace[0xFF00 + getBytePC()]);
			PC += 2;
			addCycles(12);
			break;

		case 0xF1:
			pop(AF.reg);
			PC += 1;
			addCycles(12);
			break;

		case 0xF2:
			ld(AF.hi, readOnlyAddressSpace[0xFF00 + BC.lo]);
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

		case 0xF6:
			orBitwise(AF.hi, getBytePC());
			PC += 2;
			addCycles(8);
			break;

		case 0xF7:
			rst(0x0030);
			addCycles(16);
			break;

		case 0xF8:
			{
				const int16_t immediate = static_cast<int8_t>(getBytePC());
				HL.reg = SP + immediate;

				if ((SP & 0xF) + (immediate & 0xF) > 0xF)
					setFlag(HALFCARRY_FLAG);
				else
					resetFlag(HALFCARRY_FLAG);


				if ((SP & 0xFF) + (immediate & 0xFF) > 0xFF)
					setFlag(CARRY_FLAG);
				else
					resetFlag(CARRY_FLAG);


				resetFlag(ZERO_FLAG);
				resetFlag(SUBTRACT_FLAG);

				PC += 2;
				addCycles(12);
			}
			break;

		case 0xF9:
			ld(SP, HL.reg);
			PC += 1;
			addCycles(8);
			break;

		case 0xFA:
			ld(AF.hi, readOnlyAddressSpace[getWordPC()]);
			PC += 3;
			addCycles(16);
			break;

		//EI (0xFB) then DI (0xF3) never allows interrupts to happen
		case 0xFB:
			IME = 0;
			IME_togge = true;
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
			printf("Unsupported opcode found: PC:0x%.2x, Opcode:0x%.2x\n", PC, addressSpace[PC]);
			exit(1);
		}
	}
	else
		extendedOpcodeResolver();
}
