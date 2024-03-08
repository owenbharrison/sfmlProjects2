#include "asteroidsAIDemo.h"

int main() {
	AsteroidsAIDemo demo(400, 300, "Asteroids AI");
	demo.setFramerateLimit(500);
	demo.run();

	return 0;
}