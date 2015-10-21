#include <action/Block.h>

#include <fstream>

#include <duktape/duktape.h>

#include <action/ActionFunctions.h>

void action::Block::create(sdr::PredictiveRSDR& prsdr, const std::string& code, int numLearnIterations) {
	_code = code;

	std::string possibleChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+`-=[]\\{}|;':\",./<>? \r\n";

	int numInputs = prsdr.getLayers()[0]._sdr.getVisibleWidth() * prsdr.getLayers()[0]._sdr.getVisibleHeight();

	for (int t = 0; t < numLearnIterations; t++) {
		for (int q = 0; q < _code.length(); q++) {
			for (int i = 0; i < numInputs; i++) {
				prsdr.setInput(i, 0.0f);
			}

			prsdr.setInput(possibleChars[possibleChars.find_first_of(_code[q])], 1.0f);

			prsdr.simStep();
		}
	}
}

void action::Block::createFromFile(sdr::PredictiveRSDR& prsdr, const std::string& path, int numLearnIterations) {
	std::ifstream ifs(path);
	_code.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	ifs.close();

	std::string possibleChars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+`-=[]\\{}|;':\",./<>? \r\n";

	int numInputs = prsdr.getLayers()[0]._sdr.getVisibleWidth() * prsdr.getLayers()[0]._sdr.getVisibleHeight();

	for (int t = 0; t < numLearnIterations; t++) {
		for (int q = 0; q < _code.length(); q++) {
			for (int i = 0; i < numInputs; i++) {
				prsdr.setInput(i, 0.0f);
			}

			prsdr.setInput(possibleChars[possibleChars.find_first_of(_code[q])], 1.0f);

			prsdr.simStep();
		}
	}
}

std::vector<float> action::Block::getSDR() const {
	return _sdr;
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
