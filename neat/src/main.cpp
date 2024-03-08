#include <iostream>

#include "neat/brain2.h"

#include "io/stopwatch.h"
using namespace common;

int main() {
	srand(time(NULL));

	const size_t in=64, out=16;
	Brain<in, out> b;

	std::cout<<"B: "<<b;

	std::cout<<"\nMUTATION\n\n";
	for (size_t i=0; i<3; i++) b.tryMutate(.02f);

	std::cout<<"B: "<<b;

	b.prepare();
	Stopwatch propWatch; propWatch.start();
	for (size_t i=0; i<1; i++) {
		b.propagate();
	}
	propWatch.stop();
	std::cout<<"Propagation Time: "<<propWatch.getMicros()<<"us\n";

	return 0;
}