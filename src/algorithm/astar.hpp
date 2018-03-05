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

#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <vector>
#include <memory> // for std::shared_ptr

class Level;
class Texture;

struct ASNode
{
	std::shared_ptr<ASNode> parent;
	uint8_t x, y;
	float f, g, h;
};
class AStar
{
public:
	AStar();
	~AStar();

	void free();
	void init();

	bool find_path(Level *level, int8_t start_x, int8_t start_y, int8_t end_x, int8_t end_y);
	void clear_path();

	void step();
	void render() const;

	bool get_path_found() const { return path_found; }
	uint8_t get_goto_x() const { return goto_x; }
	uint8_t get_goto_y() const { return goto_y; }
	uint8_t get_last_x() const;
	uint8_t get_last_y() const;

private:
	bool path_found;
	uint8_t goto_x, goto_y;

	std::vector<std::shared_ptr<ASNode> > path;
	Texture *path_marker;
};

#endif // ASTAR_HPP
