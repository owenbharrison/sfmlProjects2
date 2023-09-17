#include <SFML/Graphics.hpp>
using namespace sf;

#include <list>
#include <iostream>

#include "cst.h"
#include "spr.h"

#include "io/stopwatch.h"

float clamp(float x, float a, float b) {
	if (x<a) return a;
	if (x>b) return b;
	return x;
}

inline float rand01() {
	return rand()/float(RAND_MAX);
}

int main() {
	srand(time(NULL));

	//sfml setup
	const unsigned int width=720;
	const unsigned int height=480;
	const unsigned int numSubSteps=4;
	float2 screenRes(width, height);
	RenderWindow renderWindow(VideoMode(width, height), "", Style::Titlebar|Style::Close);
	renderWindow.setFramerateLimit(120);
	renderWindow.setKeyRepeatEnabled(false);
	Clock deltaClock;
	stopwatch updateWatch, renderWatch;

	//world properties
	float worldScale=1;
	float2 worldOffset=screenRes/2;
	bool panDown=false;
	float2 panStart;

	//physics setup
	const float2 gravity(0, 100);
	const aabb worldBounds(25-worldOffset, worldOffset-25);
	const aabb screenBounds(0, screenRes);
	std::list<ptc> particles;
	std::list<cst> constraints;
	std::list<spr> springs;
	const float cellSize=2.5f*ptc::defaultRad;
	const float2 cellNum=(worldBounds.max-worldBounds.min)/cellSize;
	const int cellNumX=ceilf(cellNum.x), cellNumY=ceilf(cellNum.y);
	auto ptcGrid=new std::vector<ptc*>[cellNumX*cellNumY];

	//element addition, mouse logic
	ptc* cStart=nullptr;
	ptc* sStart=nullptr;
	ptc* pHeld=nullptr;
	bool addingForm=false;
	std::vector<float2> formOutline;
	float2 gridStart;

	//space transform functions + helper
	auto worldToScreen=[&worldScale, &worldOffset] (float2 p) { return p*worldScale+worldOffset; };
	auto screenToWorld=[&worldScale, &worldOffset] (float2 p) { return (p-worldOffset)/worldScale; };
	auto drawDottedLine=[&renderWindow, &screenBounds] (float2 st, float2 en, float step, Color col=Color::White) {
		if (!screenBounds.clipLine(st, en)) return;

		float2 enst=en-st;
		float dist=length(enst);
		int x=dist/step;
		int n=(x+1)*.5f;
		VertexArray lines(Lines, n*2);
		for (int i=0; i<n; i++) {
			float tA=float(2*i)/x, tB=float(1+2*i)/x;
			float2 a=st+tA*enst, b=st+tB*enst;
			lines[i*2]=Vertex(Vector2f(a.x, a.y), col);
			lines[i*2+1]=Vertex(Vector2f(b.x, b.y), col);
		}
		renderWindow.draw(lines);
	};

	//main loop
	while (renderWindow.isOpen()) {
		//TIMING
		float actualDeltaTime=deltaClock.restart().asSeconds();
		float deltaTime=MIN(1/60.f, actualDeltaTime);

		//update title
		auto ptcStr=std::to_string(particles.size());
		auto fpsStr=std::to_string(int(1/actualDeltaTime));
		renderWindow.setTitle("Physics Drawer w/ "+ptcStr+"ptcs @ "+fpsStr+"fps");

		//USER_INPUT
		Vector2i mp=Mouse::getPosition(renderWindow);
		const float2 screenMousePos(mp.x, mp.y);
		const float2 worldMousePos=screenToWorld(screenMousePos);

		//which particle is the mouse on
		ptc* pHover=nullptr;
		for (auto& p:particles) {
			if (length(p.pos-worldMousePos)<p.rad) {
				pHover=&p;
				break;
			}
		}
		for (Event event; renderWindow.pollEvent(event);) {
			if (event.type==Event::Closed) renderWindow.close();

			//keyboard
			if (event.type==Event::KeyPressed) {
				switch (event.key.code) {
					case Keyboard::Home://reset world center and scale
					{
						worldScale=1;
						worldOffset=screenRes/2;
						break;
					}
					case Keyboard::A://add particles
					{
						particles.push_back(ptc(worldMousePos));
						break;
					}
					case Keyboard::C: { cStart=pHover; break; }
					case Keyboard::S: { sStart=pHover; break; }
					case Keyboard::F:
					{
						addingForm=true;
						formOutline.push_back(worldMousePos);
						break;
					}
					case Keyboard::G:
					{
						gridStart=worldMousePos;
						break;
					}
				}
			}
			if (event.type==Event::KeyReleased) {
				switch (event.key.code) {
					case Keyboard::C://add constraints
					{
						if (cStart!=nullptr&&pHover!=nullptr&&cStart!=pHover) {
							constraints.push_back(cst(*cStart, *pHover));
							cStart=nullptr;
						}
						break;
					}
					case Keyboard::S://add springs
					{
						if (sStart!=nullptr&&pHover!=nullptr&&sStart!=pHover) {
							springs.push_back(spr(*sStart, *pHover));
							sStart=nullptr;
						}
						break;
					}
					case Keyboard::F://add form
					{
						addingForm=false;

						//ray casting algorithm
						auto insideForm=[&formOutline] (float2 c) {
							int i, num, sz=formOutline.size();
							for (i=0, num=0; i<sz; i++) {
								//cyclic shape
								float2 a=formOutline[i], b=formOutline[(i+1)%sz];
								float2 ab=a-b, ac=a-c;
								float2 tu=float2(ac.y, ac.x*ab.y-ac.y*ab.x)/ab.y;
								num+=tu.x>=0&&tu.x<=1&&tu.y>=0;
							}
							//is num odd
							return num%2;
						};

						//find bounds of form
						float2 min(INFINITY), max=-min;
						for (const auto& p:formOutline) {
							if (p.x<min.x) min.x=p.x;
							if (p.y<min.y) min.y=p.y;
							if (p.x>max.x) max.x=p.x;
							if (p.y>max.y) max.y=p.y;
						}

						//setup ptc grid
						float sz=3*ptc::defaultRad;
						float2 start=float2(floorf(min.x/sz), floorf(min.y/sz))*sz;
						float2 dim=(max-start)/sz;
						int w=ceilf(dim.x), h=ceilf(dim.y);
						ptc** grid=new ptc*[w*h] {nullptr};
						for (int i=0; i<w; i++) {
							for (int j=0; j<h; j++) {
								float2 pos=start+sz*float2(i, j);
								float rad=sz*.25f;
								if (!worldBounds.overlaps({pos-rad, pos+rad})) continue;
								if (!insideForm(pos)) continue;

								particles.push_back(ptc(pos, rad));
								grid[i+j*w]=&particles.back();
							}
						}

						//connect springs throughout grid
						for (int i=0; i<w-1; i++) {
							for (int j=0; j<h-1; j++) {
								auto tl=grid[i+j*w], tr=grid[i+1+j*w];
								auto bl=grid[i+(j+1)*w], br=grid[i+1+(j+1)*w];
								bool a=tl!=nullptr, b=tr!=nullptr;
								bool c=bl!=nullptr, d=br!=nullptr;
								//connect to adj particles
#define NEWSPR(a, b) springs.push_back(spr(*a, *b));
								if (a) {
									if (b) NEWSPR(tl, tr);
									if (c) NEWSPR(tl, bl);
									if (d) NEWSPR(tl, br);
								}
								if (b) {
									if (c) NEWSPR(tr, bl);
									if (d) NEWSPR(tr, br);
								}
								if (c&&d) NEWSPR(bl, br)
#undef NEWSPR
							}
						}
						delete[] grid;

						formOutline.clear();
						break;
					}
					case Keyboard::G:
					{
						aabb gridBounds(gridStart, worldMousePos);
						for (float x=gridBounds.min.x; x<gridBounds.max.x; x+=2*ptc::defaultRad) {
							for (float y=gridBounds.min.y; y<gridBounds.max.y; y+=2*ptc::defaultRad) {
								float2 pos(x, y);
								float rad=(.5f+.5f*rand01())*ptc::defaultRad;

								particles.push_back(ptc(pos, rad));
							}
						}
						break;
					}
				}
			}

			//mouse
			if (event.type==Event::MouseButtonPressed) {
				switch (event.mouseButton.button) {
					case Mouse::Left: pHeld=pHover; break;
					case Mouse::Middle://start pan
					{
						panDown=true;
						panStart=screenMousePos;
						break;
					}
				}
			}
			if (event.type==Event::MouseButtonReleased) {
				switch (event.mouseButton.button) {
					case Mouse::Left: pHeld=nullptr; break;
					case Mouse::Middle://end pan
					{
						panDown=false;
						worldOffset+=screenMousePos-panStart;
						break;
					}
				}
			}
			if (event.type==Event::MouseWheelScrolled) {
				//zoom relative to mouse
				float sclCoeff=1+.1f*event.mouseWheelScroll.delta;
				worldOffset=screenMousePos+sclCoeff*(worldOffset-screenMousePos);
				worldScale*=sclCoeff;
			}
		}
		//set temp pan
		float2 panDiff=screenMousePos-panStart;
		if (panDown) {
			formOutline={worldMousePos};
			worldOffset+=panDiff;
		}

		//mouse particle dynamics
		if (pHeld!=nullptr) {
			ptc pTemp(worldMousePos, 1);
			spr sTemp(*pHeld, pTemp);
			sTemp.restLen=0;
			sTemp.update(deltaTime);
		}

		//add points to form hull intermittently
		if (addingForm) {
			float2 last=formOutline.back();
			if (length(last-worldMousePos)>ptc::defaultRad) {
				formOutline.push_back(worldMousePos);
			}
		}

		//element deletion
		if (Keyboard::isKeyPressed(Keyboard::X)) {
			pHeld=nullptr, cStart=nullptr, sStart=nullptr;

			//check all particles
			float radModifier=1+4*Keyboard::isKeyPressed(Keyboard::LShift);
			for (auto pit=particles.begin(); pit!=particles.end();) {
				auto& p=*pit;

				//check mouse particle overlap
				if (length(p.pos-worldMousePos)>p.rad*radModifier) {
					pit++;
					continue;
				}

				//remove all constraints
				for (auto cit=constraints.begin(); cit!=constraints.end();) {
					auto& c=*cit;
					if (&p==c.a||&p==c.b) cit=constraints.erase(cit);
					else cit++;
				}

				//remove all springs
				for (auto sit=springs.begin(); sit!=springs.end();) {
					auto& s=*sit;
					if (&p==s.a||&p==s.b) sit=springs.erase(sit);
					else sit++;
				}

				//remove ptc
				pit=particles.erase(pit);
			}

			//check all constraints
			for (auto cit=constraints.begin(); cit!=constraints.end();) {
				auto& c=*cit;
				float2 a=c.a->pos, ba=c.b->pos-a, pa=worldMousePos-a;
				float t=clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
				float2 pt=a+t*ba;
				if (length(pt-worldMousePos)<c.rad*radModifier) {
					cit=constraints.erase(cit);
				} else cit++;
			}
		}
		const bool delKey=Keyboard::isKeyPressed(Keyboard::Delete);
		if (delKey||Keyboard::isKeyPressed(Keyboard::End)) {
			cStart=nullptr, sStart=nullptr;
			constraints.clear();
			springs.clear();

			if (delKey) {
				pHeld=nullptr;
				particles.clear();
			}
		}

		//UPDATE
		updateWatch.start();
		float subDeltaTime=deltaTime/numSubSteps;
		if (particles.size()>0&&!Keyboard::isKeyPressed(Keyboard::LControl)) {
			for (int k=0; k<numSubSteps; k++) {
				//update connectors
				for (auto& c:constraints) c.update();
				for (auto& s:springs) s.update(deltaTime);

				//particle-particle collisions
				for (int i=0; i<cellNumX*cellNumY; i++) ptcGrid[i].clear();
				//fill grid with particles
				for (auto& p:particles) {
					float2 xy=(p.pos-worldBounds.min)/cellSize;
					int i=xy.x, j=xy.y;

					//is cell valid	
					if (i<0||i>=cellNumX||j<0||j>=cellNumY) continue;

					ptcGrid[i+j*cellNumX].push_back(&p);
				}
				//for every cell
				for (int x=0; x<cellNumX; x++) {
					for (int y=0; y<cellNumY; y++) {
						auto& curr=ptcGrid[x+y*cellNumX];

						//check all neighbors
						for (int dx=-1; dx<=1; dx++) {
							for (int dy=-1; dy<=1; dy++) {
								int i=x+dx, j=y+dy;

								//is cell valid
								if (i<0||i>=cellNumX||j<0||j>=cellNumY) continue;

								auto& adj=ptcGrid[i+j*cellNumX];
								//query both containers
								for (auto& a:curr) {
									for (auto& b:adj) {
										//dont check self
										if (a==b) continue;

										//narrow phase
										cst cTemp(*a, *b, 0);
										float totalRad=a->rad+b->rad;
										if (cTemp.restLen>totalRad) continue;

										//resolution
										cTemp.restLen=totalRad;
										cTemp.update();
									}
								}
							}
						}
					}
				}

				//particle-constraint collisons
				for (auto& p:particles) {
					for (auto& c:constraints) {
						//dont check "self"
						if (&p==c.a||&p==c.b) continue;

						//closest pt on constraint
						float2 a=c.a->pos, ba=c.b->pos-a, pa=p.pos-a;
						float t=clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
						ptc pTemp(a+t*ba, 0);
						float cMass=c.a->mass+c.b->mass;
						pTemp.mass=cMass;

						//narrow phase
						cst cTemp(p, pTemp, 0);
						float totalRad=p.rad+c.rad;
						if (cTemp.restLen>totalRad) continue;

						//resolve
						cTemp.restLen=totalRad;
						float totalMass=p.mass+cMass;
						float2 f=cTemp.getCorrection()/totalMass;
						p.pos+=cMass*f;
						c.a->pos-=(1-t)*(c.b->mass/cMass)*p.mass*f;
						c.b->pos-=t*(c.a->mass/cMass)*p.mass*f;
					}
				}

				//final update step
				for (auto& p:particles) {
					//grav: F=mg
					p.applyForce(p.mass*gravity);

					//"integrate"
					p.update(subDeltaTime);

					//bounds detection
					p.checkAABB(worldBounds);
				}
			}
		}
		updateWatch.stop();

		//RENDER
		renderWatch.start();
		renderWindow.clear(Color(0x323232ff));

		//draw bounds
		{
			float2 min=worldToScreen(worldBounds.min), max=worldToScreen(worldBounds.max);
			float2 mx(min.x, max.y), xm(max.x, min.y);
			drawDottedLine(min, mx, 5, Color::Red);
			drawDottedLine(mx, max, 5, Color::Red);
			drawDottedLine(max, xm, 5, Color::Red);
			drawDottedLine(xm, min, 5, Color::Red);
		}

		//draw pHeld
		if (pHeld!=nullptr) {
			drawDottedLine(worldToScreen(pHeld->pos), screenMousePos, 3, Color(0xa5a5a5ff));
		}

		//draw form
		{
			const int sz=formOutline.size();
			VertexArray lines(LineStrip, sz+1);
			for (int i=0; i<sz; i++) {
				float2 pos=worldToScreen(formOutline[i]);
				lines[i]=Vertex(Vector2f(pos.x, pos.y), Color::Cyan);
			}
			lines[sz]=lines[0];
			renderWindow.draw(lines);
		}

		//draw springs
		for (const auto& s:springs) {
			float2 a=s.a->pos, b=s.b->pos;
			drawDottedLine(worldToScreen(a), worldToScreen(b), 5);
		}

		//draw constraints
		//TODO: implement culling
		{
			RectangleShape line;
			line.setFillColor(Color::White);
			for (const auto& c:constraints) {
				float2 a=worldToScreen(c.a->pos), ba=worldToScreen(c.b->pos)-a;
				float rad=c.rad*worldScale;
				line.setOrigin(Vector2f(0, rad));
				line.setSize(Vector2f(length(ba), 2*rad));
				line.setPosition(Vector2f(a.x, a.y));
				line.setRotation(atan2f(ba.y, ba.x)*180/PI);
				renderWindow.draw(line);
			}
		}

		//draw particles
		{
			CircleShape circ;
			for (auto& p:particles) {
				float2 pos=worldToScreen(p.pos);
				float rad=p.rad*worldScale;
				//culling
				if (!screenBounds.overlaps({pos-rad, pos+rad})) continue;

				circ.setRadius(rad);
				circ.setOrigin(Vector2f(rad, rad));
				circ.setPosition(Vector2f(pos.x, pos.y));
				circ.setFillColor(Color::White);
				circ.setOutlineColor(Color::Black);
				circ.setOutlineThickness(-2);
				renderWindow.draw(circ);
			}
		}
		renderWatch.stop();

		//debug
		if (Keyboard::isKeyPressed(Keyboard::D)) {
			float updateTime=updateWatch.getMicros();
			float totalTime=updateTime+renderWatch.getMicros();
			int updateWidth=updateTime/totalTime*width;

			RectangleShape rect(Vector2f(updateWidth, 40));
			rect.setFillColor(Color::Red);
			renderWindow.draw(rect);

			rect.setPosition(Vector2f(updateWidth, 0));
			rect.setSize(Vector2f(width-updateWidth, 40));
			rect.setFillColor(Color::Green);
			renderWindow.draw(rect);
		}
		renderWindow.display();

		//reset temp pan
		if (panDown) worldOffset-=panDiff;
	}
	delete[] ptcGrid;

	return 0;
}