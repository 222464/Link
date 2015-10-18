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

	std::vector<sdr::PredictiveRSDR::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 16;
	layerDescs[0]._height = 16;

	layerDescs[1]._width = 8;
	layerDescs[1]._height = 8;

	layerDescs[2]._width = 4;
	layerDescs[2]._height = 4;

	sdr::PredictiveRSDR prsdr;

	prsdr.createRandom(1, 1, layerDescs, -0.01f, 0.01f, 0.01f, 1.0f, generator);

	// ---------------------------- Game Loop -----------------------------

	sf::View view = window.getDefaultView();

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	float x = 0.0f;

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

		prsdr.setInput(0, std::sin(x));

		prsdr.simStep();

		std::cout << std::sin(x) << " " << prsdr.getPrediction(0) << std::endl;

		x += 0.5f;
		
		window.display();
	} while (!quit);

	return 0;
}