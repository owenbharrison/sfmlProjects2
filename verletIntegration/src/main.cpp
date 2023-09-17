#include <list>

#include <iostream>
#include <fstream>
#include <sstream>

#include <SFML/Graphics.hpp>
using namespace sf;

#include "constraint.h"
#include "spring.h"

#include "io/stopwatch.h"

#define SUB_STEPS 3

#define COEFF_DRAG 10.89f

#define PARTICLE_RAD 9.2f
#define CONSTRAINT_RAD 4.87f
#define SPRING_STIFFNESS 560.3f*(PI*PARTICLE_RAD*PARTICLE_RAD)
#define SPRING_DAMPING 4.3f*(PI*PARTICLE_RAD*PARTICLE_RAD)

#define RANDOM (rand()/32767.f)

float clamp(float t, float a, float b) {
	if (t<a) return a;
	if (t>b) return b;
	return t;
}

float snapTo(float a, float b) {
	return roundf(a/b)*b;
}

Color lerpCol(Color a, Color b, float t) {
	return Color(
		a.r+t*(b.r-a.r),
		a.g+t*(b.g-a.g),
		a.b+t*(b.b-a.b)
	);
}

Color gradient(float t, std::vector<Color> arr) {
	t=clamp(t, 0, 1);
	if (t==0) return arr.front();
	if (t==1) return arr.back();

	int n=arr.size();
	float index=t*(n-1);
	int whole=index;
	float fract=index-whole;
	return lerpCol(arr[whole], arr[whole+1], fract);
}

float2 getClosePt(float2 a, float2 b, float2 p) {
	float2 ba=b-a, pa=p-a;
	float t=clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
	return a+t*ba;
}

