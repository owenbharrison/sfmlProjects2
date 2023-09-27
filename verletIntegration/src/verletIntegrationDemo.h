#include <iostream>
#include <fstream>
#include <sstream>

#include "engine/gameEngine.h"
using namespace sf;

#include "phys/physicsSolver.h"

#include "io/stopwatch.h"

inline float rand01() {
	return rand()/float(RAND_MAX);
}

inline float clamp(float t, float a, float b) {
	if (t<a) return a;
	if (t>b) return b;
	return t;
}

inline float snapTo(float a, float b) {
	return roundf(a/b)*b;
}

Float2 getClosePt(Float2 a, Float2 b, Float2 p) {
	Float2 ba=b-a, pa=p-a;
	float t=clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
	return a+t*ba;
}

const std::vector<Color> stressGradient{
	Color::Blue,
	Color::Cyan,
	Color::Green,
	Color::Yellow,
	Color::Red
};
Color gradient(float t, std::vector<Color> arr) {
	t=clamp(t, 0, 1);
	if (t==0) return arr.front();
	if (t==1) return arr.back();

	size_t n=arr.size();
	float index=t*(n-1);
	size_t whole=index;
	float dec=index-whole;
	Color a=arr[whole], ba=arr[whole+1]-a;
	return a+Color(dec*ba.r, dec*ba.g, dec*ba.b);
}

struct VerletIntegrationDemo : GameEngine {
	const size_t numSubSteps=3;
	float deltaTime=0;
	float totalDeltaTime=0;

	//physics members
	std::list<Particle> particles;
	std::list<Constraint> constraints;
	std::list<Spring> springs;
	PhysicsSolver solver;
	AABB bounds;

	//simulation states
	bool running=true;
	Particle* mouseParticle=nullptr;
	Particle* hoverParticle=nullptr;
	Particle* constraintStart=nullptr;
	Particle* springStart=nullptr;
	bool addingBunch=false;
	Float2 bunchStart;
	bool addingJelly=false;
	std::vector<Float2> jellyOutline;
	bool toDebug=false;

	Stopwatch updateWatch, renderWatch, postProcessWatch;

	Shader crtShader, blueprintShader;
	Texture shaderTex;

	VerletIntegrationDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

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

	void fillPie(Float2 p, float r, float start, float end, Color col=Color::White) {
		const size_t num=24;
		VertexArray pie(sf::TriangleFan, num+2);
		pie[0]=Vertex(Vector2f(p.x, p.y), col);
		for (size_t i=0; i<=num; i++) {
			float pct=float(i)/num;
			float angle=start+pct*(end-start);
			Float2 dir(cosf(angle), sinf(angle));
			Float2 pt=p+r*dir;
			pie[i+1]=Vertex(Vector2f(pt.x, pt.y), col);
		}
		draw(pie);
	}

	bool init() override {
		std::cout<<"Don't close this console, this is where you import/export files.\n";

		srand(time(NULL));

		//physics setup
		solver=PhysicsSolver(&particles, &constraints, &springs);
		solver.initGravity(Float2(0, 320));
		solver.initBounds(bounds=AABB(0, Float2(width, height)));
		if (!solver.initSpacialHash(2.1f*Particle::defRad)) return false;

		//shader setup
		if (!crtShader.loadFromFile("shader/crt.glsl", Shader::Fragment)) return false;
		crtShader.setUniform("Resolution", Vector2f(width, height));
		if (!blueprintShader.loadFromFile("shader/blueprint2.glsl", Shader::Fragment)) return false;
		blueprintShader.setUniform("Resolution", Vector2f(width, height));
		blueprintShader.setUniform("Size", 2*Particle::defRad);
		if (!shaderTex.create(width, height)) return false;

		return true;
	}

