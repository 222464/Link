#include "Settings.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <sdr/PredictiveRSDR.h>

#include <time.h>
#include <iostream>
#include <random>

#include <complex>
#include <iostream>
#include <valarray>
#include <vector>

const double PI = 3.141592653589793238460;

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
/*void fft(CArray& x)
{
	const size_t N = x.size();
	if (N <= 1) return;

	// divide
	CArray even = x[std::slice(0, N / 2, 2)];
	CArray  odd = x[std::slice(1, N / 2, 2)];

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (size_t k = 0; k < N / 2; ++k)
	{
		Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k + N / 2] = even[k] - t;
	}
}*/

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// Better optimized but less intuitive
void fft(CArray &x)
{
	// DFT
	unsigned int N = x.size(), k = N, n;
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), sin(thetaT)), T;
	while (k > 1)
	{
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; l++)
		{
			for (unsigned int a = l; a < N; a += n)
			{
				unsigned int b = a + k;
				Complex t = x[a] - x[b];
				x[a] += x[b];
				x[b] = t * T;
			}
			T *= phiT;
		}
	}
	// Decimate
	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; a++)
	{
		unsigned int b = a;
		// Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a)
		{
			Complex t = x[a];
			x[a] = x[b];
			x[b] = t;
		}
	}
}

// inverse fft (in-place)
void ifft(CArray& x)
{
	// conjugate the complex numbers
	x = x.apply(std::conj);

	// forward fft
	fft(x);

	// conjugate the complex numbers again
	x = x.apply(std::conj);

	// scale the numbers
	x /= x.size();
}

/*int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 600), "Link", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	std::string test = "'Till I have no wife, I have nothing in France.'\n"
		"Nothing in France, until he has no wife!\n"
		"Thou shalt have none, Rousillon, none in France;";

	int minimum = 255;
	int maximum = 0;

	for (int i = 0; i < test.length(); i++) {
		minimum = std::min(static_cast<int>(test[i]), minimum);
		maximum = std::max(static_cast<int>(test[i]), maximum);
	}

	int numInputs = maximum - minimum + 1;

	int inputsRoot = std::ceil(std::sqrt(static_cast<float>(numInputs)));

	std::vector<sdr::PredictiveRSDR::LayerDesc> layerDescs(2);

	layerDescs[0]._width = 32;
	layerDescs[0]._height = 32;

	layerDescs[1]._width = 32;
	layerDescs[1]._height = 32;

	sdr::PredictiveRSDR prsdr;

	prsdr.createRandom(inputsRoot, inputsRoot, layerDescs, -0.01f, 0.01f, 0.01f, 0.05f, generator);

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

		window.clear();

		if (current == 0) {
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
}*/

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 600), "Link", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	sf::SoundBuffer buff;

	buff.loadFromFile("resources/testSound.wav");

	int sampleSize = 1024;

	int sampleIncrement = 1024;

	int inputsRoot = std::ceil(std::sqrt(static_cast<float>(sampleSize)));

	std::vector<sdr::PredictiveRSDR::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 64;
	layerDescs[0]._height = 64;

	layerDescs[1]._width = 32;
	layerDescs[1]._height = 32;

	layerDescs[2]._width = 16;
	layerDescs[2]._height = 16;

	sdr::PredictiveRSDR prsdr;

	prsdr.createRandom(inputsRoot, inputsRoot, layerDescs, -0.01f, 0.01f, 0.01f, 0.05f, generator);

	CArray samplesTemp(sampleSize);

	for (int soundIter = 0; soundIter < 6; soundIter++) {
		for (int si = 0; si < buff.getSampleCount() - sampleSize; si += sampleIncrement) {
			for (int bi = 0; bi < sampleSize; bi++) {
				samplesTemp[bi] = Complex(static_cast<double>(buff.getSamples()[si + bi]) / std::numeric_limits<unsigned short>::max());
			}

			//fft(samplesTemp);

			for (int i = 0; i < samplesTemp.size(); i++)
				prsdr.setInput(i, samplesTemp[i].real());

			prsdr.simStep();

			if (si % sampleSize * 4 == 0) {
				std::cout << "Sample: " << si << std::endl;
			}
		}
	}

	std::vector<float> generatedSamplesf(buff.getSampleCount(), 0.0f);
	std::vector<float> generatedCountsf(buff.getSampleCount(), 0.0f);

	for (int si = 0; si < buff.getSampleCount() - sampleSize; si += sampleIncrement) {

		for (int i = 0; i < samplesTemp.size(); i++) {
			samplesTemp[i] = prsdr.getPrediction(i);
			prsdr.setInput(i, prsdr.getPrediction(i));
		}

		//ifft(samplesTemp);

		for (int bi = 0; bi < sampleSize; bi++) {
			generatedSamplesf[si + bi] += samplesTemp[bi].real();
			generatedCountsf[si + bi]++;
		}

		prsdr.simStep();

		if (si % sampleSize * 4 == 0) {
			std::cout << "Sample: " << si << std::endl;
		}
	}

	std::vector<sf::Int16> generatedSamples(generatedSamplesf.size());

	for (int i = 0; i < generatedSamplesf.size(); i++)
		generatedSamples[i] = static_cast<sf::Int16>(generatedSamplesf[i] / std::max(1.0f, generatedCountsf[i]) * std::numeric_limits<sf::Int16>::max());

	sf::SoundBuffer generated;
	generated.loadFromSamples(generatedSamples.data(), generatedSamples.size(), 1, buff.getSampleRate());

	generated.saveToFile("generated.wav");

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

		window.clear();

		window.display();

	} while (!quit);

	return 0;
}