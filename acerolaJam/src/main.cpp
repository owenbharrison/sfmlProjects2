#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

Float2 polarToCartesian(float angle, float rad=1) {
	return rad*Float2(cosf(angle), sinf(angle));
}

struct Creature {
	enum Type { Good, Bad, Ugly } type;
	Float2 pos;
	float rad=230;
};

struct Demo : GameEngine {
	Demo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	Float2 resolution, center;

	float rot=0;
	Float2 player;

	std::vector<Creature> creatures;

	Shader shader;
	Texture shader_tex;

	bool init() override {
		if (!shader.loadFromFile("shader/ca.glsl", Shader::Fragment)) return false;
		shader.setUniform("Resolution", Vector2f(width, height));
		if (!shader_tex.create(width, height)) return false;

		resolution=Float2(width, height), center=resolution/2;
		player=center;

		const float Pi=3.1415927;
		float spacing=.33f*width;
		creatures.push_back({Creature::Good, center+polarToCartesian(Pi/2, spacing)});
		creatures.push_back({Creature::Bad, center+polarToCartesian(Pi*7/6, spacing)});
		creatures.push_back({Creature::Ugly, center+polarToCartesian(Pi*11/6, spacing)});

		return true;
	}

	void update(float dt) override {
		if (Keyboard::isKeyPressed(Keyboard::Up)) {
			player+=polarToCartesian(rot, 120*dt);
		}
		if (Keyboard::isKeyPressed(Keyboard::Down)) {
			player-=polarToCartesian(rot, 70*dt);
		}
		if (Keyboard::isKeyPressed(Keyboard::Left)) rot-=2*dt;
		if (Keyboard::isKeyPressed(Keyboard::Right)) rot+=2*dt;
	}

	void drawArrow(Float2 a, Float2 b, Color col=Color::White) {
		Float2 sub=b-a, sz=sub*.2f, tang{-sz.y, sz.x};
		Float2 aSt=b-sz, lPt=aSt-tang*.5f, rPt=aSt+tang*.5f;
		//line
		drawThickLine(a, aSt, 1, col);
		//triangle
		drawThickLine(rPt, b, 1, col);
		drawThickLine(b, lPt, 1, col);
		drawThickLine(lPt, rPt, 1, col);
	}

	void render() override {
		clear(Color(0x777777ff));

		const Float2 player_dir=polarToCartesian(rot);
		fillCircle(player, 8, Color::Green);
		drawArrow(player, player+40*player_dir);

		for (const auto& c:creatures) {
			Color col;
			switch (c.type) {
				case Creature::Good: col=Color::Green; break;
				case Creature::Bad: col=Color::Red; break;
				case Creature::Ugly: col=Color::Blue; break;
			}
			fillCircle(c.pos, 8);
			drawCircle(c.pos, 8, Color::Black);

			drawCircle(c.pos, c.rad, col);
		}

		//calculate aberration amounts
		Float2 r_tot, g_tot, b_tot;
		for (const auto& c:creatures) {
			Float2 sub=c.pos-player;
			float amt=expf(-length(sub)/c.rad);
			Float2 offset=amt*normal(sub);
			switch (c.type) {
				case Creature::Good: g_tot+=offset; break;
				case Creature::Bad: r_tot+=offset; break;
				case Creature::Ugly: b_tot+=offset; break;
			}
		}
		r_tot/=creatures.size();
		g_tot/=creatures.size();
		b_tot/=creatures.size();
		//draw them
		drawArrow(player, player+150*r_tot, Color::Red);
		drawArrow(player, player+150*g_tot, Color::Green);
		drawArrow(player, player+150*b_tot, Color::Blue);
		//send them to shader
		//shader.setUniform("R_Off", Vector2f(r_tot.x, r_tot.y));
		//shader.setUniform("G_Off", Vector2f(g_tot.x, g_tot.y));
		//shader.setUniform("B_Off", Vector2f(b_tot.x, b_tot.y));

		//shader_tex.update(*this);
		//shader.setUniform("Main_Tex", shader_tex);
		
		//clear();
		//draw(Sprite(shader_tex), &shader);
	}
};

int main() {
	Demo demo(600, 600, "Acerola Jam");
	demo.setFramerateLimit(120);
	demo.run();

	return 0;
}