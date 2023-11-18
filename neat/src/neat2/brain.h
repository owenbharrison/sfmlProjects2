/*
#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <vector>

inline float rand01() {
	return rand()/float(RAND_MAX);
}

inline size_t randN(size_t n) {
	return size_t(n*rand01())%n;
}

struct Brain {
	size_t numIn=0, numOut=0;
	float mutRate=.01f;

	std::vector<int> somas;
	struct Axon {
		size_t from=0, to=0;
		float weight=0;
		bool enabled=true;
	};
	std::vector<Axon> axons;

	Brain() {}

	Brain(size_t in, size_t o, float r) : numIn(in), numOut(o), mutRate(r) {
		for (size_t i=0; i<numIn; i++) somas.push_back(0);
		for (size_t j=0; j<numOut; j++) somas.push_back(-1);

		for (size_t i=0; i<numIn; i++) {
			for (size_t j=0; j<numOut; j++) {
				axons.push_back({i, numIn+j, rand01()});
			}
		}
	}

	//someting wrong w/ dis
	void mutateAddSoma() {
		Axon* a;//rand enabled axon
		do a=&axons[randN(axons.size())];
		while (!a->enabled);

		a->enabled=false;

		//push everything back?[fix this]
		const size_t& st=somas[a->from], & en=somas[a->to];
		if (en-st==1) {
			for (auto& s:somas) {
				if (s>=en) s++;
			}
		}

		const size_t i=somas.size();
		somas.push_back(st+1);

		axons.push_back({st, i, rand01()});
		axons.push_back({i, en, rand01()});
	}

	void mutateAddAxon() {
		size_t f;//rand non-out soma
		do f=randN(somas.size());
		while (somas[f]==-1);

		size_t t;//rand "next" soma
		do t=randN(somas.size());
		while (somas[t]!=-1&&somas[t]<=somas[f]);

		//only add if unique
		if (std::find_if(axons.begin(), axons.end(), [&f, &t] (const Axon& a) {
			return a.from==f&&a.to==t;
		})==axons.end()) axons.push_back({f, t, rand01()});
	}

	void mutateEditAxons() {
		for (auto& a:axons) {
			if (rand01()>mutRate) continue;

			//rarely change weight entirely
			if (rand01()<.1f) a.weight=rand01();
			else a.weight+=.1f-.2f*rand01();
		}
	}

	void mutate() {
		if (rand01()<mutRate) mutateAddSoma();
		if (rand01()<mutRate) mutateAddAxon();
		mutateEditAxons();
	}

	[[nodiscard]] Brain crossover(const Brain& b) {
		Brain baby; baby.numIn=numIn, baby.numOut=numOut, baby.mutRate=.5f*(mutRate+b.mutRate);

		//make layer condense function

		return baby;
	}
};

#ifdef _OSTREAM_
std::ostream& operator<<(std::ostream& o, const Brain::Axon& a) {
	std::cout<<"A["<<a.from<<"->"<<a.to<<", w="<<a.weight<<']';

	return o;
}

std::ostream& operator<<(std::ostream& o, const Brain& b) {
	std::cout<<"Brain["<<b.numIn<<'x'<<b.numOut<<", r="<<b.mutRate<<"]:\n";

	std::cout<<"\tSomas["<<b.somas.size()<<"]:\n";
	for (size_t i=0; i<b.somas.size(); i++) {
		const auto& s=b.somas[i];
		char type=s==0?'I':s==-1?'O':'H';
		std::cout<<"\t\t"<<i<<": S["<<type<<", l="<<s<<"]\n";
	}

	std::cout<<"\tAxons["<<b.axons.size()<<"]:\n";
	for (size_t i=0; i<b.axons.size(); i++) {
		std::cout<<"\t\t"<<i<<": ";
		const auto& a=b.axons[i];
		if (a.enabled) std::cout<<a;
		else std::cout<<"[disabled]";
		std::cout<<'\n';
	}

	return o;
}
#endif
#endif
*/

#pragma once
#ifndef BRAIN_H
#define BRAIN_H

#include <vector>

inline float rand01() {
	return float(rand())/RAND_MAX;
}

inline size_t randN(size_t n) {
	size_t i=n*rand01();
	return i%n;
}

template<typename T>
inline T& randElem(std::vector<T>& vec) {
	return vec[randN(vec.size())];
}

struct Brain {
	size_t numIn=0, numOut=0;

	struct Axon {
		size_t to=0;
		float weight=0;
		bool enabled=true;
	};
	struct Soma {
		int layer=0;
		std::vector<Axon> axons;
	};
	std::vector<Soma> somas;

	Brain() {}

	Brain(size_t in, size_t out) : numIn(in), numOut(out) {
		for (size_t i=0; i<numIn+numOut; i++) {
			somas.push_back({(i<numIn)-1});
		}

		for (size_t i=0; i<numIn; i++) {
			for (size_t j=0; j<numOut; j++) {
				somas[i].axons.push_back({numIn+j, rand01()});
			}
		}
	}

	void mutateAddSoma() {
		Soma* from;//rand non-out
		do from=&randElem(somas);
		while (from->layer==-1);

		Axon* syn;//rand enabled
		do syn=&randElem(from->axons);
		while (!syn->enabled);
		
		syn->enabled=false;

		for (auto& n:somas) {
			if (n.layer>=from->layer) n.layer++;
		}

		const size_t n=somas.size();
		somas.push_back({from->layer+1});

		from->axons.push_back({n, rand01()});
		somas[n].axons.push_back({syn->to, rand01()});
	}

	void mutateAddAxon() {
		Soma* from;//rand non-out
		do from=&randElem(somas);
		while (from->layer==-1);

		size_t to;//rand "next"
		do to=randN(somas.size());
		while (somas[to].layer!=-1&&somas[to].layer>from->layer);

		if (std::find_if(from->axons.begin(), from->axons.end(), [&to] (const Axon& s) {
			return s.to==to;
		})==from->axons.end()) from->axons.push_back({to, rand01()});
	}

	void mutateEditAxons(float rate) {
		for (auto& s:somas) {
			for (auto& a:s.axons) {
				if (rand01()>rate) continue;

				//rarely change completely
				if (rand01()<.2f) a.weight=rand01();
				else a.weight+=.15f-.3f*rand01();
			}
		}
	}

	void mutate(float rate) {
		if (rand01()<rate) mutateAddSoma();
		if (rand01()<rate) mutateAddAxon();
		mutateEditAxons(rate);
	}
};

#ifdef _OSTREAM_
std::ostream& operator<<(std::ostream& o, const Brain::Soma& s) {
	char type=s.layer==0?'I':s.layer==-1?'O':'H';
	std::cout<<"Soma."<<type<<"(l="<<s.layer<<", #a="<<s.axons.size()<<")\n";
	
	for (const auto& a:s.axons) {
		std::cout<<"\tAxon(->"<<a.to<<", w="<<a.weight<<")\n";
	}

	return o;
}

std::ostream& operator<<(std::ostream& o, const Brain& b) {
	std::cout<<"Brain("<<b.numIn<<'x'<<b.numOut<<")\n";

	for (const auto& s:b.somas) std::cout<<s;

	std::cout<<"End Brain\n";
	
	return o;
}
#endif
#endif