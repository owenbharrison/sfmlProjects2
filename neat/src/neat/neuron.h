#pragma once
#ifndef NEURON_H
#define NEURON_H

#include "randUtil.h"

struct Neuron {
	int layer=0;
	float bias=0, value=0;

	Neuron() {}

	Neuron(int l) : layer(l) {
		bias=.5f-rand01();
	}

	//every so often change the bias entirely
	inline void mutate() {
		if (rand01()<.2f) bias=.5f-rand01();
		else bias+=.1f-.2f*rand01();
	}
};

//printing
#ifdef _OSTREAM_
std::ostream& operator<<(std::ostream& o, const Neuron& n) {
	auto type=n.layer==0?"Input":n.layer==-1?"Output":"Hidden";
	return o<<"Neuron."<<type<<"[layer="<<n.layer<<", bias="<<n.bias<<']';
}
#endif
#endif