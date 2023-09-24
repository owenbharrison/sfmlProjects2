#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <random>

#include "math/Matrix.h"
using namespace common;

#include <SFML/Graphics.hpp>

//rng for nn initialization
std::mt19937_64 generator(time(NULL));
std::uniform_real_distribution<float> distribution(0, 1);
inline float rand01() { return distribution(generator); }
void randomize(Matrix& m) {
	for (size_t i=0; i<m.m*m.n; i++) m.v[i]=rand01()-.5f;
}

//activation functions
inline float reLu(float x) { return x>0?x:0; }
Matrix reLu(const Matrix& a) {
	Matrix b(a.m, a.n);
	for (size_t i=0; i<a.m*a.n; i++) b.v[i]=reLu(a.v[i]);
	return b;
}

inline float ddxReLu(float x) { return x>0; }
Matrix ddxReLu(const Matrix& a) {
	Matrix b(a.m, a.n);
	for (size_t i=0; i<a.m*a.n; i++) b.v[i]=ddxReLu(a.v[i]);
	return b;
}

//classifier prediction
Matrix softmax(const Matrix& a) {
	Matrix b(a.m, a.n);
	for (size_t j=0; j<a.n; j++) {
		float sum=0;
		for (size_t i=0; i<a.m; i++) sum+=(b(i, j)=expf(a(i, j)));
		for (size_t i=0; i<a.m; i++) b(i, j)/=sum;
	}
	return b;
}

//Matrix functionality
Matrix elemMult(const Matrix& a, const Matrix& b) {
	size_t m=a.m, n=a.n;
	assert(m==b.m, "elem wise mult must be of same height");
	assert(n==b.n, "elem wise mult must be of same width");

	Matrix c(m, n);
	for (size_t i=0; i<m*n; i++) c.v[i]=a.v[i]*b.v[i];
	return c;
}

//printing
std::ostream& operator<<(std::ostream& o, const Matrix& m) {
	o<<'('<<m.m<<'x'<<m.n<<")\n";
	for (size_t i=0; i<m.m; i++) {
		for (size_t j=0; j<m.n; j++) {
			o<<m(i, j)<<' ';
		}
		o<<'\n';
	}
	return o;
}

int main() {
	const float alpha=.21f;
	const size_t inputSz=784, hiddenSz=10, outputSz=10;
	const size_t M=600;
	Matrix X(inputSz, M);
	Matrix Y(outputSz, M);

	//load training data: nn.train("res/train.csv")
	std::ifstream file("res/train.csv");
	if (file.is_open()) {
		std::string line;
		//skip first line
		std::getline(file, line);
		char junk;
		int label, pix;
		for (size_t j=0; std::getline(file, line)&&j<M; j++) {
			std::istringstream iss(line);
			char junk;

			iss>>label>>junk;
			Y(label, j)=1;

			for (size_t i=0; i<inputSz&&(iss>>pix>>junk); i++) {
				X(i, j)=pix/255.f;
			}
		}
		file.close();
		std::cout<<"successfully loaded data\n";
	} else {
		std::cerr<<"unable to load file\n";
		exit(-1);
	}

	//values that change over every iteration
	Matrix w1(hiddenSz, inputSz); randomize(w1);
	Matrix b1(hiddenSz, 1); randomize(b1);
	Matrix w2(outputSz, hiddenSz); randomize(w2);
	Matrix b2(outputSz, 1); randomize(b2);
	
	std::cout<<"gradient descent(training) started\n";
	Matrix a0=X;
	for (size_t k=0; k<1000; k++) {
		//forward propagation

		Matrix z1=w1*a0;//+b1
		for (size_t i=0; i<z1.m; i++) {
			float& b=b1(i, 0);
			for (size_t j=0; j<z1.n; j++) z1(i, j)+=b;
		}
		Matrix a1=reLu(z1);

		Matrix z2=w2*a1;//+b2
		for (size_t i=0; i<z2.m; i++) {
			float& b=b2(i, 0);
			for (size_t j=0; j<z2.n; j++) z2(i, j)+=b;
		}
		Matrix a2=softmax(z2);

		//backward propagation
		Matrix dz2=a2-Y;
		Matrix dw2=dz2*a1.transpose()*(1.f/M);
		Matrix db2(outputSz, 1);
		for (size_t i=0; i<db2.m; i++) {
			float& d=db2(i, 0);
			for (size_t j=0; j<db2.n; j++) d+=dz2(i, j);
		}
		db2*=1.f/M;

		Matrix dz1=elemMult(w2.transpose()*dz2, ddxReLu(z1));
		Matrix dw1=dz1*a0.transpose()*(1.f/M);
		Matrix db1(hiddenSz, 1);
		for (size_t i=0; i<db1.m; i++) {
			float& d=db1(i, 0);
			for (size_t j=0; j<db1.n; j++) d+=dz1(i, j);
		}
		db1*=1.f/M;

		//update params
		b2-=db2*alpha;
		w1-=dw1*alpha;
		b1-=db1*alpha;
		w2-=dw2*alpha;

		//print stats
		if (k%10==0) {
			size_t num=0;
			for (size_t j=0; j<a2.n; j++) {
				size_t r=0;
				float record=0;
				for (size_t i=0; i<a2.m; i++) {
					float v=a2(i, j);
					if (v>record) record=v, r=i;
				}
				if (Y(r, j)) num++;
			}
			int pct=100.f*num/a2.n;
			std::cout<<"iter "<<k<<": "<<pct<<"% accurate\n";

			if(pct>97) break;
		}
	}
	std::cout<<"training finished\n";

	return 0;
}