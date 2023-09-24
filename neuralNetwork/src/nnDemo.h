#include "sfmlEngine.h"

struct nnDemo : public sfmlEngine {
	nnDemo(size_t w, size_t h, std::string t) : sfmlEngine(w, h, t) {}

	void update(float deltaTime) override {

	}

	void render() override {
		CircleShape circ(50);
		circ.setOrigin(50, 50);
		circ.setPosition(width/2, height/2);
		circ.setFillColor(Color(0xff0000ff));
		circ.setOutlineColor(Color(0x00ff00ff));
		circ.setOutlineThickness(-2);
		draw(circ);
	}
};