#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <sdr/IPredictiveRSDR.h>

#include <simtree/SDRST.h>

#include <array>
#include <time.h>
#include <iostream>
#include <fstream>
#include <random>

#include <assert.h>

struct Entry {
	sf::Uint8 _coarseLabel;
	sf::Uint8 _fineLabel;
	std::array<sf::Uint8, 1024> _red;
	std::array<sf::Uint8, 1024> _green;
	std::array<sf::Uint8, 1024> _blue;
};

struct STLEntry {
	std::array<sf::Uint8, 9216> _red;
	std::array<sf::Uint8, 9216> _green;
	std::array<sf::Uint8, 9216> _blue;
};

struct VecAndEntry : public Vec {
	int _index;
};

void getCIFARImage(std::ifstream &fromFile, Entry &entry, int index) {
	const int entrySize = 1 + 1 + 3072;

	fromFile.seekg(index * entrySize);

	fromFile.read(reinterpret_cast<char*>(&entry), entrySize);
}

void getSTLImage(std::ifstream &fromFile, STLEntry &entry, int index) {
	const int entrySize = 96 * 96 * 3;

	fromFile.seekg(index * entrySize);

	fromFile.read(reinterpret_cast<char*>(&entry), entrySize);
}

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 600), "Link", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	// Training
	const float sparsity = 0.1f;
	const float lff = 0.2f;
	const float ll = 0.2f;
	const float lt = 0.1f;
	const int siter = 30;
	const float sAlpha = 0.05f;
	const float sLambda = 0.4f;
	const float sHiddenDecay = 0.01f;
	const float sWeightDecay = 0.0001f;

	std::vector<sdr::IRSDR> sdrs(4);

	sdrs[0].createRandom(96, 96, 48, 48, 8, -1, -0.001f, 0.001f, generator);
	sdrs[1].createRandom(48, 48, 32, 32, 8, -1, -0.001f, 0.001f, generator);
	sdrs[2].createRandom(32, 32, 24, 24, 8, -1, -0.001f, 0.001f, generator);
	sdrs[3].createRandom(24, 24, 16, 16, 8, -1, -0.001f, 0.001f, generator);

	std::ifstream fromFile("resources/stl10/train_X.bin", std::ios::binary);

	std::uniform_int_distribution<int> entryDist(0, 4999);
	
	SDRST tree;
	tree.create(16 * 16, generator);

	std::vector<std::unique_ptr<VecAndEntry>> vaes;

	int lastIndex = 0;

	for (int iter = 0; iter < 100; iter++) {
		int index = entryDist(generator);

		STLEntry e;

		getSTLImage(fromFile, e, index);

		for (int x = 0; x < 96; x++)
			for (int y = 0; y < 96; y++) {
				int i = x + y * 96;

				float grey = 0.3333f * (e._red[i] / 255.0f + e._green[i] / 255.0f + e._blue[i] / 255.0f);

				sdrs[0].setVisibleState(i, grey);
			}

		sdrs[0].activate(siter, sAlpha, sLambda, sHiddenDecay, 0.01f, generator);
		sdrs[0].learn(lff, 0.0f, lt, sparsity, sWeightDecay, 0.1f);

		for (int l = 1; l < sdrs.size(); l++) {
			int vs = sdrs[l].getVisibleWidth() * sdrs[l].getVisibleHeight();

			for (int i = 0; i < vs; i++)
				sdrs[l].setVisibleState(i, sdrs[l - 1].getHiddenState(i));

			sdrs[l].activate(siter, sAlpha, sLambda, sHiddenDecay, 0.01f, generator);
			sdrs[l].learn(lff, 0.0f, lt, sparsity, sWeightDecay, 0.1f);
		}

		VecAndEntry vae;

		vae._index = index;
		vae._vec.resize(16 * 16);

		lastIndex = index;

		for (int i = 0; i < vae._vec.size(); i++)
			vae._vec[i] = sdrs.back().getHiddenState(i);

		vaes.push_back(std::make_unique<VecAndEntry>());

		*vaes.back() = vae;

		tree.add(vaes.back().get(), 3, generator, 1);
	}

	// Generate random SDR and reconstruct
	std::vector<float> randSDR(sdrs.back().getHiddenWidth() * sdrs.back().getHiddenHeight(), 0.0f);

	for (int i = 0; i < randSDR.size(); i++)
		//if (dist01(generator) < sparsity)
			randSDR[i] = sdrs.back().getHiddenState(i);

	std::vector<float> reconSDR = randSDR;

	for (int l = sdrs.size() - 1; l >= 0; l--) {
		std::vector<float> recon;

		sdrs[l].reconstructFeedForward(reconSDR, recon);

		if (l != 0) {
			//sdrs[l - 1].inhibit(sparsity, recon, reconSDR);
			reconSDR = recon;
		}
		else
			reconSDR = recon;
	}

	sf::Image img;

	img.create(sdrs[0].getVisibleWidth(), sdrs[0].getVisibleHeight());

	for (int x = 0; x < img.getSize().x; x++)
		for (int y = 0; y < img.getSize().y; y++) {
			sf::Uint8 comp = 255.0f * std::min(1.0f, std::max(0.0f, reconSDR[x + y * img.getSize().x]));

			img.setPixel(y, x, sf::Color(comp, comp, comp));
		}

	img.saveToFile("random.png");

	// Find closest entry
	float sim;

	//VecAndEntry* pE = static_cast<VecAndEntry*>(tree.findMostSimilar(randSDR, sim));

	STLEntry e;

	getSTLImage(fromFile, e, 0);// pE->_index);
	//std::cout << pE->_index << std::endl;
	sf::Image img2;

	img2.create(sdrs[0].getVisibleWidth(), sdrs[0].getVisibleHeight());

	for (int x = 0; x < img2.getSize().x; x++)
		for (int y = 0; y < img2.getSize().y; y++) {
			int i = x + y * img2.getSize().x;

			img2.setPixel(y, x, sf::Color(e._red[i], e._green[i], e._blue[i]));
		}

	img2.saveToFile("random_mostSim.png");

	// ---------------------------- Game Loop -----------------------------

	sf::View view = window.getDefaultView();

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	do {
		clock.restart();

		// ----------------------------- Input -----------------------------

		sf::Event windowEvent;

		while (window.pollEvent(windowEvent)) {
			switch (windowEvent.type) {
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			quit = true;

		window.clear();



		window.display();
	} while (!quit);

	return 0;
}
