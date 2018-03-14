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

#include "engine.hpp"
#include "astar.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "actor.hpp"
#include "camera.hpp"
#include "texture_manager.hpp"

#include <cfloat> // for FLT_MAX
#include <algorithm> // for std::find

AStar::AStar() : path_found(false), path_marker(nullptr)
{

}
AStar::~AStar()
{
	free();
}
void AStar::free()
{
	path.clear();
	path_found = false;

	if (path_marker != nullptr)
	{
		engine.get_texture_manager()->free_texture(path_marker->get_name());
		path_marker = nullptr;
	}
}
bool AStar::init()
{
	if (path_marker == nullptr)
		path_marker = engine.get_texture_manager()->load_texture("core/texture/ui/path.png", true);
	if (path_marker != nullptr)
		path_marker->set_color(DAWN_BERRY);
	return path_marker != nullptr;
}
bool AStar::find_path(Level *level, int8_t start_x, int8_t start_y, int8_t end_x, int8_t end_y, uint8_t finder)
{
	// Make sure we're not trying to path into a wall
	if (level == nullptr || level->get_wall(end_x, end_y) || (end_x == start_x && end_y == start_y))
		return false;

	std::vector<std::shared_ptr<ASNode> > open_list, closed_list, final_path;
	std::shared_ptr<ASNode> c = std::make_shared<ASNode>();

	c->f = 0; c->g = 0; c->h = 0;
	c->x = start_x; c->y = start_y;
	open_list.push_back(c);

	c.reset();

	// Used for looping all neighbouring nodes
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	bool stop_search = false;
	while (!stop_search && !open_list.empty())
	{
		// Get the node with the lowest f score
		std::shared_ptr<ASNode> q;
		float f = FLT_MAX;
		for (std::shared_ptr<ASNode> n : open_list)
		{
			if (n->f < f)
			{
				f = n->f;
				q = n;
			}
		}
		// Move it to the closed list
		closed_list.push_back(q);
		open_list.erase(std::find(open_list.begin(), open_list.end(), q));

		// Loop through the neighbouring nodes
		const uint8_t max = (finder != 0) ? 8 : 4;
		for (uint8_t i = 0; i < max; i++)
		{
			const int8_t new_x = q->x + offset_x[i];
			const int8_t new_y = q->y + offset_y[i];

			if (level->get_wall(new_x, new_y))
				continue;

			if (finder != 0)
			{
				const Actor *temp_actor = level->get_actor(new_x, new_y);
				if (temp_actor != nullptr && temp_actor->get_actor_type() == finder)
					continue;
			}
			// Ignore nodes on the closed list
			bool ignore = false;
			for (std::shared_ptr<ASNode> n : closed_list)
			{
				if (n->x == new_x && n->y == new_y)
				{
					ignore = true;
					break;
				}
			}
			if (ignore) continue;

			// If it's already on the open list, check if this path is better
			for (std::shared_ptr<ASNode> n : open_list)
			{
				if (n->x == new_x && n->y == new_y)
				{
					ignore = true;
					if (n->g > q->g + (i > 3 ? 1.4f : 1.0f))
					{
						n->parent = q;
						n->g = q->g + (i > 3 ? 1.4f : 1.0f);
						n->f = n->g + n->h;
					}
					break;
				}
			}
			// If not, add it there and calculate the A* magic numbers
			if (!ignore)
			{
				std::shared_ptr<ASNode> n = std::make_shared<ASNode>();
				n->parent = q;
				n->x = new_x; n->y = new_y;

				// If we've found the end, stop the search
				if (new_x == end_x && new_y == end_y)
				{
					c = n;
					closed_list.push_back(n);
					stop_search = true;
					break;
				}
				// Heurestics, magic numbers
				n->g = q->g + (i > 3 ? 1.4f : 1.0f);
				n->h = (float)SDL_sqrt((end_x - new_x)*(end_x - new_x) + (end_y - new_y)*(end_y - new_y));
				n->f = n->g + n->h;
				open_list.push_back(n);

				if (c == nullptr || n->x < c->x)
					c = n;
				n.reset();
			}
		}
		q.reset();
	}
	/*c.reset();
	// Find the last node
	for (std::shared_ptr<ASNode> n : closed_list)
	{
		if (n->x == end_x && n->y == end_y)
		{
			c = n;
			break;
		}
	}*/
	if (c == nullptr)
	{
		open_list.clear();
		closed_list.clear();
		final_path.clear();
		return false;
	}
	// Trace the path back from the last node
	uint8_t xpos = c->x, ypos = c->y;
	while (xpos != start_x || ypos != start_y)
	{
		if (c == nullptr)
			break;
		xpos = c->x; ypos = c->y;
		final_path.push_back(c);
		c = c->parent;
	}
	// Remove the starting node from the path
	final_path.erase(final_path.end() - 1);

	path = final_path;
	goto_x = path.back()->x;
	goto_y = path.back()->y;
	path_found = true;

	open_list.clear();
	closed_list.clear();
	final_path.clear();

	return true;
}
void AStar::clear_path()
{
	path.clear();
	path_found = false;
}
void AStar::step()
{
	if (path.size() == 0)
		return;

	const uint8_t current_x = goto_x;
	const uint8_t current_y = goto_y;
	path.pop_back();

	if (path.size() == 0)
		path_found = false;
	else if (path.size() == 1)
	{
		goto_x = path[0]->x;
		goto_y = path[0]->y;
	}
	else
	{
		std::shared_ptr<ASNode> n = path.back();
		goto_x = n->x; goto_y = n->y;
	}
}
void AStar::render(uint8_t good_length) const
{
	if (!path_found || path.size() == 0)
		return;
	if (path_marker != nullptr)
	{
		path_marker->set_color(DAWN_BERRY);
		uint8_t length = path.size();

		for (std::shared_ptr<ASNode> n : path)
		{
			length -= 1;
			if (length < good_length)
				path_marker->set_color(DAWN_LEAF);

			if (camera.get_in_camera_grid(n->x, n->y))
				path_marker->render(
					n->x * 32 - camera.get_cam_x(),
					n->y * 32 - camera.get_cam_y()
				);
		}
	}
}
uint8_t AStar::get_last_x() const
{
	if (path.size() > 0)
		return path[0]->x;
	else return 0;
}
uint8_t AStar::get_last_y() const
{
	if (path.size() > 0)
		return path[0]->y;
	else return 0;
}
