#include <iostream>
#include <fstream>
#include <sstream>

#include "engine/gameEngine.h"
using namespace sf;

#include "neuralNet.h"

#include "io/stopwatch.h"
#include "geom/aabb.h"

float clamp(float x, float a, float b) {
	if (x<a) return a;
	if (x>b) return b;
	return x;
}

struct NeuralNetDemo : GameEngine {
	NeuralNetDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	~NeuralNetDemo() {
		delete[] grid;
		delete[] pixels;
	}

	const size_t gridSz=28, gridLen=gridSz*gridSz;
	float* grid=nullptr;
	Uint8* pixels=nullptr;
	Texture gridTex;
	Shader gridShader;

	AABB gridBox;

	NeuralNet neuralNet;
	Matrix trainingData;

	bool init() override {
		grid=new float[gridLen] {0};
		pixels=new Uint8[gridLen*4]{255};
		if (!gridTex.create(gridSz, gridSz)) return false;
		if (!gridShader.loadFromFile("shader/square.glsl", Shader::Fragment)) return false;
		gridShader.setUniform("Resolution", Vector2f(width, height));

		{
			Float2 ctr(width/2, height/2);
			float dim=MIN(width, height)/2;
			gridBox=AABB(ctr-dim, ctr+dim);
		}

		neuralNet=NeuralNet({gridLen, 16, 10});
		const size_t M=2500;
		Matrix X(neuralNet.inputSz, M);
		Matrix Y(neuralNet.outputSz, M);

		//load training data
		std::ifstream file("res/train.csv");
		if (file.is_open()) {
			std::cout<<"loading data...\n";

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

				for (size_t i=0; i<neuralNet.inputSz&&(iss>>pix>>junk); i++) {
					X(i, j)=pix/255.f;
				}
			}
			file.close();
			std::cout<<"successfully loaded data\n";
		} else return false;

		trainingData=X;

		std::cout<<"training started...\n";
		Stopwatch trainWatch; trainWatch.start();
		if (!neuralNet.train(X, Y, 750, .25f, [this, &Y] (NeuralNet& n, Matrix& m) {
			float cost=0;
			Matrix sub=m-Y;
			for (size_t i=0; i<sub.m*sub.n; i++) {
				cost+=sub.v[i]*sub.v[i];
			}
			std::cout<<"cost: "<<cost<<'\n';

			size_t num=0;
			for (size_t j=0; j<m.n; j++) {
				size_t r=0;
				float record=0;
				for (size_t i=0; i<m.m; i++) {
					float v=m(i, j);
					if (v>record) record=v, r=i;
				}
				if (Y(r, j)) num++;
			}
			float pct=float(num)/m.n;

			clear();
			RectangleShape rect(Vector2f(pct*width, 40));
			rect.setFillColor(Color(255*(1-pct), 255*pct, 35));
			draw(rect);
			display();

			//quit early
			return pct>.97f;
		})) return false;
		trainWatch.stop();
		std::cout<<"training finished in: "<<trainWatch.getMillis()<<"ms\n";

		return true;
	}

	void onKeyDown(Keyboard::Key key) override {
		switch (key) {
			case Keyboard::C:
			{//clear grid
				memset(grid, 0, gridLen*sizeof(float));
				break;
			}
			case Keyboard::Space:
			{
				Matrix input(gridLen, 1);
				for (size_t i=0; i<gridLen; i++) {
					input(i, 0)=grid[i];
				}
				Matrix output=neuralNet.predict(input);
				
				float record=0;
				size_t argmax=0;
				for (size_t i=0; i<output.m; i++) {
					float& v=output(i, 0);
					if (v>record) record=v, argmax=i;
				}
				std::cout<<"prediction: "<<argmax<<'\n';
				break;
			}
			case Keyboard::R: {
				size_t ri=rand01()*trainingData.n;
				for (size_t i=0; i<gridLen; i++) {
					grid[i]=trainingData(i, ri);
				}
				break;
			}
		}
	}

	void update(float dt) override {
		if (Mouse::isButtonPressed(Mouse::Left)) {
			Float2 p=gridSz*(mousePos-gridBox.min)/(gridBox.max-gridBox.min);
			int si=p.x, sj=p.y;
			for (int di=-1; di<=1; di++) {
				for (int dj=-1; dj<=1; dj++) {
					int i=si+di, j=sj+dj;
					if (i<0||j<0||i>=gridSz||j>=gridSz) continue;

					grid[i+j*gridSz]+=65*dt*expf(-(di*di+dj*dj));
				}
			}
		}

		//drag and update pixels
		for (size_t i=0; i<gridLen; i++) {
			Uint8 p=255*clamp(grid[i], 0, 1);
			pixels[i*4]=pixels[i*4+1]=pixels[i*4+2]=p;
			pixels[i*4+3]=255;
		}
		gridTex.update(pixels);
		gridShader.setUniform("MainTex", gridTex);
	}

	void showNeuralNet(const NeuralNet& n, AABB bounds, size_t maxRows=20) {
		size_t num=0;
		for (size_t i=0; i<n.numLayers; i++) {
			if (i==0) num+=MIN(n.weights[i].n, maxRows);
			num+=MIN(n.weights[i].m, maxRows);
		}
		std::cout<<num<<'\n';
	}

	void render() override {
		draw(RectangleShape(Vector2f(width, height)), &gridShader);
	}
};