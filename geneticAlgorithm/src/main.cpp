#include <iostream>

#include "population.h"

/*
int main() {
	srand(time(NULL));

	Population p("unicorn", 600, .02f);

	for (size_t n=0; n<350; n++) {
		p.calcFitness();

		std::string best;
		float record=0;
		for (size_t i=0; i<p.num; i++) {
			auto& d=p.population[i];
			if (d.fitness>record) {
				record=d.fitness;
				best=std::string(d.genes, d.num);
			}
		}
		std::cout<<"Best: "<<best<<'\n';

		p.generate();
	}

	return 0;
}*/	

int main() {
	srand(time(NULL));

	Population p("The quick brown fox jumped over the lazy dog", 500, 0.01f);
	for (size_t k=0; k<2500; k++) {
		std::cout<<k<<": ";

		std::string best;
		float record=0;
		for (size_t i=0; i<p.num; i++) {
			auto& d=p.population[i];
			d.calcFitness(p.target);

			if (d.fitness>record) {
				record=d.fitness;
				best=std::string(d.genes, d.num);
			}
		}
		std::cout<<"best: "<<best<<'\n';
		
		if (best==p.target) break;

		p.generate();
	}

	return 0;
}