	void input() override {
		toDebug=Keyboard::isKeyPressed(Keyboard::F12);

		//add points to jelly hull intermittently
		if (addingJelly) {
			Float2 last=jellyOutline.back();
			if (length(last-mousePos)>Particle::defRad) {
				jellyOutline.push_back(mousePos);
			}
		}

		//snap mouse to blueprint grid
		if (Keyboard::isKeyPressed(Keyboard::LAlt)) {
			Float2 center(width/2, height/2);
			Float2 diff=mousePos-center-Particle::defRad;
			mousePos=Float2(
				snapTo(diff.x, 2*Particle::defRad),
				snapTo(diff.y, 2*Particle::defRad)
			)+center+Particle::defRad;
		}

		//remove single element
		if (Keyboard::isKeyPressed(Keyboard::X)) {
			mouseParticle=nullptr;
			constraintStart=nullptr;
			springStart=nullptr;

			int radFactor=1+4*Keyboard::isKeyPressed(Keyboard::LControl);
			//for every Particle
			for (auto pit=particles.begin(); pit!=particles.end();) {
				auto& p=*pit;
				//if touching mouse
				if (length(mousePos-p.pos)<radFactor*p.rad) {
					//remove all connecting connectors
					for (auto cit=constraints.begin(); cit!=constraints.end();) {
						auto& c=*cit;
						if (&p==c.a||&p==c.b) cit=constraints.erase(cit);
						else cit++;
					}
					for (auto sit=springs.begin(); sit!=springs.end();) {
						auto& s=*sit;
						if (&p==s.a||&p==s.b) sit=springs.erase(sit);
						else sit++;
					}

					//remove Particle
					pit=particles.erase(pit);
				} else pit++;
			}
			//for every Constraint
			for (auto cit=constraints.begin(); cit!=constraints.end();) {
				auto& c=*cit;
				Float2 ba=c.b->pos-c.a->pos;
				Float2 closePt=getClosePt(c.a->pos, c.b->pos, mousePos);
				if (length(mousePos-closePt)<c.rad*radFactor) {
					cit=constraints.erase(cit);
				} else cit++;
			}
			//for every Spring
			for (auto sit=springs.begin(); sit!=springs.end();) {
				auto& s=*sit;
				Float2 closePt=getClosePt(s.a->pos, s.b->pos, mousePos);
				float rad=MIN(s.a->rad, s.b->rad);
				if (length(mousePos-closePt)<radFactor*rad) {
					sit=springs.erase(sit);
				} else sit++;
			}
		}

		//which particle is unde the mouse?
		hoverParticle=nullptr;
		for (auto& p:particles) {
			if (length(mousePos-p.pos)<p.rad) {
				hoverParticle=&p;
				break;
			}
		}
	}

	void onMouseDown(Mouse::Button button) override {
		switch (button) {
			case Mouse::Left:
			{//set mouse particle
				mouseParticle=hoverParticle;
				break;
			}
			case Mouse::Middle:
			{//add/remove particle from collision detection(ghosting)
				if (mouseParticle) mouseParticle->ghosted^=true;
				else {
					if (hoverParticle) hoverParticle->ghosted^=true;

					Constraint* cToGhost=nullptr;
					for (auto& c:constraints) {
						if (length(mousePos-c.a->pos)<c.a->rad) continue;
						if (length(mousePos-c.b->pos)<c.b->rad) continue;

						Float2 closePt=getClosePt(c.a->pos, c.b->pos, mousePos);
						if (length(mousePos-closePt)<c.rad) cToGhost=&c;
					}
					if (cToGhost) cToGhost->ghosted^=true;
				}
				break;
			}
			case Mouse::Right:
			{//add/remove particle from dynamics(locking)
				if (mouseParticle) mouseParticle->locked^=true;
				else {
					Particle* toLock=nullptr;
					for (auto& p:particles) {
						if (length(mousePos-p.pos)<p.rad) toLock=&p;
					}
					if (toLock) toLock->locked^=true;
				}
				break;
			}
		}
	}

	void onMouseUp(Mouse::Button button) override {
		if (button==Mouse::Left) mouseParticle=nullptr;
	}

