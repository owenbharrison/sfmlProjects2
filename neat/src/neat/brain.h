#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <iostream>
#include <vector>
#include <list>

#include "synapse.h"

template<size_t In, size_t Out>
struct Brain {
	static_assert(In>0&&Out>0, "brain dim must be nonzero");

	std::vector<Neuron> neurons;
	std::list<Synapse> synapses;

	void initNeurons() {
		for (size_t i=0; i<In+Out; i++) {
			auto l=i<In?0:-1;
			neurons.push_back(Neuron(l));
		}
	}

	Brain() {
		initNeurons();
		
		//initial connections like a nn
		for (size_t i=0; i<In; i++) {
			for (size_t j=0; j<Out; j++) {
				synapses.push_back(Synapse(
#ifdef SYNIX
					i, In+j
#else
					&neurons[i], &neurons[In+j]
#endif
				));
			}
		}
	}

	//crossover
	Brain(const Brain& a, const Brain& b) {
		*this=rand01()<.5f?a:b;
		tryMutate(.1f);
	}

	//lets hope this doesnt go to infinity...
	void checkPush(std::vector<Synapse*> vec) {
		std::vector<Synapse*> next;

		for (auto& sp:vec) {
#ifdef SYNIX
			int& fl=neurons[sp->from].layer, & tl=neurons[sp->to].layer;
#else
			int& fl=sp->from->layer, & tl=sp->to->layer;
#endif
			if (tl==-1) continue;
			if (tl-fl<1) {
				tl=fl+1;
				for (auto& o:synapses) {
					if (sp==&o) continue;
					if (o.from==sp->from) next.push_back(&o);
				}
			}
		}

		if (next.size()) checkPush(next);
	}

	//disable rand synapse, add two in its place
	void mutateAddNeuron() {
		auto sit=std::next(synapses.begin(), randN(synapses.size()));
		Synapse& s=*sit;

#ifdef SYNIX
		size_t end=neurons.size();
		neurons.push_back(Neuron(neurons[s.to].layer+1));
#else
		neurons.push_back(Neuron(s.to->layer+1));
		Neuron* end=&neurons.back();
#endif

		synapses.push_back(Synapse(s.from, end));
		synapses.push_back(Synapse(end, s.to));
		synapses.erase(sit);

		checkPush({&synapses.back()});
	}

	//connect two random neurons(in order)
	void mutateAddSynapse() {
#ifdef SYNIX
		size_t f;
		do f=randN(neurons.size());
		while (neurons[f].layer==-1);

		size_t t;
		do t=randN(neurons.size());
		while (f==t||neurons[t].layer!=-1&&neurons[t].layer<=neurons[f].layer);
#else
		Neuron* f;
		do f=&randElem(neurons);
		while (f->layer==-1);

		Neuron* t;
		do t=&randElem(neurons);
		while (f==t||t->layer!=-1&&t->layer<=f->layer);
#endif

		//unique?
		if (std::find_if(synapses.begin(), synapses.end(), [f, t] (const Synapse& s) {
			return s.from==f&&s.to==t;
		})==synapses.end()) synapses.push_back(Synapse(f, t));
	}

	void tryMutate(float rate) {
		if (rand01()<.7f*rate) mutateAddNeuron();
		if (rand01()<rate) mutateAddSynapse();
		for (auto& n:neurons) if (rand01()<rate) n.mutate();
		for (auto& s:synapses) if (rand01()<rate) s.mutate();
	}
};

//printing
#ifdef _OSTREAM_
template<size_t In, size_t Out>
std::ostream& operator<<(std::ostream& o, const Brain<In, Out>& b) {
	o<<"Brain[dim="<<In<<'x'<<Out<<",\n";

	o<<"\tneurons["<<b.neurons.size()<<"]=\n";
	for (size_t i=0; i<b.neurons.size(); i++) {
		o<<'\t'<<i<<": "<<b.neurons[i]<<",\n";
	}

	o<<"\tsynapses["<<b.synapses.size()<<"]=\n";
	size_t i=0;
	for (const auto& s:b.synapses) {
		o<<'\t'<<i++<<": "<<s<<",\n";
	}

	return o<<"]\n";
}
#endif
#endif