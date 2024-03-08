#include <map>

#include "engine/gameEngine.h"
using namespace common;
using namespace sf;

#include "game.h"

//generic array ref syntax?
template<typename T, typename F>
T& spinner(T* (&arr), size_t num, F prob) {
	float pct=rand01();
	for (size_t i=0; i<num; i++) {
		auto& a=arr[i];
		auto val=prob(a);
		if (pct<val) return a;
		pct-=val;
	}
	return arr[0];
}

struct AsteroidsAIDemo : GameEngine {
	AABB bounds;

	static const size_t numPlayers=20;
	Game* players=nullptr;

	float statTimer=0;
	float bestFitness=0;

	Font arial;

	AsteroidsAIDemo(size_t w, size_t h, std::string t) : GameEngine(w, h, t) {}

	bool init() override {
		bounds=AABB(0, Float2(width, height));

		players=new Game[numPlayers];
		for (size_t i=0; i<numPlayers; i++) {
			players[i]=Game(bounds);
		}

		if (!arial.loadFromFile("arial.ttf")) return false;

		return true;
	}

	void update(float dt) override {
		const float speed=5;
		size_t numLeft=0;
		for (size_t i=0; i<numPlayers; i++) {
			auto& p=players[i];
			if (!p.isDead()) {
				p.think();
				p.act(speed*dt);

				p.update(speed*dt);

				if(!p.hasWon()) numLeft++;
			}
		}

		//spawn next generation
		if (numLeft==0) {
			std::cout<<"Next Generation!\n";

			//next generation
			Game::BrainType newBrains[numPlayers];

			//fitness distribution normalization for spinner probabilities
			float totFit=0;
			for (size_t i=0; i<numPlayers; i++) totFit+=players[i].fitness;
			auto normal=[totFit] (const Game& s) { return s.fitness/totFit; };

			//"reproduction"
			for (size_t i=0; i<numPlayers; i++) {
				//choose two random parents
				const auto& a=spinner(players, numPlayers, normal), & b=spinner(players, numPlayers, normal);

				//make kid(50% mutation chance)
				newBrains[i]=Brain(a.brain, b.brain, .5f);
			}

			//copy over
			for (size_t i=0; i<numPlayers; i++) {
				players[i].brain=newBrains[i];
				players[i].reset();
			}
		}


		bestFitness=0;
		for (size_t i=0; i<numPlayers; i++) {
			const auto& p=players[i];
			if (p.fitness>bestFitness) bestFitness=p.fitness;
		}

		//print stats intermittently
		if (statTimer<0) {
			statTimer=.5f;

			std::cout<<"Num Players Left: "<<numLeft<<'\n';

			std::cout<<"Best Fitness: "<<bestFitness<<'\n';
		}
		statTimer-=dt;
	}
	
	template<size_t In, size_t Out>
	void drawBrain(const Brain<In, Out>& b, const AABB a) {
		size_t depth=0;
		for (const auto& n:b.neurons) {
			if (n.layer>depth) depth=n.layer;
		}
		depth+=2;

		std::map<const Neuron*, Float2> positions;
		for (const auto& n:b.neurons) {
			Float2 t;
			if (n.layer==0) t.x=0;
			else if (n.layer==-1) t.x=1;
			else t.x=n.layer/(depth-1.f);

			if (n.layer==0)

			positions[&n]=a.min+t*(a.max-a.min);
		}
	}

	void drawGame(const Game* g) {
		float rank=g->fitness/bestFitness; if (rank<0) rank=0;
		auto col=g->isDead()?Color(0xff333333):Color(0, 255*rank, 0);

		//draw bullets
		for (const auto& b:g->ship.bullets) {
			fillCircle(b.pos, 2, col);
		}

		//draw asteroids as polygons
		for (const auto& a:g->asteroids) {
			for (size_t i=0; i<a.numPts; i++) {
				Float2 b=a.outline[i], c=a.outline[(i+1)%a.numPts];
				drawLine(a.pos+b, a.pos+c, col);
			}
		}

		//draw ship
		auto rad=8;
		drawCircle(g->ship.pos, rad, col);
		Float2 dir(cosf(g->ship.rot), sinf(g->ship.rot));
		drawLine(g->ship.pos, g->ship.pos+rad*dir, col);

		//draw brain
	}

	void drawText(Float2 pos, std::string str, Color col=Color::White) {
		sf::Text text;
		text.setPosition(pos.x, pos.y);

		text.setFont(arial);
		text.setString(str);
		text.setCharacterSize(12);
		text.setFillColor(col);
		draw(text);
	}

	void render() override {
		//gray background
		clear(Color(0x3a3a3aff));

		drawText(0, "Hello World!");

		if (Keyboard::isKeyPressed(Keyboard::D)) {
			//draw all games
			for (size_t i=0; i<numPlayers; i++) drawGame(&players[i]);
		} else {
			//draw best performing creature
			float record=0;
			Game* sim=nullptr;
			for (size_t i=0; i<numPlayers; i++) {
				auto& p=players[i];
				if (p.isDead()) continue;

				if (p.fitness>record) {
					record=p.fitness;
					sim=&p;
				}
			}
			if (sim) drawGame(sim);
		}
	}
};