	void onKeyDown(Keyboard::Key key) override {
		switch (key) {
		//pause/play
			case Keyboard::Space:
			{
				mouseParticle=nullptr;
				constraintStart=nullptr;
				springStart=nullptr;
				running^=true;
				break;
			}
			//add elements
			case Keyboard::A: particles.push_back(Particle(mousePos)); break;
			case Keyboard::C:
			{
				constraintStart=hoverParticle;
				break;
			}
			case Keyboard::S:
			{
				springStart=hoverParticle;
				break;
			}
			case Keyboard::B:
			{
				addingBunch=true;
				bunchStart=mousePos;
				break;
			}
			case Keyboard::J:
			{
				addingJelly=true;
				jellyOutline.push_back(mousePos);
				break;
			}
			//remove elements
			case Keyboard::End:
			{
				constraints.clear();
				springs.clear();
				break;
			}
			case Keyboard::Delete:
			{
				bool toClear=particles.size()+constraints.size()+springs.size()==0;
				if (!toClear) {
					std::cout<<"Do you want to remove everything?";
					char response; std::cin>>response;
					if (response=='y'||response=='Y') toClear=true;
				}
				if (toClear) {
					constraints.clear();
					springs.clear();
					particles.clear();
				}
				break;
			}
			//import/export
			case Keyboard::I:
			{
				bool toLoad=particles.size()+constraints.size()+springs.size()==0;
				if (!toLoad) {
					std::cout<<"Do you want to override current?";
					char response; std::cin>>response;
					if (response=='y'||response=='Y') toLoad=true;
					else std::cout<<"Import aborted.\n";
				}
				if (toLoad) {
					//override current config
					constraints.clear();
					springs.clear();
					particles.clear();

					auto getParticleById=[this] (int id) {
						return std::find_if(particles.begin(), particles.end(), [&id] (const Particle& p) {
							return p.id==id;
						});
					};

					//parse line by line
					std::cout<<"Input filename to import: ";
					std::string filename; std::cin>>filename;
					std::ifstream loadFile(filename);
					if (loadFile) {
						int id=0;
						for (std::string line; getline(loadFile, line);) {
							std::stringstream lineStream(line);
							char junk;
							switch (line[0]) {
								case 'p':
								{//parse particles
									float x, y, rad;
									bool locked, ghosted;
									lineStream>>junk>>x>>y>>rad>>locked>>ghosted;

									Particle p(Float2(x, y), rad);
									p.locked=locked;
									p.ghosted=ghosted;
									p.id=id++;
									particles.push_back(p);
									break;
								}
								case 'c':
								{//parse constraints
									int i0, i1;
									float rad;
									bool ghosted;
									lineStream>>junk>>i0>>i1>>rad>>ghosted;

									auto it0=getParticleById(i0);
									auto it1=getParticleById(i1);
									if (it0!=particles.end()&&it1!=particles.end()) {
										Constraint c(*it0, *it1, rad);
										c.ghosted=ghosted;
										constraints.push_back(c);
									}
									break;
								}
								case 's':
								{//parse springs
									int i0, i1;
									float len, stiff, damp;
									lineStream>>junk>>i0>>i1>>len>>stiff>>damp;

									auto it0=getParticleById(i0);
									auto it1=getParticleById(i1);
									if (it0!=particles.end()&&it1!=particles.end()) {
										Spring s(*it0, *it1, stiff, damp);
										s.restLen=len;
										springs.push_back(s);
									}
									break;
								}
							}
						}

						std::cout<<"Successfully imported "
							<<particles.size()<<" particles, "
							<<constraints.size()<<" constraints, and "
							<<springs.size()<<" springs from "<<filename<<'\n';
					} else std::cout<<"Invalid filename.\n";
					loadFile.close();
				}
				break;
			}
			case Keyboard::E:
			{
				if (particles.size()+constraints.size()+springs.size()==0) {
					std::cout<<"Nothing to export.\n";
				} else {
					std::cout<<"Input filename to export: ";
					std::string filename; std::cin>>filename;

					//check override
					std::ifstream fileCheck(filename);
					std::ofstream saveFile;
					bool toSave=false;
					if (fileCheck) {
						std::cout<<"File already exists. Do you want to override it?";
						char response; std::cin>>response;
						if (response=='y'||response=='Y') {
							saveFile.open(filename, std::ios::trunc);
							toSave=true;
						} else std::cout<<"Export aborted.\n";
					} else {
						saveFile.open(filename);
						toSave=true;
					}
					fileCheck.close();

					//somewhat like a obj file.
					if (toSave) {
						int id=0;
						//p [x] [y] [rad] [locked?] [ghosted?]
						for (auto& p:particles) {
							p.id=id++;
							saveFile<<"p "<<p.pos.x<<' '<<p.pos.y
								<<' '<<p.rad<<' '<<p.locked<<' '<<p.ghosted<<'\n';
						}

						//c [i0] [i1] [rad] [ghosted?]
						for (const auto& c:constraints) {
							saveFile<<"c "<<c.a->id<<' '<<c.b->id<<' '
								<<c.rad<<' '<<c.ghosted<<'\n';
						}
						//s [i1] [i1] [len] [stiff] [damp]
						for (const auto& s:springs) {
							saveFile<<"s "<<s.a->id<<' '<<s.b->id<<' '
								<<s.restLen<<' '<<s.stiffness<<' '<<s.damping<<'\n';
						}

						std::cout<<"Successfully exported "
							<<particles.size()<<" particles, "
							<<constraints.size()<<" constraints, and "
							<<springs.size()<<" springs to "<<filename<<'\n';
					}
					saveFile.close();
				}
				break;
			}
		}
	}

