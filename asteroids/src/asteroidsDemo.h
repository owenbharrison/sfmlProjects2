#include "engine/gameEngine.h"
using namespace sf;

#include <list>
#include <vector>

#include "game/ship.h"
#include "game/asteroid.h"
#include "game/bullet.h"

Float2 ptOnAABB(AABB a) {
	Float2 p;
	if (rand01()<.5f) {
		p.x=rand01()<.5f?a.min.x:a.max.x;
		p.y=a.min.y+rand01()*(a.max.y-a.min.y);
	} else {
		p.y=rand01()<.5f?a.min.y:a.max.y;
		p.x=a.min.x+rand01()*(a.max.x-a.min.x);
	}
	return p;
}

void wrapAABB(const AABB& a, Float2& p) {
	if (p.x<a.min.x) p.x=a.max.x;
	if (p.y<a.min.y) p.y=a.max.y;
	if (p.x>a.max.x) p.x=a.min.x;
	if (p.y>a.max.y) p.y=a.min.y;
}

struct AsteroidsDemo : GameEngine {
	AsteroidsDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	Ship ship;
	std::list<Asteroid> asteroids;
	std::vector<Bullet> bullets;
	std::vector<Particle> particles;
	AABB bounds;

	float bulletTimer=0;
	float deltaTime=0;

	bool init() override {
		srand(time(NULL));
		Float2 resolution(width, height);

		bounds=AABB(-30, resolution+30);

		ship=Ship(resolution/2, 17);

		for (size_t i=0; i<4; i++) {
			float angle=2*PI*rand01();
			float speed=27+18*rand01();
			Float2 dir(cosf(angle), sinf(angle));
			size_t num=7+6*rand01();
			Asteroid ast(ptOnAABB(bounds), speed*dir, num, 20, 50);
			asteroids.push_back(ast);
		}

		return true;
	}

	void input() override {
		if (Keyboard::isKeyPressed(Keyboard::Up)) {
			ship.boost(190);
			particles.push_back(ship.emitParticle());
		}

		float turnSpeed=2.83f*deltaTime;
		if (Keyboard::isKeyPressed(Keyboard::Left)) ship.turn(-turnSpeed);
		if (Keyboard::isKeyPressed(Keyboard::Right)) ship.turn(turnSpeed);

		bulletTimer-=deltaTime;
	}

	void onKeyDown(Keyboard::Key key) override {
		if (key==Keyboard::Space&&bulletTimer<0) {
			bulletTimer=.347f;

			Float2 vel=polarToCartesian(140, ship.rot);
			bullets.push_back(Bullet(ship.pos, vel));
		}
	}

	void update(float dt) override {
		ship.update(dt);
		wrapAABB(bounds, ship.pos);

		for (auto& a:asteroids) {
			a.update(dt);
			wrapAABB(bounds, a.pos);
		}

		//check every bullet
		for (auto bit=bullets.begin(); bit!=bullets.end();) {
			auto& b=*bit;

			b.update(dt);

			bool toDel=false;
			for (auto ait=asteroids.begin(), aEnd=asteroids.end(); ait!=aEnd;) {
				auto& a=*ait;

				if (a.containsPt(b.pos)) {
					//split asteroid into two
					Asteroid a0, a1;
					if (a.split(a0, a1)) {
						asteroids.push_back(a0);
						asteroids.push_back(a1);
					}

					//add particle explosion
					size_t num=45+15*rand01();
					for (size_t i=0; i<num; i++) {
						float angle=2*PI*rand01();
						float speed=35*rand01();
						Float2 vel=polarToCartesian(speed, angle);
						float rad=.75f+rand01();
						float lifespan=1+rand01();
						particles.push_back(Particle(a.pos, vel, rad, lifespan));
					}

					//delete asteroid
					ait=asteroids.erase(ait);

					toDel=true;
					break;
				} else ait++;
			}
			if (!toDel) toDel=!b.isValid(bounds);
			if (toDel) bit=bullets.erase(bit);
			else bit++;
		}

		for (auto it=particles.begin(); it!=particles.end();) {
			auto& p=*it;

			p.update(dt);

			if (p.isValid(bounds)) it++;
			else it=particles.erase(it);
		}

		deltaTime=dt;
	}

	void render() override {
		clear();

		//draw ship	
		{
			//ship raycasting
			if (Keyboard::isKeyPressed(Keyboard::R)) {
				const size_t num=256;
				const float rad=134.7f, fov=.9f*PI;
				std::pair<Float2, Color> lToDraw[num];
				for (size_t i=0; i<num; i++) {
					float offset=fov*(float(i)/num-.5f);
					float angle=ship.rot+offset;
					Float2 vBA=polarToCartesian(rad, angle);
					Float2 vA=ship.pos, vB=vA+vBA;

					float record=1;
					Float2 ixPt;
					for (const auto& a:asteroids) {
						for (size_t j=0; j<a.numPoints; j++) {
							Float2 vC=a.pos+a.outline[j];
							Float2 vD=a.pos+a.outline[(j+1)%a.numPoints];
							Float2 tu=lineLineIntersection(vA, vB, vC, vD);
							if (tu.x>=0&&tu.x<=1&&tu.y>=0&&tu.y<=1) {
								if (tu.x<record) {
									record=tu.x;
									ixPt=vA+tu.x*vBA;
								}
							}
						}
					}

					//we hit something
					if (record!=1) {
						//show raycasting boxes
						if (Keyboard::isKeyPressed(Keyboard::LShift)) {
							float pct=1-record*cosf(offset);
							float wid=width/num, hei=height*pct;
							RectangleShape rect(Vector2f(wid, hei));
							rect.setPosition(wid*i, .5f*(height-hei));
							rect.setFillColor(Color(255*pct, 255*pct, 255*pct));
							draw(rect);
						}
						lToDraw[i]={ixPt, Color::Red};
					} else lToDraw[i]={vB, Color::Green};
				}

				Float2 prev=ship.pos;
				for (size_t i=0; i<num; i++) {
					const auto& l=lToDraw[i];
					drawLine(prev, l.first, l.second);
					prev=l.first;
				}
			}
		

			Float2 front=polarToCartesian(.5f*ship.rad, ship.rot);
			Float2 side=.6f*Float2(-front.y, front.x);
			Float2 a=ship.pos+front, b=ship.pos-front-side, c=ship.pos-front+side;
			drawThickLine(a, b, 1); drawThickLine(a, c, 1);
			drawThickLine(b, c, 1);
		}

		for (const auto& p:particles) {
			float pct=1-p.age/p.lifespan;
			fillCircle(p.pos, p.rad, Color(255*pct, 255*pct, 255*pct));
		}

		for (const auto& b:bullets) {
			fillCircle(b.pos, 2.1f);
		}

		for (const auto& a:asteroids) {
			for (size_t i=0; i<a.numPoints; i++) {
				Float2 b=a.outline[i], c=a.outline[(i+1)%a.numPoints];
				drawThickLine(a.pos+b, a.pos+c, 1.2f);
			}
		}
	}
};