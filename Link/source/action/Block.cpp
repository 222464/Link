#include <action/Block.h>

#include <fstream>

#include <duktape/duktape.h>

#include <action/ActionFunctions.h>

void action::Block::create(const std::string& code, const std::vector<sdr::PredictiveRSDR::LayerDesc>& layerDescs, float initMinWeight, float initMaxWeight, float initMinInhibition, float initMaxInhibition, std::mt19937& generator, int numLearnIterations) {
	_code = code;

	std::string possibleChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+`-=[]\\{}|;':\",./<>? \r\n";
	// wah means width and height, in this case the sdr input is a square so they are the same
	int wah = std::ceil(std::sqrt(possibleChars.length()));

	_prsdr.createRandom(wah, wah, layerDescs, initMinWeight, initMaxWeight, initMinInhibition, initMaxInhibition, generator);

	int numInputs = wah * wah;

	for (int t = 0; t < numLearnIterations; t++) {
		for (int q = 0; q < _code.length(); q++) {
			for (int i = 0; i < numInputs; i++) {
				_prsdr.setInput(i, 0.0f);
			}

			_prsdr.setInput(possibleChars[possibleChars.find_first_of(_code[q])], 1.0f);

			_prsdr.simStep();
		}
	}
}

void action::Block::createFromFile(const std::string& path, const std::vector<sdr::PredictiveRSDR::LayerDesc>& layerDescs, float initMinWeight, float initMaxWeight, float initMinInhibition, float initMaxInhibition, std::mt19937& generator, int numLearnIterations) {
	std::ifstream ifs(path);
	_code.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	ifs.close();

	std::string possibleChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+`-=[]\\{}|;':\",./<>? \r\n";
	// wah means width and height, in this case the sdr input is a square so they are the same
	int wah = std::ceil(std::sqrt(possibleChars.length()));

	_prsdr.createRandom(wah, wah, layerDescs, initMinWeight, initMaxWeight, initMinInhibition, initMaxInhibition, generator);

	int numInputs = wah * wah;

	for (int t = 0; t < numLearnIterations; t++) {
		for (int q = 0; q < _code.length(); q++) {
			for (int i = 0; i < numInputs; i++) {
				_prsdr.setInput(i, 0.0f);
			}

			_prsdr.setInput(possibleChars[possibleChars.find_first_of(_code[q])], 1.0f);

			_prsdr.simStep();
		}
	}
}

bool action::Block::execute() {
	duk_context* ctx = duk_create_heap_default();
	if (!ctx)
		return false;

	registerAllFunctions(ctx);

	duk_eval_string(ctx, _code.c_str());

	duk_destroy_heap(ctx);

	return true;
}
