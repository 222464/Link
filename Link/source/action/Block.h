#pragma once

#include <string>

#include <sdr/PredictiveRSDR.h>

namespace action {
	class Block {
	private:
		std::string _code;
		std::vector<float> _sdr;

	public:
		// create from code string
		void create(sdr::PredictiveRSDR& prsdr, const std::string& code, int numIterations);
		// create form code file
		void createFromFile(sdr::PredictiveRSDR& prsdr, const std::string& path, int numIterations);

		// gets the block's matching SDR
		std::vector<float> getSDR() const;

		// executes the block's code
		bool execute();
	};
}