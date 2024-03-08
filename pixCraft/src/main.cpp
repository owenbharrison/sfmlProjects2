/*TODO:
-flowing blocks
	-add momentum to water
	-add lava [DONE]
		-very slow(viscous)
		-water+lava=obsidian+pollution
		-add pollution
		-trees?
	-add acid, fire, gunpowder?
-little alchemy?
-powdergame?
-add textures
-add grass "grown" value
-interactive environment
	-smoke adds to pressure?
	-cfd(wind)
	-creatures
		-game ai6
		-boss ai?
			-nn to learn player habits
	-non pixel elements
		-rope bridges
		-teardown esc physics
		-grapple guns?
-player
	-wheel select controls
	-movement controls
		-physics
			-aabb collisions
	-viewport scale + transform
	-health
	-breathing?
-chunking 16x16
-shader effects
	-crt
	-chromatic abberation
	-nausea
	-edge blurring?
	-sun bloom
	-clouds
-procedural generation
	-terrain
		-perlin noise
		-fractal brownian motion
		-sum of sines [DONE]
	-biomes
	-structure generation
		-wave function collapse
-FUTURE PROOFING
-NO PREMATURE OPTIMIZATION!
*/
#include <iostream>
#include <vector>
#include <algorithm>

#include <random>
static std::mt19937_64 RandomGenerator(time(NULL));
inline float randFloat(float a=0, float b=1) {
	static std::uniform_real_distribution<float> dist(a, b);
	return dist(RandomGenerator);
}
inline size_t randInt(size_t n) {
	static std::uniform_int_distribution<size_t> dist(0, n);
	return dist(RandomGenerator);
}

#include "block.h"
static const Block
_Air{BlockProperty::None, BlockType::Air},
_Border{BlockProperty::None, BlockType::Border},
_Sand{BlockProperty::Down|BlockProperty::DownSide, BlockType::Sand},
_Dirt{BlockProperty::None, BlockType::Dirt},
_Water{BlockProperty::Down|BlockProperty::Side|BlockProperty::DownSide, BlockType::Water},
_Lava{_Water.properties, BlockType::Lava},
_Obsidian{BlockProperty::None, BlockType::Obsidian};
//_Smoke{BlockProperty::Up|BlockProperty::Side|BlockProperty::UpSide, BlockType::Air, 1};

#include "engine/gameEngine.h"
#include "io/stopwatch.h"
using namespace sf;
using namespace common;

Color lerpCol(Color a, Color b, float t=.5f) {
	Color ba=b-a;
	return a+Color(t*ba.r, t*ba.g, t*ba.b, t*ba.a);
}

struct Demo : GameEngine {
	size_t num_x=0, num_y=0;
	Block* blocks=nullptr;
	inline size_t IX(size_t i, size_t j) { return i+num_x*j; }

	//from->to
	std::vector<std::pair<size_t, size_t>> changes;

	float update_timer=0;

	bool to_time=false;
	Stopwatch update_watch, render_watch;

	Demo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	bool init() override {
		auto size=10;
		num_x=width/size, num_y=height/size;
		blocks=new Block[num_x*num_y];
		generateTerrain();
		generateBorder();

		return true;
	}

	void generateBorder() {
		//top and bottom
		for (size_t i=0; i<num_x; i++) {
			blocks[IX(i, 0)]=blocks[IX(i, num_y-1)]=_Border;
		}

		//left and right
		for (size_t j=0; j<num_y; j++) {
			blocks[IX(0, j)]=blocks[IX(num_x-1, j)]=_Border;
		}
	}

	//implement fractal brownian motion?
	//implement wave function collapse
	void generateTerrain() {
		const float offset=randFloat(0, 100000);
		for (size_t i=0; i<num_x; i++) {
			//sum of sines
			float x=offset+float(i)/num_x, y=.7f;
			float amp=.2f, freq=1;
			for (size_t i=0; i<8; i++) {
				y+=amp*sinf(freq*x);
				amp/=2, freq*=2;
			}
			y*=num_y;

			for (size_t j=0; j<num_y; j++) {
				blocks[IX(i, j)]=j<y?_Air:_Dirt;
			}
		}
	}

	void onKeyDown(Keyboard::Key key) override {
		switch (key) {
			case Keyboard::Home:
				generateTerrain();
				generateBorder();
				break;
			case Keyboard::Delete://clear w/ borders
				for (size_t i=0; i<num_x*num_y; i++) blocks[i]=_Air;
				generateBorder();
				break;
		}
	}

