#include "neuralNetDemo.h"

int main() {
	NeuralNetDemo demo(640, 480, "Neural Net");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}