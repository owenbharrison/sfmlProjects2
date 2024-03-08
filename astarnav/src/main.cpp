#include "aStarNav.h"

int main() {
	AStarNav demo(480, 480, "A* Navigation");
	demo.setFramerateLimit(60);
	demo.run();

	return 0;
}