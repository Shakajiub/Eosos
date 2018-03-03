//	Copyright (C) 2018 Jere Oikarinen
//
//	This file is part of Eosos.
//
//	Eosos is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Eosos is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with Eosos. If not, see <http://www.gnu.org/licenses/>.

#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <vector>

class Dungeon;

struct DNode
{
	uint8_t x, y;
	uint8_t distance;
};
class Dijkstra
{
public:
	Dijkstra();
	~Dijkstra();

	void build_map(Dungeon *scene);
	void render_map() const;

	bool check_in_map(uint8_t xpos, uint8_t ypos) const;
	std::pair<uint8_t, uint8_t> get_node_downhill(Dungeon *scene, uint8_t xpos, uint8_t ypos) const;

private:
	bool debug_render;
	std::vector<DNode> dijkstra_map;
};

#endif // DIJKSTRA_HPP
