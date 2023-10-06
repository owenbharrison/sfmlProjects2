#include "asteroidsDemo.h"

int main() {
	AsteroidsDemo demo(640, 480, "Asteroids Game");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}