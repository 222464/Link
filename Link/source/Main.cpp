#include "Settings.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <sdr/PredictiveRSDR.h>

#include <time.h>
#include <iostream>
#include <random>

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 600), "Link", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	std::string test = "This is a string to test the prediction capabilities of PredictiveRSDR.";

	int minimum = 255;
	int maximum = 0;

	for (int i = 0; i < test.length(); i++) {
		minimum = std::min(static_cast<int>(test[i]), minimum);
		maximum = std::max(static_cast<int>(test[i]), maximum);
	}

	int numInputs = maximum - minimum;

	int inputsRoot = std::ceil(std::sqrt(static_cast<float>(numInputs)));

	std::vector<sdr::PredictiveRSDR::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 16;
	layerDescs[0]._height = 16;

	layerDescs[1]._width = 12;
	layerDescs[1]._height = 12;

	layerDescs[2]._width = 8;
	layerDescs[2]._height = 8;

	sdr::PredictiveRSDR prsdr;

	prsdr.createRandom(inputsRoot, inputsRoot, layerDescs, -0.001f, 0.001f, 0.01f, 0.05f, generator);

	// ---------------------------- Game Loop -----------------------------

	sf::View view = window.getDefaultView();

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	int current = 0;

	float averageError = 0.0f;

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

		window.clear();

		for (int i = 0; i < numInputs; i++)
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

		if (current == 0)
			std::cout << "Average Error: " << averageError << std::endl;
		
		//window.display();
	} while (!quit);

	return 0;
}