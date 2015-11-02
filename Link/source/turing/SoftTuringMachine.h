#pragma once

#include <memory>
#include <vector>

class SoftTuringMachine {
private:
	int _numMemoryLocations;
	int _memoryLocationSize;
	int _readWriteSize;

	std::vector<float> _memory;

public:
	void create(int numMemoryLocations, int memoryLocationSize, int readWriteSize)
};