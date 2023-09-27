#include "neuralNetDemo.h"

int main() {
	NeuralNetDemo demo(640, 480, "NN Testing");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}