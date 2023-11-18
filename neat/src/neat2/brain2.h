#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <vector>

inline float rand01() {
	return float(rand())/RAND_MAX;
}

template<typename T>
T& randElem(std::vector<T>& arr) {
	size_t i=arr.size()*rand01();
	return arr[i%arr.size()];
}

struct Brain {
	size_t in=0, out=0;
	std::vector<int> somas;

	struct Axon {
		size_t a=0, b=0;
		float weight=0;
		bool enabled=true;
	};
	std::vector<Axon> axons;

	Brain() {}

	Brain(size_t i, size_t o) : in(), out(o) {
		for (size_t i=0; i<in+out; i++) {
			somas.push_back((i<in)-1);
		}

		for (size_t i=0; i<in; i++) {
			for (size_t j=0; j<out; j++) {
				axons.push_back({i, in+j, rand01()});
			}
		}
	}

	void checkLayerOverlap() {
		bool found;
		do {
			found=false;
			for (auto& a:axons) {
				int& la=somas[a.a], & lb=somas[a.b];
				if (lb!=-1&&lb-la<1) lb=la+1, found=true;
			}
		} while (found);
	}

	void mutateAddSoma() {
		Axon* a;
		do a=&randElem(axons);
		while (!a->enabled);

		a->enabled=false;

		size_t e=a->b, i=somas.size();
		somas.push_back(somas[e]+1);

		axons.push_back({a->a, i, rand01()});
		axons.push_back({i, e, rand01()});

		checkLayerOverlap();
	}
};

//print overloads
#ifdef _OSTREAM_

#endif
#endif