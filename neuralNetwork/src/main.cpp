#include "neuralNetDemo.h"

int main() {
	NeuralNetDemo demo(240, 180, "Neural Net");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}