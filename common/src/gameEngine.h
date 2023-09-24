#pragma once
#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <SFML/Graphics.hpp>

#include "math/float2.h"

namespace common {
	struct GameEngine : sf::RenderWindow {
		size_t width, height;
		Float2 mousePos;

		GameEngine();

		GameEngine(size_t w, size_t h, std::string t);

		void run();

	private:
		std::string m_title;
		sf::Clock m_deltaClock;

		//user input functions
		virtual void onMouseDown(sf::Mouse::Button button) {}
		virtual void onMouseUp(sf::Mouse::Button button) {}
		virtual void onMouseScroll(float delta) {}
		virtual void onKeyDown(sf::Keyboard::Key key) {}
		virtual void onKeyUp(sf::Keyboard::Key key) {}

		//rendering ease of use
		void drawLine(Float2 a, Float2 b, sf::Color col=sf::Color::White);
		void drawThickLine(Float2 a, Float2 b, float w, sf::Color col=sf::Color::White);
		void drawCircle(Float2 p, float r, sf::Color col=sf::Color::White);
		void fillCircle(Float2 p, float r, sf::Color col=sf::Color::White);

		//on prog start
		virtual void init() {}
		//run as fast as possible
		virtual void update(float dt)=0;
		virtual void render()=0;
	};
}
#endif