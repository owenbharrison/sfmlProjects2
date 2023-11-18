#include <iostream>

#include "neat2/brain.h"

int main() {
	srand(time(NULL));

	const size_t in=3, out=1;
	Brain a(in, out), b(in, out);

	std::cout<<a<<b;

	std::cout<<"\n\n<--MUTATION-->\n\n";
	for (size_t i=0; i<3; i++) a.mutate(.02f), b.mutate(1.f);

	std::cout<<a<<b;

	return 0;
}