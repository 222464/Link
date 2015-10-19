#pragma once

#include <string>

#include <sdr/PredictiveRSDR.h>

namespace action {
	class Block {
	public:
		std::string _code;
		sdr::PredictiveRSDR _prsdr;

		// assigns the given code to this block and generates an sdr from it
		void create(const std::string& code, const std::vector<sdr::PredictiveRSDR::LayerDesc>& layerDescs, float initMinWeight, float initMaxWeight, float initMinInhibition, float initMaxInhibition, std::mt19937& generator, int numLearnIterations);
		// assigns the code in the given file and generates an sdr from it
		void createFromFile(const std::string& path, const std::vector<sdr::PredictiveRSDR::LayerDesc>& layerDescs, float initMinWeight, float initMaxWeight, float initMinInhibition, float initMaxInhibition, std::mt19937& generator, int numLearnIterations);

		// executes the block's code
		bool execute();
	};
}