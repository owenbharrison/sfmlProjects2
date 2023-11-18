#include "engine/gameEngine.h"
using namespace common;

#include <list>

#include "link.h"

struct AStarNavDemo : GameEngine {
	std::list<Node> nodes;
	std::list<Link> links;


};