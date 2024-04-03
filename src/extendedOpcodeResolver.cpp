#include "gameboy.hpp"

void GameBoy::extendedOpcodeResolver() {
	PC += 1;

	switch (readOnlyAddressSpace[PC]) {
	case 0x00:
		rlc(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x01:
		rlc(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x02:
		rlc(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x03:
		rlc(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x04:
		rlc(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x05:
		rlc(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x06:
		rlc(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x07:
		rlc(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x08:
		rrc(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x09:
		rrc(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x0A:
		rrc(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x0B:
		rrc(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x0C:
		rrc(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x0D:
		rrc(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x0E:
		rrc(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x0F:
		rrc(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x10:
		rl(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x11:
		rl(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x12:
		rl(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x13:
		rl(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x14:
		rl(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x15:
		rl(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x16:
		rl(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x17:
		rl(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x18:
		rr(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x19:
		rr(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x1A:
		rr(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x1B:
		rr(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x1C:
		rr(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x1D:
		rr(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x1E:
		rr(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x1F:
		rr(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x20:
		sla(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x21:
		sla(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x22:
		sla(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x23:
		sla(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x24:
		sla(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x25:
		sla(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x26:
		sla(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x27:
		sla(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x28:
		sra(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x29:
		sra(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x2A:
		sra(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x2B:
		sra(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x2C:
		sra(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x2D:
		sra(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x2E:
		sra(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x2F:
		sra(AF.hi);
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

	case 0x38:
		srl(BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x39:
		srl(BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x3A:
		srl(DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x3B:
		srl(DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x3C:
		srl(HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x3D:
		srl(HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x3E:
		srl(addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x3F:
		srl(AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x40:
		bit(0, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x41:
		bit(0, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x42:
		bit(0, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x43:
		bit(0, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x44:
		bit(0, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x45:
		bit(0, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x46:
		bit(0, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x47:
		bit(0, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x48:
		bit(1, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x49:
		bit(1, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x4A:
		bit(1, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x4B:
		bit(1, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x4C:
		bit(1, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x4D:
		bit(1, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x4E:
		bit(1, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x4F:
		bit(1, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x50:
		bit(2, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x51:
		bit(2, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x52:
		bit(2, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x53:
		bit(2, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x54:
		bit(2, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x55:
		bit(2, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x56:
		bit(2, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x57:
		bit(2, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x58:
		bit(3, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x59:
		bit(3, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x5A:
		bit(3, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x5B:
		bit(3, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x5C:
		bit(3, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x5D:
		bit(3, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x5E:
		bit(3, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x5F:
		bit(3, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x60:
		bit(4, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x61:
		bit(4, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x62:
		bit(4, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x63:
		bit(4, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x64:
		bit(4, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x65:
		bit(4, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x66:
		bit(4, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x67:
		bit(4, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x68:
		bit(5, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x69:
		bit(5, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x6A:
		bit(5, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x6B:
		bit(5, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x6C:
		bit(5, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x6D:
		bit(5, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x6E:
		bit(5, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x6F:
		bit(5, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x70:
		bit(6, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x71:
		bit(6, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x72:
		bit(6, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x73:
		bit(6, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x74:
		bit(6, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x75:
		bit(6, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x76:
		bit(6, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x77:
		bit(6, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x78:
		bit(7, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x79:
		bit(7, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x7A:
		bit(7, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x7B:
		bit(7, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x7C:
		bit(7, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x7D:
		bit(7, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x7E:
		bit(7, readOnlyAddressSpace[HL.reg]);
		PC += 1;
		addCycles(12);
		break;

	case 0x7F:
		bit(3, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x80:
		res(0, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x81:
		res(0, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x82:
		res(0, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x83:
		res(0, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x84:
		res(0, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x85:
		res(0, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x86:
		res(0, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x87:
		res(0, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x88:
		res(1, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x89:
		res(1, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x8A:
		res(1, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x8B:
		res(1, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x8C:
		res(1, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x8D:
		res(1, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x8E:
		res(1, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x8F:
		res(1, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x90:
		res(2, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x91:
		res(2, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x92:
		res(2, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x93:
		res(2, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x94:
		res(2, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x95:
		res(2, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x96:
		res(2, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x97:
		res(2, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x98:
		res(3, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x99:
		res(3, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x9A:
		res(3, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x9B:
		res(3, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x9C:
		res(3, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0x9D:
		res(3, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0x9E:
		res(3, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0x9F:
		res(3, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA0:
		res(4, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA1:
		res(4, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xA2:
		res(4, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA3:
		res(4, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xA4:
		res(4, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA5:
		res(4, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xA6:
		res(4, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xA7:
		res(4, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA8:
		res(5, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xA9:
		res(5, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xAA:
		res(5, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xAB:
		res(5, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xAC:
		res(5, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xAD:
		res(5, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xAE:
		res(5, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xAF:
		res(5, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB0:
		res(6, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB1:
		res(6, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xB2:
		res(6, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB3:
		res(6, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xB4:
		res(6, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB5:
		res(6, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xB6:
		res(6, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xB7:
		res(6, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB8:
		res(7, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xB9:
		res(7, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xBA:
		res(7, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xBB:
		res(7, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xBC:
		res(7, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xBD:
		res(7, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xBE:
		res(7, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xBF:
		res(7, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC0:
		set(0, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC1:
		set(0, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xC2:
		set(0, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC3:
		set(0, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xC4:
		set(0, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC5:
		set(0, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xC6:
		set(0, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xC7:
		set(0, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC8:
		set(1, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xC9:
		set(1, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xCA:
		set(1, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xCB:
		set(1, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xCC:
		set(1, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xCD:
		set(1, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xCE:
		set(1, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xCF:
		set(1, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD0:
		set(2, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD1:
		set(2, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xD2:
		set(2, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD3:
		set(2, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xD4:
		set(2, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD5:
		set(2, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xD6:
		set(2, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xD7:
		set(2, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD8:
		set(3, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xD9:
		set(3, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xDA:
		set(3, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xDB:
		set(3, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xDC:
		set(3, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xDD:
		set(3, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xDE:
		set(3, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xDF:
		set(3, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE0:
		set(4, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE1:
		set(4, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xE2:
		set(4, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE3:
		set(4, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xE4:
		set(4, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE5:
		set(4, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xE6:
		set(4, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xE7:
		set(4, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE8:
		set(5, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xE9:
		set(5, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xEA:
		set(5, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xEB:
		set(5, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xEC:
		set(5, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xED:
		set(5, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xEE:
		set(5, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xEF:
		set(5, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF0:
		set(6, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF1:
		set(6, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xF2:
		set(6, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF3:
		set(6, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xF4:
		set(6, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF5:
		set(6, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xF6:
		set(6, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xF7:
		set(6, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF8:
		set(7, BC.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xF9:
		set(7, BC.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xFA:
		set(7, DE.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xFB:
		set(7, DE.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xFC:
		set(7, HL.hi);
		PC += 1;
		addCycles(8);
		break;

	case 0xFD:
		set(7, HL.lo);
		PC += 1;
		addCycles(8);
		break;

	case 0xFE:
		set(7, addressSpace[HL.reg]);
		PC += 1;
		addCycles(16);
		break;

	case 0xFF:
		set(7, AF.hi);
		PC += 1;
		addCycles(8);
		break;

	default:
		printf("Unsupported extended opcode found: PC:0x%.2x, Opcode:0xcb%.2x\n", PC, addressSpace[PC]);
		exit(1);
	}
}
