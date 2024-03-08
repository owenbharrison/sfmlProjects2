#include <iostream>

#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

struct AStarNav : GameEngine {
	AStarNav(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	std::vector<Float2> pts;

	bool dragging=false;
	Float2 dragStart, tempOffset;
	Float2 offset;

	float scale=0;

	inline Float2 wldToScr(Float2 p) {
		return offset+scale*p;
	}

	inline Float2 scrToWld(Float2 p) {
		return (p-offset)/scale;
	}

	void resetTransform() {
		offset=Float2(width, height)/2;
		scale=std::min(width, height)/2;
	}

	bool init() override {
		resetTransform();

		return true;
	}

	void onMouseDown(Mouse::Button btn) override {
		switch (btn) {
			case Mouse::Left:
				dragStart=mousePos;
				dragging=true;
				break;
		}
	}

	void onMouseUp(Mouse::Button btn) override {
		switch (btn) {
			case Mouse::Left:
				offset+=tempOffset;
				tempOffset=0;
				dragging=false;
				break;
		}
	}

	void onMouseScroll(float scroll) override {
		float diff=.1f*scroll;

		scale*=1+diff;
		offset+=diff*(offset-mousePos);
	}

	void onKeyDown(Keyboard::Key key) override {
		switch (key) {
			case Keyboard::A: 
				pts.push_back(scrToWld(mousePos));
				break;
			case Keyboard::Home:
				resetTransform();
				break;
		}
	}

	void update(float dt) override {
		if(dragging) tempOffset=mousePos-dragStart;
	}

	void drawArrow(Float2 a, Float2 b, Color col=Color::White) {
		drawThickLine(a, b, 1, col);

		float t=.1f;
		Float2 tng=t*(b-a), nrm(tng.y, -tng.x);
		drawThickLine(b, b-tng-nrm, 1, col);
		drawThickLine(b, b-tng+nrm, 1, col);
	}

	void render() override {
		offset+=tempOffset;
		clear(Color(0x404040ff));

		drawArrow(wldToScr(0), wldToScr({.5f, 0}), Color::Red);
		drawArrow(wldToScr(0), wldToScr({0, -.5f}), Color::Green);

		for (const auto& p:pts) {
			fillCircle(wldToScr(p), scale*10.f);
			drawCircle(wldToScr(p), scale*10.f);
		}

		offset-=tempOffset;
	}
};