	void onKeyUp(Keyboard::Key key) override {
		switch (key) {
			//add elements
			case Keyboard::C:
			{
				if (constraintStart&&hoverParticle&&hoverParticle!=constraintStart) {
					constraints.push_back(Constraint(*constraintStart, *hoverParticle));
				}
				constraintStart=nullptr;
				break;
			}
			case Keyboard::S:
			{
				if (springStart&&hoverParticle&&hoverParticle!=springStart) {
					springs.push_back(Spring(*springStart, *hoverParticle));
				}
				springStart=nullptr;
				break;
			}
			case Keyboard::B:
			{
				addingBunch=false;
				AABB bunchBounds(bunchStart, mousePos);
				for (float x=bunchBounds.min.x; x<bunchBounds.max.x; x+=2*Particle::defRad) {
					for (float y=bunchBounds.min.y; y<bunchBounds.max.y; y+=2*Particle::defRad) {
						Float2 pos(rand01()-.5f+x, rand01()-.5f+y);
						float rad=(.5f+.5f*rand01())*Particle::defRad;
						if (!bounds.containsPt(pos)) continue;

						bool toAdd=true;
						for (const auto& p:particles) {
							if (length(pos-p.pos)<rad+p.rad) {
								toAdd=false;
								break;
							}
						}
						if (toAdd) particles.push_back(Particle(pos, rad));
					}
				}
				break;
			}
			case Keyboard::J:
			{
				addingJelly=false;

				//ray casting algorithm
				auto insideJelly=[this] (Float2 c) {
					size_t i, num, sz=jellyOutline.size();
					for (i=0, num=0; i<sz; i++) {
						//cyclic shape
						Float2 a=jellyOutline[i], b=jellyOutline[(i+1)%sz];
						Float2 ab=a-b, ac=a-c;
						Float2 tu=Float2(ac.y, ac.x*ab.y-ac.y*ab.x)/ab.y;
						num+=tu.x>=0&&tu.x<=1&&tu.y>=0;
					}
					//is num odd
					return num%2;
				};

				//find bounds of jelly
				Float2 min(INFINITY), max=-min;
				for (const auto& p:jellyOutline) {
					if (p.x<min.x) min.x=p.x;
					if (p.y<min.y) min.y=p.y;
					if (p.x>max.x) max.x=p.x;
					if (p.y>max.y) max.y=p.y;
				}

				//setup ptc grid
				float sz=3*Particle::defRad;
				Float2 start=Float2(floorf(min.x/sz), floorf(min.y/sz))*sz;
				Float2 dim=(max-start)/sz;
				size_t w=ceilf(dim.x), h=ceilf(dim.y);
				Particle** grid=new Particle*[w*h] {nullptr};
				for (size_t i=0; i<w; i++) {
					for (size_t j=0; j<h; j++) {
						Particle pTemp(start+sz*Float2(i, j), sz*.25f);
						if (!bounds.containsPt(pTemp.pos)) continue;
						if (!insideJelly(pTemp.pos)) continue;

						bool toAdd=true;
						for (const auto& p:particles) {
							if (length(pTemp.pos-p.pos)<pTemp.rad+p.rad) {
								toAdd=false;
								break;
							}
						}
						if (toAdd) {
							particles.push_back(pTemp);
							grid[i+j*w]=&particles.back();
						}
					}
				}

				//connect springs throughout grid
				for (size_t i=0; i<w-1; i++) {
					for (size_t j=0; j<h-1; j++) {
						auto tl=grid[i+j*w], tr=grid[i+1+j*w];
						auto bl=grid[i+(j+1)*w], br=grid[i+1+(j+1)*w];
						//connect to adj particles
						if (tl) {
							if (tr) springs.push_back(Spring(*tl, *tr));
							if (bl) springs.push_back(Spring(*tl, *bl));
							if (br) springs.push_back(Spring(*tl, *br));
						}
						if (tr) {
							if (bl) springs.push_back(Spring(*tr, *bl));
							if (br) springs.push_back(Spring(*tr, *br));
						}
						if (bl&&br) springs.push_back(Spring(*bl, *br));
					}
				}
				delete[] grid;

				jellyOutline.clear();
				break;
			}
		}
	}

