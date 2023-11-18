#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <list>
#include <vector>

#include "math/float2.h"
using namespace common;

struct Graph {
	struct Node {
		Float2 pos, gf{INFINITY};
		std::list<Node*> links;

		static bool compF(Node* a, Node* b) {
			return a->gf.y<b->gf.y;
		}
	};
	std::list<Node> nodes;

	std::vector<Node> aStar(Node* start, Node* goal) {
		std::vector<Node*> openSet{start};

		std::map<...>

		start->gf.x=0;

		auto h=[&goal] (Node* a) {
			return length(goal->pos-a->pos);
		};

		start->gf.y=h(start);

		while (!openSet.empty()) {
			auto it=std::min_element(openSet.begin(), openSet.end(), Node::compF);
			auto curr=*it;
			if (*it==goal) return;// reconstructPath(cameFrom, curr);

			openSet.erase(it);
			for (auto& n:curr->links) {
				float tentativeG=curr->gf.x+length(curr->pos-n->pos);
				if (tentativeG<n->gf.x) {
					cameFrom[n]=curr;
					n->gf.y=(n->gf.x=tentativeG)+h(n);
				}
			}
		}
	}
};
#endif