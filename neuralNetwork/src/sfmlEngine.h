#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#include <SFML/Graphics.hpp>
using namespace sf;

struct sfmlEngine : RenderWindow {
	size_t width, height;
	Vector2i mousePos;

	sfmlEngine() : width(0), height(0) {}

	sfmlEngine(size_t w, size_t h, std::string t) : width(w), height(h), m_title(t), RenderWindow(VideoMode(w, h), "", Style::Titlebar|Style::Close) {
		setKeyRepeatEnabled(false);
	}

	void run() {
		init();

		while (isOpen()) {
			mousePos=Mouse::getPosition(*this);

			//polling
			for (Event e; pollEvent(e);) {
				switch (e.type) {
					case Event::Closed: close(); break;
					case Event::MouseButtonPressed:
					{
						onMouseDown(e.mouseButton.button);
						break;
					}
					case Event::MouseButtonReleased:
					{
						onMouseUp(e.mouseButton.button);
						break;
					}
					case Event::MouseWheelScrolled:
					{
						onMouseScroll(e.mouseWheelScroll.delta);
						break;
					}
					case Event::KeyPressed:
					{
						onKeyDown(e.key.code);
						break;
					}
					case Event::KeyReleased:
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

private:
	std::string m_title;
	Clock m_deltaClock;

	//user input functions
	virtual void onMouseDown(Mouse::Button button) {}
	virtual void onMouseUp(Mouse::Button button) {}
	virtual void onMouseScroll(float delta) {}
	virtual void onKeyDown(Keyboard::Key key) {}
	virtual void onKeyUp(Keyboard::Key key) {}

	//on prog start
	virtual void init() {}
	//run as fast as possible
	virtual void update(float dt)=0;
	virtual void render()=0;
};
#endif