	void update(float actualDeltaTime) override {
		if(toDebug) updateWatch.start();

		//update shader, set dt lower bound
		totalDeltaTime+=actualDeltaTime;
		blueprintShader.setUniform("Time", totalDeltaTime);
		deltaTime=MIN(actualDeltaTime, 1.f/60);

		//mouse particle dynamics
		if (mouseParticle) {
			if (running) {
				//more of a dragging motion.
				if (!mouseParticle->locked) {
					Float2 sub=mousePos-mouseParticle->pos;
					mouseParticle->pos+=sub*deltaTime;
				}
			} else {
				//set mouse Particle to mouse pos.
				mouseParticle->pos=mouseParticle->oldpos=mousePos;
				//reset connectors
				for (auto& c:constraints) {
					if (mouseParticle==c.a||mouseParticle==c.b) {
						c.restLen=length(c.a->pos-c.b->pos);
					}
				}
				for (auto& s:springs) {
					if (mouseParticle==s.a||mouseParticle==s.b) {
						s.restLen=length(s.a->pos-s.b->pos);
					}
				}
			}
		}

		//particles, constraints, springs
		if (running) {
			float subDeltaTime=deltaTime/numSubSteps;
			for (size_t k=0; k<numSubSteps; k++) {
				solver.update(subDeltaTime);
			}
		}

		if(toDebug) updateWatch.stop();
	}

