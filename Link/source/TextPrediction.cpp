#include "Settings.h"

#if EXPERIMENT_SELECTION == EXPERIMENT_TEXT_PREDICTION

#include "Settings.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <sdr/PredictiveRSDR.h>

#include <simtree/SDRST.h>

#include <time.h>
#include <iostream>
#include <random>

#include <duktape/duktape.h>

#include <assert.h>

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 600), "Link", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	std::vector<std::vector<float>> testVecs(16);

	for (int i = 0; i < testVecs.size(); i++) {
		testVecs[i].resize(4);

		for (int j = 0; j < 4; j++)
			testVecs[i][j] = static_cast<float>((i >> j) & 0x00000001);;
	}

	std::vector<float> searchVec = { 1, 1, 0, 1 };

	std::vector<Vec> vecs(testVecs.size());

	for (int i = 0; i < testVecs.size(); i++)
		vecs[i]._vec = testVecs[i];

	SDRST tree;

	tree.create(4, generator);

	for (int i = 0; i < testVecs.size(); i++)
		tree.add(&vecs[i], 2, generator, 1);

	float sim;

	Vec* pSim = tree.findMostSimilar(searchVec, sim);

	assert(pSim != nullptr);

	for (int i = 0; i < pSim->_vec.size(); i++)
		std::cout << pSim->_vec[i] << " ";

	std::cout << "Sim: " << sim << std::endl;

	std::string test = "I once stated: Hold there, nobel knight, ye shall not cross this bridge. Then came a rebutal: You shallt not hinder my progression through this kingdom. Thou cannot prohibit my forward motion!";

	int minimum = 255;
	int maximum = 0;

	for (int i = 0; i < test.length(); i++) {
		minimum = std::min(static_cast<int>(test[i]), minimum);
		maximum = std::max(static_cast<int>(test[i]), maximum);
	}

	int numInputs = maximum - minimum + 1;

	int inputsRoot = std::ceil(std::sqrt(static_cast<float>(numInputs)));

	std::vector<sdr::PredictiveRSDR::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 16;
	layerDescs[0]._height = 16;

	layerDescs[1]._width = 12;
	layerDescs[1]._height = 12;

	layerDescs[2]._width = 8;
	layerDescs[2]._height = 8;

	sdr::PredictiveRSDR prsdr;

	prsdr.createRandom(inputsRoot, inputsRoot, 16, layerDescs, -0.01f, 0.01f, 0.01f, 0.02f, 0.1f, generator);

	// ---------------------------- Game Loop -----------------------------

	sf::View view = window.getDefaultView();

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	int current = 0;

	float averageError = 0.0f;

	sf::Font font;
	if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf"))
		return 1;

	sf::Text avgText;
	avgText.setColor(sf::Color::Red);
	avgText.setFont(font);
	avgText.setPosition(sf::Vector2f(100.0f, 100.0f));

	do {
		clock.restart();

		// ----------------------------- Input -----------------------------

		sf::Event windowEvent;

		while (window.pollEvent(windowEvent))
		{
			switch (windowEvent.type)
			{
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			quit = true;

		
		if (current == 0) {
			window.clear();
			
			window.draw(avgText);

			window.display();
		}

		for (int i = 0; i < inputsRoot * inputsRoot; i++)
			prsdr.setInput(i, 0.0f);

		int index = test[current] - minimum;

		prsdr.setInput(index, 1.0f);

		prsdr.simStep();

		int predIndex = 0;

		for (int i = 1; i < numInputs; i++)
			if (prsdr.getPrediction(i) > prsdr.getPrediction(predIndex))
				predIndex = i;

		char predChar = predIndex + minimum;

		std::cout << predChar;

		current = (current + 1) % test.length();

		float error = 1.0f;

		if (predChar == test[current])
			error = 0.0f;

		//std::cout << predChar << " " << test[current] << std::endl;

		averageError = 0.99f * averageError + 0.01f * error;
		avgText.setString("Avg Err: " + std::to_string(averageError));

		if (current == 0)
			std::cout << "\n";
	} while (!quit);

	return 0;
}

#endif