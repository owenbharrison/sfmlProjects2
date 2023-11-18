#include "gameAIDemo.h"

int main() {
	GameAIDemo demo(640, 480, "Game AI");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}