	bool moveBetween(size_t a, size_t b) {
		if (blocks[a].type==blocks[b].type) return false;

		if (!blocks[b].isDisplacable()) return false;
		
		changes.emplace_back(a, b);
		return true;
	}

	//algorithm like winterdev's
	//https://blog.winter.dev/articles/falling-sand
	void update(float dt) override {
		to_time=Keyboard::isKeyPressed(Keyboard::LControl);

		if (to_time) update_watch.start();

		//user input
		bool to_place=true; Block said_block; int rad;
		if (Keyboard::isKeyPressed(Keyboard::A)) said_block=_Air, rad=2;
		else if (Keyboard::isKeyPressed(Keyboard::S)) said_block=_Sand, rad=1;
		else if (Keyboard::isKeyPressed(Keyboard::D)) said_block=_Dirt, rad=2;
		else if (Keyboard::isKeyPressed(Keyboard::W)) said_block=_Water, rad=2;
		else if (Keyboard::isKeyPressed(Keyboard::L)) said_block=_Lava, rad=1;
		else to_place=false;
		if (to_place) {
			int mouse_i=mousePos.x/width*num_x;
			int mouse_j=mousePos.y/height*num_y;
			for (int di=-rad; di<=rad; di++) {
				for (int dj=-rad; dj<=rad; dj++) {
					if (di*di+dj*dj>rad*rad) continue;

					int i=mouse_i+di, j=mouse_j+dj;
					if (i<1||j<1||i>num_x-2||j>num_y-2) continue;

					blocks[IX(i, j)]=said_block;
				}
			}
		}

		while (update_timer<0) {
			const float fixedDt=1/60.f;
			update_timer+=fixedDt;

			//moving blocks
			changes.clear();
			bool flip_x=randFloat()<.5f;
			for (int i=flip_x?num_x-2:1; flip_x?i>=1:i<num_x-1; i+=1-2*flip_x) {
			  bool flip_y=randFloat()<.5f;
				for (int j=flip_y?num_y-2:1; flip_y?j>=1:j<num_y-1; j+=1-2*flip_y) {
					const size_t ctr=IX(i, j);
					const auto& curr=blocks[ctr];

					//prioritize down, then up
					if (curr.properties&BlockProperty::Down&&moveBetween(ctr, IX(i, j+1)));
					else if (curr.properties&BlockProperty::Up&&moveBetween(ctr, IX(i, j-1)));
					//then to the sides
					else {
						const bool move_side=curr.properties&BlockProperty::Side,
							move_down_side=curr.properties&BlockProperty::DownSide,
							move_up_side=curr.properties&BlockProperty::UpSide;
						if (move_side||move_down_side||move_up_side) {
							const size_t lf=IX(i-1, j), rt=IX(i+1, j);
							bool left=blocks[lf].type==BlockType::Air;
							bool right=blocks[rt].type==BlockType::Air;
							if (left&&right) left=randFloat()<.5f, right=!left;

							//WHY GOTO? its fun :3
							size_t to_swap;
							if (left) {
								if (move_down_side) {
									to_swap=IX(i-1, j+1);
									if (blocks[to_swap].type==BlockType::Air) goto DOSWAP;
								}
								if (move_up_side) {
									to_swap=IX(i-1, j-1);
									if (blocks[to_swap].type==BlockType::Air) goto DOSWAP;
								}
								if (move_side) { to_swap=lf; goto DOSWAP; }
							}
							if (right) {
								if (move_down_side) {
									to_swap=IX(i+1, j+1);
									if (blocks[to_swap].type==BlockType::Air) goto DOSWAP;
								}
								if (move_up_side) {
									to_swap=IX(i+1, j-1);
									if (blocks[to_swap].type==BlockType::Air) goto DOSWAP;
								}
								if (move_side) { to_swap=rt; goto DOSWAP; }
							}
							continue;

						DOSWAP:
							changes.emplace_back(ctr, to_swap);
						}
					}
				}
			}

			/*//remove blocks with filled destination...
			for (auto it=changes.begin(); it!=changes.end();) {
				const auto& c=*it;
				if (blocks[it->second].isDisplacable()) it++;
				else it=changes.erase(it);
			}*/

			//sort by destination
			std::sort(changes.begin(), changes.end(),
				[] (auto& a, auto& b) { return a.second<b.second; }
			);

			//catch end
			changes.emplace_back(-1, -1);
			for (size_t i=0, i_prev=0; i<changes.size()-1; i++) {
				//compare dst
				if (changes[i].second!=changes[i+1].second) {
					size_t rnd=i_prev+randInt(i-i_prev);

					size_t src=changes[rnd].first;
					size_t dst=changes[rnd].second;

					//blocks[dst]=blocks[src];
					//blocks[src]=_Air;
					std::swap(blocks[src], blocks[dst]);

					i_prev=i+1;
				}
			}

			//block combinations
			for (size_t i=0; i<num_x; i++) {
				for (size_t j=0; j<num_y; j++) {
					auto& curr=blocks[IX(i, j)];
					if (curr.type!=BlockType::Lava) continue;

					//left, up, right, down
					bool crystalize=false;
					for (size_t s=0; s<4; s++) {
						int c_i=i, c_j=j;
						switch (s) {
							case 0: c_i--; break;
							case 1: c_j--; break;
							case 2: c_i++; break;
							case 3: c_j++; break;
						}
						if (c_i<0||c_j<0||c_i>=num_x||c_j>=num_y) continue;

						auto& check=blocks[IX(c_i, c_j)];
						if (check.type!=BlockType::Water) continue;

						check=_Air, crystalize=true;
					}
					if (crystalize) curr=_Obsidian;
				}
			}
		}
		update_timer-=dt;

		if (to_time) update_watch.stop();
	}

