#ifndef TESTING_HPP
#define TESTING_HPP

#include <cstdint>
#include <vector>
#include <tuple>
#include <iostream>
#include <stdexcept>
#include <string>
#include "defines.hpp"

struct GameboyTestState {
	Word PC;
	Word SP;
	Byte A;
	Byte F;
	Byte B;
	Byte C;
	Byte D;
	Byte E;
	Byte H;
	Byte L;
	//Byte IME;
	//Byte IE;
	std::vector<std::tuple<Word, Byte>> RAM;
};

inline bool operator==(const GameboyTestState& lhs, const GameboyTestState& rhs) {
	for (int i = 0; i < lhs.RAM.size(); i++) {
		if (std::get<1>(lhs.RAM[i]) != std::get<1>(rhs.RAM[i]))
			return false;
	}
	return (lhs.A == rhs.A &&
		lhs.F == rhs.F &&
		lhs.B == rhs.B &&
		lhs.C == rhs.C &&
		lhs.D == rhs.D &&
		lhs.E == rhs.E &&
		lhs.H == rhs.H &&
		lhs.L == rhs.L);
}

#endif //TESTING_HPP