	void render() override {
		if (toDebug) renderWatch.start();
		clear(running?Color(35, 35, 35):Color(0, 0, 0, 0));

		//draw bunch bounds
		if (addingBunch) {
			AABB bunchBounds(bunchStart, mousePos);
			Float2 nx(bunchBounds.min.x, bunchBounds.max.y);
			Float2 xn(bunchBounds.max.x, bunchBounds.min.y);
			drawThickLine(bunchBounds.min, nx, 1);
			drawThickLine(nx, bunchBounds.max, 1);
			drawThickLine(bunchBounds.max, xn, 1);
			drawThickLine(xn, bunchBounds.min, 1);
		}

		//draw jelly
		if (size_t sz=jellyOutline.size()) {
			for (size_t i=0; i<sz; i++) {
				Float2 a=jellyOutline[i], b=jellyOutline[(i+1)%sz];
				drawThickLine(a, b, 1, Color::Green);
			}
		}

		//draw mouse particle highlight
		if (running&&mouseParticle) {
			float factor=1.5f+(sinf(totalDeltaTime*3.)+1)/4;
			fillCircle(mouseParticle->pos, mouseParticle->rad*factor, Color(0x666666FF));
		}

		//draw constraints
		for (const auto& c:constraints) {
			int ghostVal=c.ghosted?127:255;
			Color ghostedWhite(255, 255, 255, ghostVal);
			if (running) {
				drawThickLine(c.a->pos, c.b->pos, c.rad, Color(0, 0, 0, ghostVal));
				drawThickLine(c.a->pos, c.b->pos, c.rad-2, ghostedWhite);
			} else {
				Float2 rAB=c.b->pos-c.a->pos;
				float tTheta=atan2f(rAB.y, rAB.x);
				float aTheta=asinf(c.rad/c.a->rad);
				Float2 blPt=c.a->pos+Float2(cosf(tTheta+aTheta), sinf(tTheta+aTheta))*c.a->rad;
				Float2 brPt=c.a->pos+Float2(cosf(tTheta-aTheta), sinf(tTheta-aTheta))*c.a->rad;

				float bTheta=asinf(c.rad/c.b->rad);
				Float2 tlPt=c.b->pos+Float2(cosf(tTheta-bTheta+PI), sinf(tTheta-bTheta+PI))*c.b->rad;
				Float2 trPt=c.b->pos+Float2(cosf(tTheta+bTheta+PI), sinf(tTheta+bTheta+PI))*c.b->rad;

				drawThickLine(blPt, tlPt, 1, ghostedWhite);
				drawThickLine(brPt, trPt, 1, ghostedWhite);
			}
		}

		//draw springs as dotted lines
		for (const auto& s:springs) {
			Float2 sub=s.b->pos-s.a->pos;
			float rad=.25f*(s.a->rad+s.b->rad);
			int num=MAX(4, int(s.restLen/rad));
			for (int i=0; i<num; i++) {
				int d=i-num/2;
				if (d%2) {
					Float2 a=s.a->pos+float(i)/num*sub;
					Float2 b=s.a->pos+float(i+1)/num*sub;
					if (running) {
						drawThickLine(a, b, .5f*rad, Color::White);
					} else drawThickLine(a, b, 1);
				}
			}
		}

		//draw particles
		float maxSpeed=1;
		for (const auto& p:particles) {
			Float2 vel=(p.pos-p.oldpos)/deltaTime;
			float speed=length(vel);
			if (speed>maxSpeed) maxSpeed=speed;
		}
		bool toShowSpeed=Keyboard::isKeyPressed(Keyboard::V);
		for (const auto& p:particles) {
			int ghostVal=p.ghosted?127:255;
			Float2 vel=(p.pos-p.oldpos)/deltaTime;
			float spd01=length(vel)/maxSpeed;
			//modeled after some arbitrary stress gradient
			Color speedCol=gradient(spd01, stressGradient);
			Color ghostedWhite(255, 255, 255, ghostVal);
			Color ghostedRed(255, 0, 0, ghostVal);
			if (running) {
				fillCircle(p.pos, p.rad, toShowSpeed?speedCol:ghostedWhite);
				drawCircle(p.pos, p.rad, Color(0, 0, 0, ghostVal));
				if (p.locked) {
					//draw x
					float v=p.rad/1.85f;
					drawThickLine(p.pos-v, p.pos+v, 1, ghostedRed);
					drawThickLine(p.pos+Float2(-v, v), p.pos+Float2(+v, -v), 1, ghostedRed);
				}
			} else drawCircle(p.pos, p.rad, p.locked?ghostedRed:toShowSpeed?speedCol:ghostedWhite);
		}

		//show particle "forces"
		if (Keyboard::isKeyPressed(Keyboard::F)) {
			for (auto& p:particles) {
				Float2 vel=(p.pos-p.oldpos)/deltaTime;
				drawArrow(p.pos, p.pos+vel, Color::Blue);
				Float2 force=p.oldforce/p.mass*deltaTime*10;
				float dpNormal=dot(vel, force)/(length(vel)*length(force));
				float theta01=acosf(dpNormal)/PI;

				float redVal=sqrtf(1-powf(theta01-1, 2));
				float greenVal=sqrtf(1-theta01*theta01);
				drawArrow(p.pos, p.pos+force, Color(255*redVal, 255*greenVal, 40));
			}
		}

		//show possible future constraint
		if (constraintStart) {
			int ybVal=128+127*sinf(totalDeltaTime*4);
			Color yellowBlack(ybVal, ybVal, 20);
			fillCircle(constraintStart->pos, constraintStart->rad, yellowBlack);
			drawArrow(constraintStart->pos, mousePos, yellowBlack);
		}

		//possible future spring
		if (springStart) {
			int mgVal=119*cosf(totalDeltaTime*5);
			Color magentaCyan(120+mgVal, 120-mgVal, 240);
			fillCircle(springStart->pos, springStart->rad, magentaCyan);
			drawArrow(springStart->pos, mousePos, magentaCyan);
		}

		if(toDebug) renderWatch.stop(), postProcessWatch.start();

		//no need for render textures!
		shaderTex.update(*this);
		clear();

		//post processing
		Shader* currShader=&(running?crtShader:blueprintShader);
		currShader->setUniform("MainTex", shaderTex);
		draw(sf::Sprite(shaderTex), currShader);


		if (toDebug) {
			postProcessWatch.stop();

			int updateTime=updateWatch.getMicros();
			int renderTime=renderWatch.getMicros();
			int postProcessTime=postProcessWatch.getMicros();
			float constant=2*PI/(updateTime+renderTime+postProcessTime);
			float a=constant*updateTime,
				b=a+constant*renderTime;
			float sz=80; Float2 center=Float2(width, height)-sz-10;
			fillPie(center, sz, 0, a, Color::Red);//update
			fillPie(center, sz, a, b, Color::Green);//render
			fillPie(center, sz, b, 2*PI, Color::Blue);//post process
		}
	}
};