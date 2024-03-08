#pragma once
#ifndef NEURALNET_H
#define NEURALNET_H

#include "math/matrix.h"
using namespace common;

#include <random>
std::mt19937_64 generator(time(NULL));
std::uniform_real_distribution<float> distribution(0, 1);

inline float rand01() {
	return distribution(generator);
}

void randomize(const Matrix& m) {
	for (size_t i=0; i<m.m*m.n; i++) m.v[i]=rand01()-.5f;
}

//element wise multiplication
Matrix operator&(const Matrix& a, const Matrix& b) {
	assert(a.m==b.m&&a.n==b.n);

	Matrix c(a.m, a.n);
	for (size_t i=0; i<a.m*a.n; i++) c.v[i]=a.v[i]*b.v[i];
	return c;
}

struct NeuralNet {
	//activation functions
	static inline float reLu(float x) {
		return x<0?0:x;
	}
	static inline float sigmoid(float x) {
		return 1/(1+expf(-x));
	}

	static FloatFunc ddx(FloatFunc func) {
		return [func] (float x) {
			const float h=.00001f;
			return (func(x+h)-func(x))/h;
		};
	}

	//classifier prediction
	static Matrix softmax(const Matrix& a) {
		Matrix b(a.m, a.n);
		for (size_t j=0; j<a.n; j++) {
			float sum=0;//thru each column
			for (size_t i=0; i<a.m; i++) sum+=(b(i, j)=expf(a(i, j)));
			for (size_t i=0; i<a.m; i++) b(i, j)/=sum;//scale
		}
		return b;
	}

	size_t numLayers, inputSz, outputSz;
	Matrix* weights, * biases;
	FloatFunc hiddenActivation=reLu;
	std::function<Matrix(const Matrix&)> outputActivation=softmax;

	NeuralNet() :numLayers(0), inputSz(0), outputSz(0), weights(nullptr), biases(nullptr) {}

	NeuralNet(std::vector<size_t> sizes) : numLayers(sizes.size()-1) {
		inputSz=sizes[0], outputSz=sizes[numLayers];

		weights=new Matrix[numLayers];
		biases=new Matrix[numLayers];
		for (size_t i=0; i<numLayers; i++) {
			randomize(weights[i]=Matrix(sizes[i+1], sizes[i]));
			randomize(biases[i]=Matrix(sizes[i+1], 1));
		}
	}

	NeuralNet(const NeuralNet& n) : numLayers(n.numLayers), inputSz(n.inputSz), outputSz(n.outputSz) {
		weights=new Matrix[numLayers];
		biases=new Matrix[numLayers];
		for (size_t i=0; i<numLayers; i++) {
			weights[i]=n.weights[i];
			biases[i]=n.biases[i];
		}
	}

	~NeuralNet() {
		delete[] weights;
		delete[] biases;
	}

	NeuralNet& operator=(const NeuralNet& n) {
		if (this==&n) return *this;

		delete[] weights;
		delete[] biases;

		numLayers=n.numLayers;
		inputSz=n.inputSz;
		outputSz=n.outputSz;

		weights=new Matrix[numLayers];
		biases=new Matrix[numLayers];

		for (size_t i=0; i<numLayers; i++) {
			weights[i]=n.weights[i];
			biases[i]=n.biases[i];
		}

		return *this;
	}

	//gradient descent
	[[nodiscard]] bool train(const Matrix& X, const Matrix& Y, size_t iter, float alpha, std::function<bool(NeuralNet&, Matrix&)> callback={}) {
		size_t M=X.n;
		if (X.m!=inputSz||Y.m!=outputSz||M!=Y.n) return false;

		Matrix* unact=new Matrix[numLayers];
		Matrix* actvs=new Matrix[numLayers+1]; actvs[0]=X;
		Matrix* deltW=new Matrix[numLayers];
		for (size_t t=0; t<iter; t++) {
			//forward propagation
			for (size_t k=0; k<numLayers; k++) {
				auto& z=(unact[k]=weights[k]*actvs[k]);
				for (size_t i=0; i<z.m; i++) {
					float& b=biases[k](i, 0);
					for (size_t j=0; j<z.n; j++) z(i, j)+=b;
				}

				actvs[k+1]=k==numLayers-1?outputActivation(z):z.forEach(hiddenActivation);
			}

			//backward propagation
			Matrix pDz;
			for (size_t k=0; k<numLayers; k++) {
				size_t l=numLayers-k-1;

				Matrix dz=k==0?
					actvs[numLayers]-Y:
					weights[l+1].transpose()*pDz&unact[l].forEach(ddx(hiddenActivation));
				deltW[l]=dz*actvs[l].transpose();

				Matrix db(dz.m, 1);
				for (size_t i=0; i<dz.m; i++) {
					float& d=db(i, 0);
					for (size_t j=0; j<dz.n; j++) dz(i, j)+=d;
				}
				biases[l]-=db*(alpha/M);

				pDz=dz;
			}

			//update params
			for (size_t i=0; i<numLayers; i++) {
				weights[i]-=deltW[i]*(alpha/M);
			}

			//optional callback function
			if (callback(*this, actvs[numLayers])) break;
		}
		delete[] unact;
		delete[] actvs;
		delete[] deltW;

		return true;
	}

	Matrix predict(const Matrix& m) {
		assert(m.m==inputSz&&m.n==1);

		Matrix a=m;
		for (size_t i=0; i<numLayers; i++) {
			Matrix z=weights[i]*a+biases[i];
			a=(i==numLayers-1)?outputActivation(z):z.forEach(hiddenActivation);
		}
		return a;
	}
};
#endif