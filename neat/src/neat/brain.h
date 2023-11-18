#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <vector>
#include <algorithm>

inline float rand01() {
	return float(rand())/RAND_MAX;
}

struct Brain {
	size_t numIn=0, numOut=0;
	struct Neuron {
		enum Type { Input=0, Hidden, Output } type;
		int layer;

		Neuron() : type(Hidden), layer(0) {}

		Neuron(Type t, int l) : type(t), layer(l) {}
	};
	std::vector<Neuron> neurons;

	struct Synapse {
		size_t a=0, b=0;
		float weight=0;
		bool enabled=true;

		Synapse() : a(0), b(0), weight(0) {}

		Synapse(size_t a, size_t b, float w) : a(a), b(b), weight(w) {}

		//every so often change the weight entirely
		void mutate(float rate) {
			if (rand01()<rate) weight=rand01();
			else weight+=.2f*rand01();
		}

		bool operator<(const Synapse& s) const {
			if (a==s.a) return b<s.b;
			return a<s.a;
		}
	};
	std::vector<Synapse> synapses;

	//start out like a one layer nn
	Brain(size_t in, size_t out) : numIn(in), numOut(out) {
		for (size_t i=0; i<numIn+numOut; i++) {
			bool b=i<numIn;
			Neuron n(b?Neuron::Input:Neuron::Output, b-1);
			neurons.push_back(n);
		}

		for (size_t i=0; i<numIn; i++) {
			for (size_t j=0; j<numOut; j++) {
				Synapse s(i, numIn+j, rand01());
				synapses.push_back(s);
			}
		}
	}

	//topological transformations
	void mutateAddNode() {
		//random enabled synapse
		Synapse* s;
		do s=&synapses[rand()%synapses.size()];
		while (!s->enabled);

		//disable
		s->enabled=false;

		//new node
		neurons.push_back(Neuron(Neuron::Hidden, neurons[s->a].layer+1));

//TODO: err only in debug mode??(2nd synapse[b]=inf)?
		//replace with 2 synapses
		const size_t i=neurons.size()-1;
		synapses.push_back(Synapse(s->a, i, rand01()));
		synapses.push_back(Synapse(i, s->b, rand01()));
	}

	void mutateAddSynapse() {
		//random non-output neuron
		size_t a, b;
		do a=rand()%neurons.size();
		while (neurons[a].layer==-1);

		//random "consecutive" neuron
		do b=rand()%neurons.size();
		while (neurons[b].layer!=-1&&neurons[b].layer>neurons[a].layer);

		//if it already exists, forget it.
		if (std::find_if(synapses.begin(), synapses.end(), [&a, &b] (Synapse& s) {
			return s.a==a&&s.b==b;
		})!=synapses.end()) return;

		//otherwise add the sucker
		synapses.push_back(Synapse(a, b, rand01()));
	}

	//not sure what the weighting should be...
	void mutate(float rate) {
		if (rand01()<rate) mutateAddNode();
		if (rand01()<rate) mutateAddSynapse();

		for (auto& s:synapses) s.mutate(rate);
	}

	//merge two brains
	static [[nodiscard]] bool crossover(Brain& a, Brain& b, Brain& out) {
		if (a.numIn!=b.numIn||a.numOut!=b.numOut) return false;

		//ting
		out={a.numIn, a.numOut};

		return true;
	}
};

#ifdef _OSTREAM_
std::ostream& operator<<(std::ostream& o, const Brain::Neuron& n) {
	return o<<"N["<<"IHO"[n.type]<<", l="<<n.layer<<']';
}

std::ostream& operator<<(std::ostream& o, const Brain::Synapse& s) {
	return o<<"S["<<s.a<<"->"<<s.b<<", w="<<s.weight<<"]";
}

std::ostream& operator<<(std::ostream& o, const Brain& b) {
	o<<"B[\n";
	for (const auto& n:b.neurons) o<<'\t'<<n<<'\n';
	for (const auto& s:b.synapses) o<<'\t'<<s<<'\n';
	o<<"]\n";

	return o;
}
#endif
#endif