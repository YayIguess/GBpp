#include <string>
#include <filesystem>
#include <vector>
#include "3rdParty/json.hpp"
#include "gameboy.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void runJSONTests(GameBoy* gb);

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <bios> <game>" << std::endl;
		return -1;
	}

	auto* gb = new GameBoy();
	gb->SDL2setup();
	//runJSONTests(gb);
	gb->start(argv[1], argv[2]);
	gb->SDL2destroy();
	delete gb;

	return 0;
}

void runJSONTests(GameBoy* gb) {
	std::string path = "../tests/sm83/v1";
	std::vector<std::string> testFiles;
	int failed = 0;
	for (const auto& entry : fs::directory_iterator(path))
		testFiles.emplace_back(entry.path());


	for (const auto& testFile : testFiles) {
		std::ifstream file(testFile);
		std::cout << "Running test: " << testFile << std::endl;
		const json tests = json::parse(file);

		for (auto& test : tests) {
			//create state
			std::vector<std::tuple<Word, Byte>> initialRAM;
			for (int i = 0; i < test["initial"]["ram"].size(); i++)
				initialRAM.emplace_back(test["initial"]["ram"][i][0], test["initial"]["ram"][i][1]);

			GameboyTestState initialState = {
				test["initial"]["pc"],
				test["initial"]["sp"],
				test["initial"]["a"],
				test["initial"]["f"],
				test["initial"]["b"],
				test["initial"]["c"],
				test["initial"]["d"],
				test["initial"]["e"],
				test["initial"]["h"],
				test["initial"]["l"],
				initialRAM
			};

			//run
			GameboyTestState result = gb->runTest(initialState);

			//compare new state to expected
			std::vector<std::tuple<Word, Byte>> finalRAM;
			for (int i = 0; i < test["final"]["ram"].size(); i++)
				finalRAM.emplace_back(test["final"]["ram"][i][0], test["final"]["ram"][i][1]);

			GameboyTestState finalState = {
				test["final"]["pc"],
				test["final"]["sp"],
				test["final"]["a"],
				test["final"]["f"],
				test["final"]["b"],
				test["final"]["c"],
				test["final"]["d"],
				test["final"]["e"],
				test["final"]["h"],
				test["final"]["l"],
				finalRAM
			};

			if (finalState != result) {
				std::cout << "Test " << testFile << " failed!" << std::endl;
				failed += 1;
				break;
			}
		}
	}
	if (!failed)
		std::cout << "Success!" << std::endl;
	else
		std::cout << failed << "/" << testFiles.size() << " failed!" << std::endl;
}
