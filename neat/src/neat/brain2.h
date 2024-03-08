#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <list>

#include <random>

std::mt19937_64 generator(time(NULL));

inline float rand01() {
	static std::uniform_real_distribution<float> dist(0, 1);
	return dist(generator);
}

template<typename T>
auto randIt(std::list<T>& arr) {
	static std::uniform_int_distribution<size_t> dist(0, arr.size()-1);
	return std::next(arr.begin(), dist(generator));
}

//forward def for recursivity
struct Neuron;
struct Synapse {
	Neuron* to=nullptr;
	float weight=0;

	Synapse() {}

	Synapse(Neuron* a) : to(a) {
		weight=.5f-rand01();
	}

	inline void mutate() {
		if (rand01()<.1f) weight=.5f-rand01();
		else weight+=.1f-.2f*rand01();
	}
};

struct Neuron {
	int layer=0;
	float bias=0, value=0;
	std::list<Synapse> synapses;

	Neuron() {}

	Neuron(int l) : layer(l) {
		bias=.5f-rand01();
	}

	inline void mutate() {
		if (rand01()<.1f) bias=.5f-rand01();
		else bias+=.1f-.2f*rand01();
	}

	void checkLayerOverlap() {
		for (auto& s:synapses) {
			auto& lh=layer, & rh=s.to->layer;

			if (rh!=-1&&rh-lh<1) {
				rh=lh+1;

				s.to->checkLayerOverlap();
			}
		}
	}
};

template<size_t In, size_t Out>
struct Brain {
	std::list<Neuron> neurons;
	Neuron* inputs[In], * outputs[Out];

	static inline float reLu(float x) {
		return x<0?0:x;
	}

	static inline float leakyReLu(float x) {
		return x*(x<0?.1f:1);
	}

	static inline float sigmoid(float x) {
		return 1/(1+expf(-x));
	}

	//relu, sigmoid, arctan, tanhf...
	float(*activation)(float)=sigmoid;

	Brain() {
		for (size_t i=0; i<In; i++) {
			neurons.push_back(Neuron(0));
			inputs[i]=&neurons.back();
		}
		for (size_t i=0; i<Out; i++) {
			neurons.push_back(Neuron(-1));
			outputs[i]=&neurons.back();
		}

		for (auto i:inputs) {
			for (auto o:outputs) {
				i->synapses.push_back(Synapse(o));
			}
		}
	}

	//crossover (not really)
	Brain(const Brain& a, const Brain& b, float rate=.05f) {
		*this=rand01()<.5f?a:b;
		tryMutate(rate);
	}

	void mutateAddNode() {
	//rand syn
		std::list<Neuron>::iterator from;
		do from=randIt(neurons);
		while (from->layer==-1);
		auto syn=randIt(from->synapses);

		//add neuron in the middlde
		neurons.push_back(Neuron(from->layer+1));
		auto mid=&neurons.back();

		//connect it up, remove5 old
		from->synapses.push_back(Synapse(mid));
		mid->synapses.push_back(Synapse(syn->to));
		from->synapses.erase(syn);

		mid->checkLayerOverlap();
	}

	void mutateAddSynapse() {
	//rand non-out
		std::list<Neuron>::iterator f;
		do f=randIt(neurons);
		while (f->layer==-1);

		//rand "next"
		std::list<Neuron>::iterator t;
		do t=randIt(neurons);
		while (t->layer!=-1&&t->layer-f->layer<1);

		//unique?
		Neuron* tp=&*t;
		if (std::find_if(f->synapses.begin(), f->synapses.end(), [tp] (const Synapse& s) {
			return tp==s.to;
		})==f->synapses.end()) f->synapses.push_back(Synapse(tp));
	}

	void tryMutate(float rate) {
		if (rand01()<.7f*rate) mutateAddNode();
		if (rand01()<rate) mutateAddSynapse();

		for (auto& n:neurons) {
			if (rand01()<rate) n.mutate();

			for (auto& s:n.synapses) {
				if (rand01()<rate) n.mutate();
			}
		}
	}

	//get ready for batch propagation
	void prepare() {
		neurons.sort([] (const Neuron& a, const Neuron& b) {
			if (a.layer==-1&&b.layer!=-1) return false;
			if (a.layer!=-1&&b.layer==-1) return true;
			return a.layer<b.layer;
		});
	}

	//requires individual input set first.
	void propagate() {
	//reset all non inputs
		for (auto& n:neurons) {
			if (n.layer!=0) n.value=0;
		}

		int prevLayer=0;
		for (auto it=neurons.begin(); it!=neurons.end(); it++) {
			auto& n=*it;
			if (n.layer!=prevLayer) {
			//apply activation to entire layer...
				for (auto sit=std::next(it); sit!=neurons.end(); sit++) {
					auto& ns=*sit;
					if (ns.layer!=n.layer) break;

					ns.value=activation(ns.value+ns.bias);
				}
			}

			//propagate
			for (auto& s:n.synapses) {
				s.to->value+=s.weight*n.value;
			}

			prevLayer=n.layer;
		}
	}
};

#include <iostream>

#ifdef _OSTREAM_
template<size_t In, size_t Out>
std::ostream& operator<<(std::ostream& o, const Brain<In, Out>& b) {
	/* ideal printing pattern...
	1 B[d=3x1, #n=6:
	2		0x1e3: N[l=0, b=-.67, #s=2:
	3			S[->0x671, w=.174],
	4			S[->0xe91, w=.91]
	5		],
	6		0xe7a: N[l=0, b=.13, #s=1:
	7			S[->0x8e1, w=.123]
	8		],
	9		0x90c: N[l=0, b=-.19, #s=0]
	a ]*/

	//brain header w/ opt args
	size_t nNeu=b.neurons.size();
	o<<"B[d="<<In<<'x'<<Out<<", #n="<<nNeu;
	if (nNeu) {
		o<<":\n";

		const auto nLast=std::prev(b.neurons.end());
		for (auto nit=b.neurons.begin(); nit!=b.neurons.end(); nit++) {
			const auto& n=*nit;

			//neuron header w/ opt args
			size_t nSyn=n.synapses.size();
			auto nRelIx=std::distance(b.neurons.begin(), nit);
			o<<'\t'<<nRelIx<<": N[l="<<n.layer<<", b="<<n.bias<<", #s="<<nSyn;
			if (nSyn) {
				o<<":\n";

				//for all synapses
				const auto sLast=std::prev(n.synapses.end());
				for (auto sit=n.synapses.begin(); sit!=n.synapses.end(); sit++) {
					const auto& s=*sit;

					auto tp=s.to;
					auto tit=std::find_if(b.neurons.begin(), b.neurons.end(), [tp] (const Neuron& n) {
						return &n==tp;
					});
					auto tRelIx=std::distance(b.neurons.begin(), tit);
					o<<"\t\t"<<"S[->"<<tRelIx<<", w = "<<s.weight<<']';

					//list opt comma
					if (sit!=sLast) o<<',';
					o<<'\n';
				}
				o<<'\t';
			}
			o<<']';

			//list opt comma
			if (nit!=nLast) o<<',';
			o<<'\n';
		}
	}
	return o<<"]\n";
}
#endif
#endif