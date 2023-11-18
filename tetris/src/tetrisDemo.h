#include <iostream>

#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

#include "tetris.h"

struct TetrisDemo : GameEngine {
	Tetris game;

	Uint8* pixels=nullptr;
	Texture blockTex;
	Shader blockShader;

	TetrisDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	bool init() override {
		srand(time(NULL));

		pixels=new Uint8[4*game.width*game.height];
		if (!blockTex.create(game.width, game.height)) return false;
		if (!blockShader.loadFromFile("shader/block.glsl", Shader::Fragment)) return false;
		blockShader.setUniform("Resolution", Vector2f(width, height));

		return true;
	}

	void onKeyDown(Keyboard::Key k) override {
		switch (k) {
			case Keyboard::Left: game.moveLeft(); break;
			case Keyboard::Right: game.moveRight(); break;
			case Keyboard::Z: game.rotateLeft(); break;
			case Keyboard::Up: game.rotateRight(); break;
			case Keyboard::Down: game.softDrop(true); break;
			case Keyboard::Space: game.hardDrop(); break;
		}
	}

	void update(float dt) override {
		game.update(dt);

		for (size_t i=0; i<game.width*game.height; i++) {
			pixels[4*i]=pixels[1+4*i]=pixels[2+4*i]=0;
			pixels[3+4*i]=255;
		}

		int futY=game.curY;
		while (game.canFit(game.curTet, game.curX, futY+1, game.curRot)) futY++;
		for (size_t i=0; i<3; i++) {
			for (size_t j=0; j<3; j++) {
				//future piece
				if (game.tetros[game.curTet][game.tIx(i, j, game.curRot)]) {
					size_t k=4*game.gIx(game.curX+i, futY+j);
					pixels[k]=pixels[1+k]=pixels[2+k]=85;
				}
				//current piece
				if (game.tetros[game.curTet][game.tIx(i, j, game.curRot)]) {
					size_t k=4*game.gIx(game.curX+i, game.curY+j);
					pixels[k]=pixels[1+k]=pixels[2+k]=pixels[3+k]=170;
				}
			}
		}

		//previous
		for (size_t i=0; i<game.width*game.height; i++) {
			if (game.grid[i]) pixels[4*i]=pixels[1+4*i]=pixels[2+4*i]=255;
		}

		blockTex.update(pixels);
		blockShader.setUniform("MainTex", blockTex);

		std::cout<<"Score: "<<game.score<<" Level: "<<game.level<<'\n';
	}

	void render() override {
		draw(RectangleShape(Vector2f(width, height)), &blockShader);
	}
};