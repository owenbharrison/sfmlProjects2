/*remove SYNIX in final impl
too indecisive :C*/

#pragma once
#ifndef SYNAPSE_H
#define SYNAPSE_H

#include "neuron.h"

//#define SYNIX
struct Synapse {
#ifdef SYNIX
	size_t from=0, to=0;
#else
	Neuron* from=nullptr, * to=nullptr;
#endif
	float weight=0;

	Synapse() {};

	Synapse(
#ifdef SYNIX
		size_t f, size_t t
#else
		Neuron* f, Neuron* t
#endif
	) : from(f), to(t) {
		weight=.5f-rand01();
	}

	//every so often change the weight entirely
	inline void mutate() {
		if (rand01()<.2f) weight=.5f-rand01();
		else weight+=.1f-.2f*rand01();
	}
};

//printing
#ifdef _OSTREAM_
std::ostream& operator<<(std::ostream& o, const Synapse& s) {
	return o<<"Synapse["<<s.from<<"->"<<s.to<<", weight="<<s.weight<<']';
}
#endif
#endif