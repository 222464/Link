#pragma once

#include <string>

#include <sdr/IPredictiveRSDR.h>

namespace action {
	class Block {
	private:
		std::string _code;
		std::vector<float> _sdr;

	public:
		// create from code string
		void create(sdr::IPredictiveRSDR& prsdr, const std::string& code, int numIterations);
		// create form code file
		void createFromFile(sdr::IPredictiveRSDR& prsdr, const std::string& path, int numIterations);

		// gets the block's matching SDR
		std::vector<float> getSDR() const;

		// executes the block's code
		bool execute();
	};
}