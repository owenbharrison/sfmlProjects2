#include <SFML/Graphics.hpp>
using namespace sf;

#include <iostream>

#include <list>
#include <algorithm>

#include "Fruit.h"
#include "FX.h"

#define PI 3.1415927f
#define MIN(a, b) ((a)<(b)?(a):(b))
#define RANDOM (float(rand())/RAND_MAX)

inline Float2 polarToCartesian(float angle, float rad) {
	return rad*Float2(cosf(angle), sinf(angle));
}

int main() {
	srand(time(NULL));

	//sfml setup
	unsigned int width=800;
	unsigned int height=600;
	RenderWindow window(VideoMode(width, height), "", Style::Titlebar|Style::Close);
	window.setFramerateLimit(165);
	Clock deltaClock;
	float totalDeltaTime=0;
	float FruitTimer=0, sliceTimer=0;

	//basic prog setup
	Float2 grav(0, 130);
	AABB bounds(Float2(0), Float2(width, height));
	std::list<Fruit> Fruits;
	std::list<FX> effects;

	bool toSlice=false;
	std::vector<Float2> slice;

	RenderTexture renderTex;
	if (!renderTex.create(width, height)) {
		std::cerr<<"Error: unable to create render texture.";
		exit(1);
	}
	Shader paperShader;
	if (!paperShader.loadFromFile("shader/paper.glsl", Shader::Fragment)) {
		std::cerr<<"Error: couldn't load shader";
		exit(1);
	}
	paperShader.setUniform("Resolution", Glsl::Vec2(width, height));
	Texture shaderTex;
	if (!shaderTex.create(width, height)) {
		std::cerr<<"Error: couldn't create texture";
		exit(1);
	}
	Sprite shaderSprite(shaderTex);

	//loop
	auto drawLine=[&renderTex] (Float2 a, Float2 b, Color col=Color::White, float w=1) {
		Float2 ba=b-a;
		RectangleShape line(Vector2f(length(ba), 2*w));
		line.setOrigin(Vector2f(0, w));
		line.setRotation(atan2(ba.y, ba.x)*180/PI);
		line.setPosition(Vector2f(a.x, a.y));

		line.setFillColor(col);
		renderTex.draw(line);
	};
	auto drawCircle=[&renderTex] (Float2 p, float r, Color col=Color::White) {
		CircleShape circ(r);
		circ.setOutlineThickness(-2);
		circ.setOutlineColor(col);
		circ.setFillColor(Color::Transparent);

		circ.setOrigin(Vector2f(r, r));
		circ.setPosition(Vector2f(p.x, p.y));
		renderTex.draw(circ);
	};
	auto fillCircle=[&renderTex] (Float2 p, float r, Color col=Color::White) {
		CircleShape circ(r);
		circ.setFillColor(col);

		circ.setOrigin(Vector2f(r, r));
		circ.setPosition(Vector2f(p.x, p.y));
		renderTex.draw(circ);
	};
	while (window.isOpen()) {
		//mouse position
		Vector2i mp=Mouse::getPosition(window);
		Float2 mousePos(mp.x, mp.y);

		//polling
		for (Event event; window.pollEvent(event);) {
			if (event.type==Event::Closed) window.close();
		}

		//timing
		float actualDeltaTime=deltaClock.restart().asSeconds();
		float deltaTime=MIN(actualDeltaTime, 1/60.f);
		totalDeltaTime+=actualDeltaTime;
		std::string fpsStr=std::to_string(int(1/actualDeltaTime))+"fps";
		window.setTitle("Fruit Ninja @ "+fpsStr+" w/ "+std::to_string(Fruits.size())+"Fruits & "+std::to_string(effects.size())+"FX");

		//start USER_INPUT
		bool toSliceTemp=Mouse::isButtonPressed(Mouse::Left);
		if (toSliceTemp!=toSlice) {
			if (toSlice) slice.clear();
			else slice.push_back(mousePos);
		}
		toSlice=toSliceTemp;
		//end USER_INPUT

		//start UPDATE
		const float sliceStep=.02f;
		if (toSlice) {
			//append to slice
			if (sliceTimer>sliceStep) {
				sliceTimer-=sliceStep;
				slice.push_back(mousePos);
			}

			//only check while slicing.
			for (auto it=Fruits.begin(); it!=Fruits.end();) {
				auto& f=*it;
				if (f.sliced) {
					it++;
					continue;
				}

				//find intersections
				int len=f.pts.size();
				bool found=false;
				Float2 ixPtA, ixPtB;
				int sIxA=-1, sIxB=-1, fIxA=-1, fIxB=-1;
				for (int i=0; i<slice.size()-1; i++) {
					Float2 a=f.localize(slice[i]);
					Float2 b=f.localize(slice[i+1]);
					//if "mid-slice" and point not in Fruit
					if (sIxA!=-1&&!f.checkLocalizedPt(a)) break;
					for (int j=0; j<len; j++) {
						Float2 c=f.pts[j];
						Float2 d=f.pts[(j+1)%len];
						Float2 tu=lineLineIntersection(a, b, c, d);
						if (tu.x>=0&&tu.x<=1&&tu.y>=0&&tu.y<=1) {
							Float2 ixPt=a+tu.x*(b-a);
							if (sIxA==-1) sIxA=i, fIxA=j, ixPtA=ixPt;
							else if (sIxB==-1) {
								sIxB=i, fIxB=j;
								ixPtB=ixPt;
								found=true;
								break;
							}
						}
					}
					if (found) break;
				}
				if (found&&sIxA!=sIxB&&fIxA!=fIxB) {//add chunks
					Fruit toAdd[2];

					//construct shell
					std::vector<Float2> slicePart{ixPtA};
					for (int i=sIxA+1; i<=sIxB; i++) slicePart.push_back(f.localize(slice[i]));
					slicePart.push_back(ixPtB);
					toAdd[0].pts=slicePart;
					for (int i=fIxB; i!=fIxA; i=i==0?len-1:i-1) toAdd[0].pts.push_back(f.pts[i]);
					for (int i=fIxA; i!=fIxB; i=i==0?len-1:i-1) toAdd[1].pts.push_back(f.pts[i]);
					std::reverse(toAdd[1].pts.begin(), toAdd[1].pts.end());
					toAdd[1].pts.insert(toAdd[1].pts.begin(), slicePart.begin(), slicePart.end());

					//silly for loop
					for (int i=0; i<2; i++) {
						auto& ch=toAdd[i];
						ch.pos=f.pos;
						ch.rot=f.rot;
						ch.sliced=true;

						float heading=2*PI*RANDOM;
						float speed=50+70*RANDOM;
						ch.vel+=polarToCartesian(heading, speed);
						ch.rotVel=(i*2-1)*f.rotVel*(1+.5f*RANDOM);

						//center around average
						Float2 avg=ch.getLocalizedAvg();
						for (auto& p:ch.pts) p-=avg;
						ch.pos+=rotVec(avg, ch.rot);

						Fruits.push_front(ch);
					}

					{//scoring
						float area0=toAdd[0].getArea();
						float totalArea=area0+toAdd[1].getArea();
						float t=area0/totalArea;
						int pct=100*t;
						int diff=abs(pct-50);
						std::string rating=
							diff<7?"awesome!":
							diff<15?"good":
							diff<30?"bad":
							"terrible";
						std::cout<<"split: "<<pct<<'/'<<100-pct<<". "<<rating<<'\n';
					}

					{//add particles
						float avgRad=0;
						for (const auto& p:f.pts) avgRad+=length(p);
						avgRad/=f.pts.size();
						int num=60+90*RANDOM;
						for (int i=0; i<num; i++) {
							float angle=2*PI*RANDOM;
							float rad=avgRad*RANDOM;
							float rot=2*PI*RANDOM;
							float speed=15+25*RANDOM;
							float lifespan=1+2*RANDOM;
							effects.push_back(FX(f.pos+polarToCartesian(angle, rad), polarToCartesian(rot, speed), lifespan));
						}
					}

					it=Fruits.erase(it);
				} else it++;
			}

			//manage slice length
			if (slice.size()>50) slice.erase(slice.begin());
		}
		sliceTimer+=deltaTime;

		//every now and then throw a Fruit from the bottom
		if (FruitTimer>1) {
			FruitTimer-=.6f+.6f*RANDOM;

			float heading=PI*.25f*(5+2*RANDOM);
			float speed=240+100*RANDOM;
			float rot=2*PI*RANDOM;
			float rotVel=PI*(1+RANDOM);
			Fruit fTemp(Float2(width*(.3f+.4f*RANDOM), height), polarToCartesian(heading, speed), rot, rotVel);
			int num=5+7*RANDOM;
			for (int i=0; i<num; i++) {
				float angle=2*PI*i/num;
				float rad=20+30*RANDOM;
				fTemp.pts.push_back(Float2(cosf(angle), sinf(angle))*rad);
			}
			Fruits.push_back(fTemp);
		}
		FruitTimer+=deltaTime;

		//Fruit dynamics(space pause debug)
		if (!Keyboard::isKeyPressed(Keyboard::Space)) for (auto it=Fruits.begin(); it!=Fruits.end();) {
			auto& f=*it;
			f.rotVel*=1-.3f*deltaTime;
			f.accelerate(grav);
			f.update(deltaTime);

			//remove if offscreen
			AABB fBounds=f.getAABB();
			if (fBounds.max.x<0||fBounds.min.x>width||fBounds.min.y>height) it=Fruits.erase(it);
			else it++;
		}

		//particle dynamics
		for (auto it=effects.begin(); it!=effects.end();) {
			auto& f=*it;

			f.accelerate(grav);
			f.update(deltaTime);

			//remove if too old or offscreen
			if (f.isDead()||!bounds.containsPt(f.pos)) it=effects.erase(it);
			else it++;
		}
		//end UPDATE

		//start RENDER
		renderTex.clear(Color::White);
		if (toSlice) for (int i=1; i<slice.size(); i++) {
			drawLine(slice[i-1], slice[i], Color(190, 190, 20));
		}

		//draw particles
		for (const auto& f:effects) {
			float pct=1-f.age/f.lifespan;
			Color smoke(20*pct, 190*pct, 50*pct, 130);
			fillCircle(f.pos, 2, smoke);
		}

		//draw Fruits
		for (const auto& f:Fruits) {
			Color colToUse=f.sliced?Color(20, 190, 50):Color(20, 20, 190);

			int len=f.pts.size();
			for (int i=0; i<len; i++) {
				Float2 a=f.globalize(f.pts[i]);
				Float2 b=f.globalize(f.pts[(i+1)%len]);
				drawLine(a, b, colToUse);
			}
		}

		renderTex.display();
		//end RENDER

		//start POST_PROCESS
		Texture renderedTex=renderTex.getTexture();
		paperShader.setUniform("MainTex", renderedTex);
		window.clear();
		window.draw(shaderSprite, &paperShader);
		window.display();
		//end POST_PROCESS
	}

	return 0;
}