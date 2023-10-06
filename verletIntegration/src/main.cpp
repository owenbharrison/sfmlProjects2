#include "verletIntegrationDemo.h"

int main() {
	VerletIntegrationDemo demo(720, 480, "Verlet Integration");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}