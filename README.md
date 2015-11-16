# Link
Link text to commands and generated programs.
Basic sequence learning algorithms are implemented so far.

## Build
You will need SFML2. On debian/ubuntu-like linuxes it can be installed with
```
sudo apt-get install libsfml-dev libsfml-system2 libsfml-window2 libsfml-graphics2
```
or you can install SFML from source http://www.sfml-dev.org/download/sfml/2.1/

Then compile the demos
```
cd /path/to/Link/Link
cmake .
make -j4
```

## Demos

Demos require datasets that are not in this repository.

For experiment-mnistvideo:
```
cd path/to/Link/Link/resources
wget http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz
gunzip train-images-idx3-ubyte.gz
```

For experiment-timeseries:
```
cd path/to/Link/Link/resources
wget -O laser.txt http://www-psych.stanford.edu/~andreas/Time-Series/SantaFe/A.dat
```

For experiment-textprediction place any text file to resources/corpus.txt


Happy hacking!
