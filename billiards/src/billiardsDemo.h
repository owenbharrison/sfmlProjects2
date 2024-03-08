#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

#include <vector>

#include <random>

#include "ball.h"

std::mt19937_64 generator(time(NULL));

inline float rand01() {
	static std::uniform_real_distribution<float> dist(0, 1);
	return dist(generator);
}

struct BilliardsDemo : GameEngine {
	BilliardsDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	std::vector<Ball> balls;

	AABB bounds;

	float timer=0;

	bool init() override {
		bounds=AABB(Float2(0), Float2(width, height));

		return true;
	}

	void update(float dt) override {
		for (size_t i=0; i<balls.size(); i++) {
			auto& a=balls[i];
			for (size_t j=i+1; j<balls.size(); j++) {
				auto& b=balls[j];

				Float2 xSub=a.pos-b.pos;
				float dSq=dot(xSub, xSub);
				float tRad=a.rad+b.rad;
				if (dSq<tRad*tRad) {
					float mTot=a.mass+b.mass;
					Float2 vSub=a.vel-b.vel;
					float vdotx=dot(vSub, xSub);
					Float2 diff=2*vdotx/(mTot*dSq)*xSub;

					a.vel-=b.mass*diff;
					b.vel+=a.mass*diff;

					float err=sqrtf(dSq)-tRad;
					Float2 amt=.5f/mTot*err*normal(xSub);
					a.pos-=a.mass*amt;
					b.pos+=b.mass*amt;
				}
			}
		}

		float mTot=0;
		for (auto& b:balls) {
			//b.vel.y+=b.mass*9.8f*dt;

			//b.vel*=1-dt;

			b.update(dt);
			b.checkAABB(bounds);
			mTot+=b.mass;
		}

		if (Float2 sub=bounds.max-bounds.min; mTot/(sub.x*sub.y)<.2f) {
			float rad=7+3*rand01();
			Float2 xy(rand01(), rand01());
			Float2 pos=bounds.min+sub*xy;
			float angle=2*PI*rand01();
			Float2 dir(cosf(angle), sinf(angle));
			float speed=20+15*rand01();
			balls.push_back(Ball(pos, dir*speed, rad));
		}
	}

	void render() override {
		clear(Color(51, 51, 51));

		for (const auto& b:balls) {
			fillCircle(b.pos, b.rad);
			drawCircle(b.pos, b.rad, Color::Black);
		}
	}
};