	void drawArrow(Float2 a, Float2 b, Color col=Color::White, float t=.24f) {
		Float2 sub=b-a, sz=t*sub, tang{-sz.y, sz.x};
		Float2 aSt=b-sz, lPt=aSt-tang*.5f, rPt=aSt+tang*.5f;
		//line
		drawLine(a, aSt, col);
		//triangle
		drawLine(rPt, b, col);
		drawLine(b, lPt, col);
		drawLine(lPt, rPt, col);
	}

	void render() override {
		if (to_time) render_watch.start();

		const float size_x=width/float(num_x), size_y=height/float(num_y);
		RectangleShape rect(Vector2f(size_x, size_y));
		for (size_t i=0; i<num_x; i++) {
			for (size_t j=0; j<num_y; j++) {
				rect.setPosition(size_x*i, size_y*j);

				Color col;
				const auto& curr=blocks[IX(i, j)];
				switch (curr.type) {
					case BlockType::Air:
						col=lerpCol(
							Color(0x2287C9FF),
							Color(0x7C5715FF),
							curr.value
						);
						break;
					case BlockType::Dirt:
						col=Color(0x8F531BFF);
						if (j>0) {
							const auto& up=blocks[IX(i, j-1)];
							if (up.isDisplacable()||up.type==BlockType::Water) {
								col=Color(0x0C8A21FF);
							}
						}
						break;
					case BlockType::Sand:
						col=Color(0xE6D38AFF);
						break;
					case BlockType::Water:
						col=lerpCol(
							Color(0x1B4AC2FF),
							Color(0xABE3FFFF),
							curr.value
						);
						break;
					case BlockType::Lava:
						col=lerpCol(
							Color(0xD70000FF),
							Color(0xF2C700FF),
							curr.value
						);
						break;
					case BlockType::Obsidian:
						col=Color(0x21003BFF);
						break;
				}
				rect.setFillColor(col);
				draw(rect);
			}
		}

		const Float2 size(size_x, size_y);
		for (const auto& c:changes) {
			size_t src_i=c.first%num_x, src_j=c.first/num_x;
			size_t dst_i=c.second%num_x, dst_j=c.second/num_x;
			Float2 src_pos=size*(.5f+Float2(src_i, src_j));
			Float2 dst_pos=size*(.5f+Float2(dst_i, dst_j));
			drawArrow(src_pos, dst_pos, Color::Black);
		}

		if (to_time) {
			render_watch.stop();
			std::cout<<"update: "<<update_watch.getMicros()<<"us ";
			std::cout<<"render: "<<render_watch.getMicros()<<"us"<<std::endl;
		}
	}
};

int main() {
	Demo demo(800, 600, "sand sim");
	demo.run();

	return 0;
}