int main() {
	std::cout<<"Don't close this console, this is where you import/export files.\n";

	srand(time(NULL));

	//sfml setup
	unsigned int width=720;
	unsigned int height=480;
	RenderWindow window(VideoMode(width, height), "", Style::Titlebar|Style::Close);
	window.setFramerateLimit(165);
	Clock deltaClock;
	float totalDeltaTime=0;

	//basic prog setup
	float2 grav(0, 320);
	aabb bounds(float2(0), float2(width, height));
	std::list<particle> particles;
	auto getParticleById=[&particles] (int id) {
		return std::find_if(particles.begin(), particles.end(), [&id] (const particle& p) {
			return p.id==id;
		});
	};
	std::list<constraint> constraints;
	std::list<spring> springs;

	//user input states
	bool running=true;
	bool pausePlay=false;

	bool addParticle=false;
	bool addConstraint=false;
	particle* constraintStart=nullptr;
	bool addSpring=false;
	particle* springStart=nullptr;

	bool addMess=false;
	float2 messStart;

	bool lockParticle=false;
	bool ghostParticle=false;

	bool mouseDown=false;
	particle* mouseParticle=nullptr;

	bool importFile=false;
	bool exportFile=false;

	//debug
	stopwatch uiWatch, updateWatch, renderWatch, postProcessWatch;

	//shader setup
	RenderTexture renderTex;
	if (!renderTex.create(width, height)) {
		std::cerr<<"Error: unable to create render texture.";
		exit(1);
	}
	Shader crtShader;
	if (!crtShader.loadFromFile("shader/crt.glsl", Shader::Fragment)) {
		std::cerr<<"Error: couldn't load shader";
		exit(1);
	}
	crtShader.setUniform("Resolution", Glsl::Vec2(width, height));
	Shader blueprintShader;
	if (!blueprintShader.loadFromFile("shader/blueprint.glsl", Shader::Fragment)) {
		std::cerr<<"Error: couldn't load shader";
		exit(1);
	}
	blueprintShader.setUniform("Resolution", Glsl::Vec2(width, height));
	blueprintShader.setUniform("Size", 2*PARTICLE_RAD);
	Texture shaderTex;
	if (!shaderTex.create(width, height)) {
		std::cerr<<"Error: couldn't create texture";
		exit(1);
	}
	Sprite shaderSprite(shaderTex);
	shaderSprite.setOrigin(Vector2f(0, height/2));
	shaderSprite.setPosition(Vector2f(0, height/2));
	shaderSprite.setScale(Vector2f(1, -1));

	//rendering ease of use functions
	auto drawLine=[&renderTex] (float2 a, float2 b, Color col=Color::White, float w=1) {
		float2 ba=b-a;
		RectangleShape line(Vector2f(length(ba), 2*w));
		line.setOrigin(Vector2f(0, w));
		line.setRotation(atan2(ba.y, ba.x)*180/PI);
		line.setPosition(Vector2f(a.x, a.y));

		line.setFillColor(col);
		renderTex.draw(line);
	};
	auto drawThickLine=[&renderTex] (float2 a, float2 b, float w, Color col=Color::White) {
		float2 ba=b-a;
		RectangleShape line(Vector2f(length(ba), 2*w));
		line.setOrigin(Vector2f(0, w));
		line.setRotation(atan2(ba.y, ba.x)*180/PI);
		line.setPosition(Vector2f(a.x, a.y));

		line.setFillColor(col);
		renderTex.draw(line);
	};
	auto drawCircle=[&renderTex] (float2 p, float r, Color col=Color::White) {
		CircleShape circ(r);
		circ.setOutlineThickness(-2);
		circ.setOutlineColor(col);
		circ.setFillColor(Color::Transparent);

		circ.setOrigin(Vector2f(r, r));
		circ.setPosition(Vector2f(p.x, p.y));
		renderTex.draw(circ);
	};
	auto fillCircle=[&renderTex] (float2 p, float r, Color col=Color::White) {
		CircleShape circ(r);
		circ.setFillColor(col);

		circ.setOrigin(Vector2f(r, r));
		circ.setPosition(Vector2f(p.x, p.y));
		renderTex.draw(circ);
	};
	auto drawArrow=[&drawLine] (float2 a, float2 b, Color col=Color::White) {
		float2 sub=b-a, sz=sub*.2f, tang{-sz.y, sz.x};
		float2 aSt=b-sz, lPt=aSt-tang*.5f, rPt=aSt+tang*.5f;
		//line
		drawLine(a, aSt, col);
		//triangle
		drawLine(lPt, rPt, col), drawLine(rPt, b, col), drawLine(lPt, b, col);
	};
	auto fillPie=[&window] (float2 p, float r, float start, float end, Color col=Color::White, int n=24) {
		VertexArray pie(sf::TriangleFan, n+2);
		pie[0]=Vertex(Vector2f(p.x, p.y), col);
		for (int i=0; i<=n; i++) {
			float pct=float(i)/n;
			float angle=start+pct*(end-start);
			float2 dir(cosf(angle), sinf(angle));
			float2 pt=p+r*dir;
			pie[i+1]=Vertex(Vector2f(pt.x, pt.y), col);
		}
		window.draw(pie);
	};
	while (window.isOpen()) {
		//mouse position
		Vector2i mp=Mouse::getPosition(window);
		float2 mousePos(mp.x, mp.y);

		//polling
		for (Event event; window.pollEvent(event);) {
			if (event.type==Event::Closed) window.close();
		}

		//timing
		float actualDeltaTime=deltaClock.restart().asSeconds();
		float deltaTime=MIN(actualDeltaTime, 1/60.f);
		totalDeltaTime+=actualDeltaTime;
		blueprintShader.setUniform("Time", totalDeltaTime);
		std::string fpsStr=std::to_string(int(1/actualDeltaTime))+"fps";
		window.setTitle("Verlet Integration 2 @ "+fpsStr);

		//start USER_INPUT
		bool toDebug=Keyboard::isKeyPressed(Keyboard::F12);
		if (toDebug) uiWatch.start();

		//this is a bad way of doing this. i should use more memory safe pointers...
		bool pausePlayTemp=Keyboard::isKeyPressed(Keyboard::Space);
		if (pausePlayTemp&&!pausePlay) {
			constraintStart=nullptr;
			springStart=nullptr;
			mouseParticle=nullptr;

			running^=true;
		}
		pausePlay=pausePlayTemp;

		//snap mouse to blueprint grid
		if (Keyboard::isKeyPressed(Keyboard::LAlt)) {
			float2 center(width/2, height/2);
			float2 diff=mousePos-center;
			mousePos=float2(
				snapTo(diff.x, 2*PARTICLE_RAD),
				snapTo(diff.y, 2*PARTICLE_RAD)
			)+center;
		}

		//mouse constraint dragging
		bool mouseDownTemp=Mouse::isButtonPressed(Mouse::Left);
		if (mouseDownTemp^mouseDown) {
			if (mouseDownTemp) {//on mouse down
				for (auto& p:particles) {
					if (length(p.pos-mousePos)<p.rad) {
						mouseParticle=&p;
					}
				}
			} else mouseParticle=nullptr;
		}
		mouseDown=mouseDownTemp;

		bool lockParticleTemp=Mouse::isButtonPressed(Mouse::Right);
		if (lockParticleTemp&&!lockParticle) {
			if (mouseParticle!=nullptr) mouseParticle->locked^=true;
			else {
				particle* toLock=nullptr;
				for (auto& p:particles) {
					if (length(mousePos-p.pos)<p.rad) toLock=&p;
				}
				if (toLock!=nullptr) toLock->locked^=true;
			}
		}
		lockParticle=lockParticleTemp;

		bool ghostParticleTemp=Mouse::isButtonPressed(Mouse::Middle);
		if (ghostParticleTemp&&!ghostParticle) {
			if (mouseParticle!=nullptr) mouseParticle->ghosted^=true;
			else {
				particle* pToGhost=nullptr;
				for (auto& p:particles) {
					if (length(mousePos-p.pos)<p.rad) pToGhost=&p;
				}
				if (pToGhost!=nullptr) pToGhost->ghosted^=true;

				constraint* cToGhost=nullptr;
				for (auto& c:constraints) {
					if (length(mousePos-c.a->pos)<c.a->rad) continue;
					if (length(mousePos-c.b->pos)<c.b->rad) continue;

					float2 closePt=getClosePt(c.a->pos, c.b->pos, mousePos);
					if (length(mousePos-closePt)<c.rad) cToGhost=&c;
				}
				if (cToGhost!=nullptr) cToGhost->ghosted^=true;
			}
		}
		ghostParticle=ghostParticleTemp;

		bool addParticleTemp=Keyboard::isKeyPressed(Keyboard::A);
		if (addParticleTemp&&!addParticle) {//on keydown
			particles.push_back(particle(mousePos, PARTICLE_RAD));
		}
		addParticle=addParticleTemp;

		bool addConstraintTemp=Keyboard::isKeyPressed(Keyboard::C);
		if (addConstraintTemp^addConstraint) {
			if (addConstraintTemp) {//on keydown
				for (auto& p:particles) {
					if (length(p.pos-mousePos)<p.rad) constraintStart=&p;
				}
			} else {
				if (constraintStart!=nullptr) {
					particle* constraintEnd=nullptr;
					for (auto& p:particles) {
						if (length(p.pos-mousePos)<p.rad) constraintEnd=&p;
					}
					if (constraintEnd!=nullptr&&constraintEnd!=constraintStart) {
						constraints.push_back(constraint(*constraintStart, *constraintEnd, CONSTRAINT_RAD));
					}
				}
				//reset
				constraintStart=nullptr;
			}
		}
		addConstraint=addConstraintTemp;

		bool addSpringTemp=Keyboard::isKeyPressed(Keyboard::S);
		if (addSpringTemp^addSpring) {
			if (addSpringTemp) {//on keydown
				for (auto& p:particles) {
					if (length(p.pos-mousePos)<p.rad) {
						springStart=&p;
					}
				}
			} else {//on key up
				if (springStart!=nullptr) {
					particle* springEnd=nullptr;
					for (auto& p:particles) {
						if (length(p.pos-mousePos)<p.rad) springEnd=&p;
					}
					if (springEnd!=nullptr&&springEnd!=springStart) {
						springs.push_back(spring(*springStart, *springEnd, SPRING_STIFFNESS, SPRING_DAMPING));
					}
				}
				//reset
				springStart=nullptr;
			}
		}
		addSpring=addSpringTemp;

		//remove single element
		if (Keyboard::isKeyPressed(Keyboard::X)) {
			mouseParticle=nullptr;
			constraintStart=nullptr;
			springStart=nullptr;

			int radFactor=1+4*Keyboard::isKeyPressed(Keyboard::LControl);
			//for every particle
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

					//remove particle
					pit=particles.erase(pit);
				} else pit++;
			}
			//for every constraint
			for (auto cit=constraints.begin(); cit!=constraints.end();) {
				auto& c=*cit;
				float2 ba=c.b->pos-c.a->pos;
				float2 closePt=getClosePt(c.a->pos, c.b->pos, mousePos);
				if (length(mousePos-closePt)<c.rad*radFactor) {
					cit=constraints.erase(cit);
				} else cit++;
			}
			//for every spring
			for (auto sit=springs.begin(); sit!=springs.end();) {
				auto& s=*sit;
				float2 closePt=getClosePt(s.a->pos, s.b->pos, mousePos);
				float rad=MIN(s.a->rad, s.b->rad);
				if (length(mousePos-closePt)<radFactor*rad) {
					sit=springs.erase(sit);
				} else sit++;
			}
		}

		//remove all connectors
		if (Keyboard::isKeyPressed(Keyboard::End)) {
			bool toClear=constraints.size()+springs.size()==0;
			constraints.clear();
			springs.clear();
		}

		//remove everything
		if (Keyboard::isKeyPressed(Keyboard::Delete)) {
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
		}

		//loading
		bool importFileTemp=Keyboard::isKeyPressed(Keyboard::I);
		if (importFileTemp&&!importFile) {
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

								particle p(float2(x, y), rad);
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
									constraint c(*it0, *it1, rad);
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
									spring s(*it0, *it1, stiff, damp);
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
		}
		importFile=importFileTemp;

		//saving
		bool exportFileTemp=Keyboard::isKeyPressed(Keyboard::E);
		if (exportFileTemp&&!exportFile) {
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
		}
		exportFile=exportFileTemp;
		if (toDebug) uiWatch.stop();
		//end USER_INPUT

		//start UPDATE
		if (toDebug) updateWatch.start();
		if (mouseParticle!=nullptr) {
			if (running) {
				//more of a dragging motion.
				if (!mouseParticle->locked) {
					float2 sub=mousePos-mouseParticle->pos;
					mouseParticle->pos+=sub*deltaTime;
				}
			} else {
				//set mouse particle to mouse pos.
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

		if (running) {
			float subDeltaTime=deltaTime/SUB_STEPS;
			for (int k=0; k<SUB_STEPS; k++) {
				for (auto& c:constraints) c.update();

				for (auto& s:springs) s.update(subDeltaTime);

				//grav and constraint
				for (auto& p:particles) {
					//gravity: f=mg
					p.applyForce(grav*p.mass);

					if (!p.ghosted) {
						//drag: f=-cv
						float2 vel=(p.oldpos-p.pos)/deltaTime;
						p.applyForce(COEFF_DRAG*vel);
					}

					//bounds detection: "keep in window"
					p.checkAABB(bounds);
				}

				//particle-particle collisons
				for (auto ait=particles.begin(); ait!=particles.end(); ait++) {
					auto& a=*ait;
					if (a.ghosted) continue;

					for (auto bit=std::next(ait); bit!=particles.end(); bit++) {
						auto& b=*bit;
						if (b.ghosted) continue;

						float totalRad=a.rad+b.rad;
						constraint tempC(a, b, 0);
						//if intrinsic dist is overlapping
						if (tempC.restLen>totalRad) continue;

						tempC.restLen=totalRad;
						tempC.update();
					}
				}

				//particle-constraint collisions
				for (auto& p:particles) {
					if (p.ghosted) continue;

					for (auto& c:constraints) {
						if (c.ghosted) continue;

						//dont check "self"
						if (&p==c.a||&p==c.b) continue;

						float2 pa=p.pos-c.a->pos, ba=c.b->pos-c.a->pos;
						float t=clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
						particle tempP(c.a->pos+ba*t, 0);
						float totalRad=p.rad+c.rad;
						constraint tempC(p, tempP, 0);
						//if intrinsic dist is overlapping
						if (tempC.restLen>totalRad) continue;

						//resolve particle
						tempC.restLen=totalRad;
						float2 tempForce=tempC.getForce();
						float totalMass=p.mass+c.a->mass+c.b->mass;
						if (!p.locked) p.pos+=(c.a->mass+c.b->mass)/totalMass*2*tempForce;

						//resolve constraint proportionally(torque-ish?)
						float2 abForce=p.mass/totalMass*2*tempForce;
						if (!c.a->locked) c.a->pos-=abForce*(1-t);
						if (!c.b->locked) c.b->pos-=abForce*t;
					}
				}

				//"integrate"
				for (auto& p:particles) p.update(subDeltaTime);
			}
		}
		if (toDebug) updateWatch.stop();
		//end UPDATE

		//start RENDER
		if (toDebug) renderWatch.start();

		renderTex.clear(running?Color(35, 35, 35):Color(0, 0, 0, 0));

		{//experimental
			//add tons of particles
			float2 messEnd=mousePos;
			float nx=MAX(0, MIN(messStart.x, messEnd.x));
			float ny=MAX(0, MIN(messStart.y, messEnd.y));
			float mx=MIN(width, MAX(messStart.x, messEnd.x));
			float my=MIN(height, MAX(messStart.y, messEnd.y));

			bool addMessTemp=Keyboard::isKeyPressed(Keyboard::M);
			if (addMessTemp) {
				drawLine({nx, ny}, {mx, ny});
				drawLine({mx, ny}, {mx, my});
				drawLine({mx, my}, {nx, my});
				drawLine({nx, my}, {nx, ny});
			}

			if (addMessTemp^addMess) {
				if (addMessTemp) messStart=mousePos;
				else {
					for (float x=nx+PARTICLE_RAD; x<mx-PARTICLE_RAD; x+=PARTICLE_RAD) {
						for (float y=ny+PARTICLE_RAD; y<my-PARTICLE_RAD; y+=PARTICLE_RAD) {
							float factor=.5f+.5f*RANDOM;
							particle tempP(float2(x, y), factor*PARTICLE_RAD-.1f);

							//check whether would be overlapping
							bool toAdd=true;
							for (const auto& p:particles) {
								if (p.ghosted) continue;

								if (length(tempP.pos-p.pos)<tempP.rad+p.rad) {
									toAdd=false;
									break;
								}
							}
							for (const auto& c:constraints) {
								if (c.ghosted) continue;

								float2 closePt=getClosePt(c.a->pos, c.b->pos, tempP.pos);
								if (length(tempP.pos-closePt)<tempP.rad+c.rad) {
									toAdd=false;
									break;
								}
							}
							if (toAdd) particles.push_back(tempP);
						}
					}
				}
			}
			addMess=addMessTemp;

			//highlight "continuously connected objects"
			if (Keyboard::isKeyPressed(Keyboard::LShift)) {
				particle* qStart=nullptr;
				for (auto& p:particles) {
					if (length(mousePos-p.pos)<p.rad) qStart=&p;
				}
				if (qStart!=nullptr) {
					std::list<std::pair<particle*, particle*>> connectors;
					for (auto& c:constraints) connectors.push_back({c.a, c.b});
					for (auto& s:springs) connectors.push_back({s.a, s.b});
					std::list<particle*> queue{qStart}, pFlood;
					Color seeThruPink(255, 0, 255, 70);
					while (queue.size()>0) {
						for (auto qit=queue.begin(); qit!=queue.end(); qit=queue.erase(qit)) {
							auto& pPtr=*qit;
							for (auto cit=connectors.begin(); cit!=connectors.end();) {
								auto& c=*cit;
								bool isA=pPtr==c.first;
								bool isB=pPtr==c.second;
								if (isA^isB) {
									queue.push_front(isA?c.second:c.first);

									float rad=MIN(c.first->rad, c.second->rad);
									drawThickLine(c.first->pos, c.second->pos, rad, seeThruPink);

									cit=connectors.erase(cit);
								} else cit++;
							}
							//prevent duplicates
							auto inList=std::find(pFlood.begin(), pFlood.end(), pPtr);
							if (inList==pFlood.end()) pFlood.push_back(pPtr);

							fillCircle(pPtr->pos, pPtr->rad, seeThruPink);
						}
					}
				}
			}
		}

		//draw mouse particle highlight
		if (running&&mouseParticle!=nullptr) {
			float factor=1.5f+(sinf(totalDeltaTime*3.)+1)/4;
			fillCircle(mouseParticle->pos, mouseParticle->rad*factor, Color(0x666666FF));
		}

		//draw constraints
		for (const auto& c:constraints) {//pipes
			int ghostVal=c.ghosted?127:255;
			Color ghostedWhite(255, 255, 255, ghostVal);
			if (running) {
				drawLine(c.a->pos, c.b->pos, Color(0, 0, 0, ghostVal), c.rad);
				drawLine(c.a->pos, c.b->pos, ghostedWhite, c.rad-2);
			} else {
				float2 rAB=c.b->pos-c.a->pos;
				float tTheta=atan2f(rAB.y, rAB.x);
				float aTheta=asinf(c.rad/c.a->rad);
				float2 blPt=c.a->pos+float2(cosf(tTheta+aTheta), sinf(tTheta+aTheta))*c.a->rad;
				float2 brPt=c.a->pos+float2(cosf(tTheta-aTheta), sinf(tTheta-aTheta))*c.a->rad;

				float bTheta=asinf(c.rad/c.b->rad);
				float2 tlPt=c.b->pos+float2(cosf(tTheta-bTheta+PI), sinf(tTheta-bTheta+PI))*c.b->rad;
				float2 trPt=c.b->pos+float2(cosf(tTheta+bTheta+PI), sinf(tTheta+bTheta+PI))*c.b->rad;

				drawLine(blPt, tlPt, ghostedWhite);
				drawLine(brPt, trPt, ghostedWhite);
			}
		}

		//draw springs as dotted lines
		for (const auto& s:springs) {
			float2 sub=s.b->pos-s.a->pos;
			float rad=.25f*(s.a->rad+s.b->rad);
			int num=MAX(4, int(s.restLen/rad));
			for (int i=0; i<num; i++) {
				int d=i-num/2;
				if (d%2) {
					float2 a=s.a->pos+float(i)/num*sub;
					float2 b=s.a->pos+float(i+1)/num*sub;
					if (running) {
						drawLine(a, b, Color::White, .5f*rad);
					} else drawLine(a, b);
				}
			}
		}

		//draw particles
		float maxSpeed=1;
		for (const auto& p:particles) {
			float2 vel=(p.pos-p.oldpos)/deltaTime;
			float speed=length(vel);
			if (speed>maxSpeed) maxSpeed=speed;
		}
		bool toShowSpeed=Keyboard::isKeyPressed(Keyboard::V);
		for (const auto& p:particles) {
			int ghostVal=p.ghosted?127:255;
			float2 vel=(p.pos-p.oldpos)/deltaTime;
			float spd01=length(vel)/maxSpeed;
			//modeled after some arbitrary stress gradient
			Color speedCol=gradient(spd01, {Color::Blue, Color::Cyan, Color::Green, Color::Yellow, Color::Red});
			Color ghostedWhite(255, 255, 255, ghostVal);
			Color ghostedRed(255, 0, 0, ghostVal);
			if (running) {
				fillCircle(p.pos, p.rad, toShowSpeed?speedCol:ghostedWhite);
				drawCircle(p.pos, p.rad, Color(0, 0, 0, ghostVal));
				if (p.locked) {
					//draw x
					float v=p.rad/1.85f;
					drawLine(p.pos-v, p.pos+v, ghostedRed, 2);
					drawLine(p.pos+float2(-v, v), p.pos+float2(+v, -v), ghostedRed, 2);
				}
			} else drawCircle(p.pos, p.rad, p.locked?ghostedRed:toShowSpeed?speedCol:ghostedWhite);
		}

		//show particle "forces"
		if (Keyboard::isKeyPressed(Keyboard::F)) {
			for (auto& p:particles) {
				float2 vel=(p.pos-p.oldpos)/deltaTime;
				drawArrow(p.pos, p.pos+vel, Color::Blue);
				float2 force=p.oldforce/p.mass*deltaTime*10;
				float dpNormal=dot(vel, force)/(length(vel)*length(force));
				float theta01=acosf(dpNormal)/PI;

				float redVal=sqrtf(1-powf(theta01-1, 2));
				float greenVal=sqrtf(1-theta01*theta01);
				drawArrow(p.pos, p.pos+force, Color(255*redVal, 255*greenVal, 40));
			}
		}

		//show possible future constraint
		if (constraintStart!=nullptr) {
			int ybVal=128+127*sinf(totalDeltaTime*4);
			Color yellowBlack(ybVal, ybVal, 20);
			fillCircle(constraintStart->pos, constraintStart->rad, yellowBlack);
			drawArrow(constraintStart->pos, mousePos, yellowBlack);
		}

		//possible future spring
		if (springStart!=nullptr) {
			int mgVal=119*cosf(totalDeltaTime*5);
			Color magentaCyan(120+mgVal, 120-mgVal, 240);
			fillCircle(springStart->pos, springStart->rad, magentaCyan);
			drawArrow(springStart->pos, mousePos, magentaCyan);
		}

		if (toDebug) renderWatch.stop();
		//end RENDER

		//start POST_PROCESS
		if (toDebug) postProcessWatch.start();
		Texture renderedTex=renderTex.getTexture();
		crtShader.setUniform("MainTex", renderedTex);
		blueprintShader.setUniform("MainTex", renderedTex);

		window.clear();
		if (running) window.draw(shaderSprite, &crtShader);
		else window.draw(shaderSprite, &blueprintShader);
		if (toDebug) postProcessWatch.stop();
		//end POST_PROCESS

		//show debug stats
		if (toDebug) {
			int uiTime=uiWatch.getMicros();
			int updateTime=updateWatch.getMicros();
			int renderTime=renderWatch.getMicros();
			int postProcessTime=postProcessWatch.getMicros();
			float constant=2*PI/(uiTime+updateTime+renderTime+postProcessTime);
			float a=constant*uiTime,
				b=a+constant*updateTime,
				c=b+constant*renderTime;
			float sz=80; float2 center=float2(width, height)-sz-10;
			fillPie(center, sz, 0, a, Color::Red);//ui
			fillPie(center, sz, a, b, Color::Yellow);//update
			fillPie(center, sz, b, c, Color::Green);//render
			fillPie(center, sz, c, 2*PI, Color::Blue);//post process
		}
		window.display();
	}

	return 0;
}