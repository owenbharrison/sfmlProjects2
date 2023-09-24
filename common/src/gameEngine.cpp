#include "gameEngine.h"

namespace common {
	GameEngine::GameEngine() : width(0), height(0) {}

	GameEngine::GameEngine(size_t w, size_t h, std::string t) : width(w), height(h), m_title(t), RenderWindow(sf::VideoMode(w, h), "", sf::Style::Titlebar|sf::Style::Close) {}

	void GameEngine::drawLine(Float2 a, Float2 b, sf::Color col=sf::Color::White) {
		sf::Vertex line[]{
			sf::Vertex(sf::Vector2f(a.x, a.y), col),
			sf::Vertex(sf::Vector2f(b.x, b.y), col)
		};
		draw(line, 2, sf::Lines);
	}

	void GameEngine::drawThickLine(Float2 a, Float2 b, float w, sf::Color col=sf::Color::White) {
		Float2 ba=b-a;
		sf::RectangleShape line(sf::Vector2f(length(ba), 2*w));
		line.setOrigin(sf::Vector2f(0, w));
		//deg=rad*180/PI
		line.setRotation(atan2f(ba.y, ba.x)*57.2957795f);
		line.setPosition(sf::Vector2f(a.x, a.y));

		line.setFillColor(col);
		draw(line);
	}

	void GameEngine::drawCircle(Float2 p, float r, sf::Color col=sf::Color::White) {
		sf::CircleShape circ(r);
		circ.setOrigin(sf::Vector2f(r, r));
		circ.setPosition(sf::Vector2f(p.x, p.y));

		circ.setFillColor(sf::Color::Transparent);
		circ.setOutlineThickness(-2);
		circ.setOutlineColor(col);
		draw(circ);
	}

	void GameEngine::fillCircle(Float2 p, float r, sf::Color col=sf::Color::White) {
		sf::CircleShape circ(r);
		circ.setOrigin(sf::Vector2f(r, r));
		circ.setPosition(sf::Vector2(p.x, p.y));

		circ.setFillColor(col);
		draw(circ);
	}

	void GameEngine::run() {
		init();

		while (isOpen()) {
			sf::Vector2i mp=sf::Mouse::getPosition(*this);
			mousePos={mp.x, mp.y};

			//polling
			for (sf::Event e; pollEvent(e);) {
				switch (e.type) {
					case sf::Event::Closed: close(); break;
					case sf::Event::MouseButtonPressed:
					{
						onMouseDown(e.mouseButton.button);
						break;
					}
					case sf::Event::MouseButtonReleased:
					{
						onMouseUp(e.mouseButton.button);
						break;
					}
					case sf::Event::MouseWheelScrolled:
					{
						onMouseScroll(e.mouseWheelScroll.delta);
						break;
					}
					case sf::Event::KeyPressed:
					{
						onKeyDown(e.key.code);
						break;
					}
					case sf::Event::KeyReleased:
					{
						onKeyDown(e.key.code);
						break;
					}
				}
			}

			//timing
			float deltaTime=m_deltaClock.restart().asSeconds();
			setTitle(m_title+" @ "+std::to_string(int(1/deltaTime))+"fps");

			//update
			update(deltaTime);

			//render
			clear();
			render();
			display();
		}
	}
}