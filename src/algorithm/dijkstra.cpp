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
#include "dijkstra.hpp"
#include "dungeon.hpp"

#include "player.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "object.hpp"
#include "level.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Dijkstra::Dijkstra()
{

}
Dijkstra::~Dijkstra()
{

}
void Dijkstra::build_map(Dungeon *scene)
{
	if (scene == nullptr || !scene->get_level()->get_map_loaded())
		return;

	if (options.get_b("debug-render_dijkstra"))
		debug_render = true;
	else debug_render = false;

	dijkstra_map.clear();
	DNode current_node = { player->get_grid_x(), player->get_grid_y(), 0 };
	std::vector<DNode> unvisited_nodes;

	// Used for looping all neighbouring nodes
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	bool stop_search = false;
	while (!stop_search)
	{
		const uint8_t new_distance = current_node.distance + 1;
		if (new_distance > 11)
			break;

		for (uint8_t i = 0; i < 8; i++)
		{
			const int8_t new_x = current_node.x + offset_x[i];
			const int8_t new_y = current_node.y + offset_y[i];

			// Don't calculate nodes outside of the map or in a wall
			if (new_x < 0 || new_x >= scene->get_level()->get_map_width() ||
				new_y < 0 || new_y >= scene->get_level()->get_map_height())
				continue;
			if (scene->get_level()->get_wall(new_x, new_y))
				continue;

			bool node_exists = false;
			for (DNode n : unvisited_nodes)
			{
				if (n.x == new_x && n.y == new_y)
				{
					if (n.distance > new_distance)
						n.distance = new_distance;
					node_exists = true;
					break;
				}
			}
			for (DNode n : dijkstra_map)
			{
				if (n.x == new_x && n.y == new_y)
				{
					node_exists = true;
					break;
				}
			}
			if (!node_exists)
			{
				DNode new_node = { (uint8_t)new_x, (uint8_t)new_y, new_distance };
				unvisited_nodes.push_back(new_node);
			}
		}
		// Move the current node to our dijkstra map
		dijkstra_map.push_back(current_node);

		int16_t current_pos = -1;
		bool found_current_node = false;

		for (DNode n : unvisited_nodes)
		{
			current_pos += 1;
			if (n.x == current_node.x && n.y == current_node.y)
			{
				found_current_node = true;
				break;
			}
		}
		if (found_current_node)
			unvisited_nodes.erase(unvisited_nodes.begin() + current_pos);

		// Get a node with minimal distance to the start
		uint8_t temp_distance = UINT8_MAX;
		for (DNode n : unvisited_nodes)
		{
			if (n.distance < temp_distance)
			{
				current_node = n;
				temp_distance = n.distance;
			}
		}
		if (temp_distance == UINT8_MAX)
			stop_search = true;
	}
}
void Dijkstra::render_map() const
{
	if (!debug_render)
		return;
	for (DNode n : dijkstra_map)
	{
		if (camera.get_in_camera_grid(n.x, n.y))
			ui.get_bitmap_font()->render_text(
				(n.x * 32) - camera.get_cam_x(),
				(n.y * 32) - camera.get_cam_y(), std::to_string(n.distance)
			);
	}
}
bool Dijkstra::check_in_map(uint8_t xpos, uint8_t ypos) const
{
	for (DNode n : dijkstra_map)
	{
		if (n.x == xpos && n.y == ypos)
			return true;
	}
	return false;
}
std::pair<uint8_t, uint8_t> Dijkstra::get_node_downhill(Dungeon *scene, uint8_t xpos, uint8_t ypos) const
{
	if (scene == nullptr)
		return std::make_pair(0, 0);

	std::vector<DNode> neighbors;
	for (DNode n : dijkstra_map)
	{
		if (n.x == xpos && n.y == ypos)
			continue;
		if (std::abs(n.x - xpos) < 2 && std::abs(n.y - ypos) < 2)
			neighbors.push_back(n);
	}
	DNode final_node = { 0, 0, UINT8_MAX };
	for (DNode n : neighbors)
	{
		if (scene->get_level()->get_actor(n.x, n.y) == player)
		{
			final_node = n;
			break;
		}
		if (scene->get_level()->get_wall(n.x, n.y, true))
			continue;
		if (n.distance < final_node.distance)
			final_node = n;
	}
	return std::make_pair(final_node.x, final_node.y);
}
