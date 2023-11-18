#include <list>

#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

#include "entity.h"

#include "sound.h"

struct GameAIDemo : GameEngine {
	Entity player;
	float deltaTime=0;
	std::list<Sound> sounds;

	GameAIDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	bool init() override {
		return true;
	}

	void update(float dt) override {
		float moveSpeed=95.3f*deltaTime;
		if (Keyboard::isKeyPressed(Keyboard::LShift)) moveSpeed*=.3f;
		const float turnSpeed=4.6f*deltaTime;

		bool up=Keyboard::isKeyPressed(Keyboard::Up);
		bool down=Keyboard::isKeyPressed(Keyboard::Down);
		if (up) player.move(moveSpeed);
		if (down) player.move(-.6f*moveSpeed);
		if (up||down) sounds.push_back({player.pos});
		if (Keyboard::isKeyPressed(Keyboard::Left)) player.turn(-turnSpeed);
		if (Keyboard::isKeyPressed(Keyboard::Right)) player.turn(turnSpeed);

		for (auto it=sounds.begin(); it!=sounds.end();) {
			auto& s=*it;

			s.update(dt);

			if (!s.isDead()) it++;
			else it=sounds.erase(it);
		}

		deltaTime=dt;
	}

	void render() override {
		clear(Color(0xa5a5a5ff));
		Float2 dir(cosf(player.rot), sinf(player.rot));
		Float2 fr=player.pos+15*dir, ba=player.pos-15*dir;
		Float2 tang(-dir.y, dir.x);
		Float2 lt=ba-15*tang, rt=ba+15*tang;
		drawThickLine(lt, rt, 2);
		drawThickLine(lt, fr, 2);
		drawThickLine(rt, fr, 2);

		for (const auto& s:sounds) {
			float pct=1-s.rad/s.maxRad;
			drawCircle(s.pos, s.rad, Color(255*pct, 255*pct, 255*pct));
		}
	}
};