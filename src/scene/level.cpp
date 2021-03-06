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
#include "level.hpp"
#include "actor.hpp"
#include "dijkstra.hpp"
#include "texture.hpp"
#include "generator_forest.hpp"

#include "actor_manager.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "logging.hpp"
#include "texture_manager.hpp"
#include "ui.hpp"

#include <fstream> // for std::ifstream
#include <sstream> // for std::istringstream

Level::Level() :
	victory(false), dmg_base(0), map_created(false), map_texture(nullptr),
	map_generator(nullptr), map_width(0), map_height(0), dijkstra_map(nullptr)
{

}
Level::~Level()
{
	free();
}
void Level::free()
{
	for (uint8_t y = 0; y < map_height; y++)
	{
		for (uint8_t x = 0; x < map_width; x++)
		{
			map_data[y][x].occupying_actor = nullptr;
			map_data[y][x].floor_texture = nullptr;
			map_data[y][x].wall_texture = nullptr;
		}
	}
	map_created = false;

	map_data.clear();
	neighbor_rules.clear();

	if (map_texture != nullptr)
	{
		SDL_DestroyTexture(map_texture);
		map_texture = nullptr;
	}
	if (map_generator != nullptr)
	{
		delete map_generator;
		map_generator = nullptr;
	}
	if (dijkstra_map != nullptr)
	{
		delete dijkstra_map;
		dijkstra_map = nullptr;
	}
	for (Texture *t : textures)
		engine.get_texture_manager()->free_texture(t->get_name());

	textures.clear();
	sub_nodes.clear();
}
void Level::create(uint8_t depth)
{
	free();

	if (depth > 1)
		engine.get_actor_manager()->clear_actors(this);

	map_generator = new GeneratorForest;
	map_generator->init();

	bool floor_layer = true;
	uint8_t map_x = 0;
	uint8_t map_y = 0;

	victory = false;
	dmg_base = 0;

	std::istringstream level(map_generator->generate(depth));
	std::string line;

	while (std::getline(level, line))
	{
		if (line[0] == 'l') // Lines starting with an 'l' (for "level") declare our nodes
		{
			// If a node with the given key (third character of the line) does not exist, we need to create it
			auto it = sub_nodes.find(line[2]);
			if (it == sub_nodes.end())
			{
				SubNode temp = { engine.get_texture_manager()->load_texture(line.substr(4, line.length() - 4)) };
				if (temp.sub_texture != nullptr)
				{
					textures.push_back(temp.sub_texture);
					temp.sub_type = get_node_type(temp.sub_texture->get_name());
					temp.sub_animated = get_node_animated(temp.sub_texture);
					sub_nodes[line[2]] = temp;
				}
			}
		}
		else if (line[0] != '\n') // Other lines define the map itself
		{
			std::vector<MapNode> map_line;
			MapNode temp_map_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
			for (char c : line)
			{
				if (sub_nodes.find(c) != sub_nodes.end()) // If the character is defined as a node, update temp_map_node
				{
					if (!floor_layer)
					{
						temp_map_node.wall_type = sub_nodes[c].sub_type;
						temp_map_node.wall_texture = sub_nodes[c].sub_texture;
						temp_map_node.wall_animated = sub_nodes[c].sub_animated;
					}
					else temp_map_node.floor_texture = sub_nodes[c].sub_texture;
				}
				if (!floor_layer) // Every other character ends a node definition
				{
					map_line.push_back(temp_map_node);
					temp_map_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
					map_x += 1;
				}
				floor_layer = !floor_layer;
			}
			map_data.push_back(map_line);
			map_y += 1; map_x = 0;
		}
	}
	map_width = (uint8_t)map_data[0].size();
	map_height = (uint8_t)map_data.size();
	map_created = true;

	map_generator->post_process(this);
	engine.get_actor_manager()->place_actors(this, get_base_pos());

	dijkstra_map = new Dijkstra;
	dijkstra_map->build_map(this);

	// Correct the frames for all map nodes (so that tiles connect to eachother nicely)
	load_neighbor_rules();
	for (uint8_t y = 0; y < map_height; y++)
	{
		for (uint8_t x = 0; x < map_width; x++)
		{
			if (map_data[y][x].floor_texture != nullptr)
				correct_frame(x, y, NT_FLOOR);
			if (map_data[y][x].wall_texture != nullptr)
				correct_frame(x, y, map_data[y][x].wall_type);
		}
	}
	neighbor_rules.clear();
	init_map_texture();

	//camera.update_position(((map_width - 2) * 32) / 2, ((map_height - 1) * 32) / 2);
	logging.cout(std::string("Map created, size: ") + std::to_string((int)map_width) + ", " + std::to_string((int)map_height), LOG_LEVEL);
}
void Level::render() const
{
	if (map_texture != nullptr)
	{
		const SDL_Rect clip = { 0, 0, (map_width + 1) * 32, (map_height + 1) * 32 };
		const SDL_Rect quad = {
			-camera.get_cam_x() - 16, -camera.get_cam_y() - 16,
			(map_width + 1) * 32, (map_height + 1) * 32
		};
		SDL_RenderCopyEx(engine.get_renderer(), map_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
	if (dijkstra_map != nullptr && options.get_b("debug-render_dijkstra"))
		dijkstra_map->render_map();
}
void Level::render_ui() const
{
	if (map_created && map_generator != nullptr)
		map_generator->render_ui();
}
void Level::animate()
{
	if (!map_created)
		return;

	bool refresh_texture = false;
	for (uint8_t y = 0; y < map_height; y++)
	{
		for (uint8_t x = 0; x < map_width; x++)
		{
			if (map_data[y][x].wall_animated && map_data[y][x].wall_texture != nullptr)
			{
				if (map_data[y][x].wall_type == NT_HILL && engine.get_rng() % 3 == 0)
					continue;
				if (map_data[y][x].wall_rect.y < map_data[y][x].wall_texture->get_height() / 2)
					map_data[y][x].wall_rect.y += map_data[y][x].wall_texture->get_height() / 2;
				else map_data[y][x].wall_rect.y -= map_data[y][x].wall_texture->get_height() / 2;

				map_data[y][x].node_rendered = false;
				refresh_texture = true;
			}
		}
	}
	if (refresh_texture)
		refresh_map_texture(true);
}
void Level::next_turn()
{
	if (map_generator != nullptr)
		map_generator->next_turn(this);
}
bool Level::get_wall(int8_t xpos, int8_t ypos, bool check_occupying) const
{
	if (!map_created || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return true;
	if (check_occupying)
	{
		if (map_data[ypos][xpos].occupying_actor != nullptr)
			return true;
	}
	if (map_data[ypos][xpos].wall_type == NT_ROAD || map_data[ypos][xpos].wall_type == NT_BASE)
		return false;
	return map_data[ypos][xpos].wall_texture != nullptr;
}
NodeType Level::get_wall_type(int8_t xpos, int8_t ypos) const
{
	if (!map_created || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return NT_NONE;
	return map_data[ypos][xpos].wall_type;
}
Actor* Level::get_actor(uint8_t xpos, uint8_t ypos) const
{
	if (!map_created || xpos >= map_width || ypos >= map_height)
		return nullptr;
	return map_data[ypos][xpos].occupying_actor;
}
MapNode Level::get_node(uint8_t xpos, uint8_t ypos) const
{
	if (!map_created || xpos >= map_width || ypos >= map_height)
		return { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
	return map_data[ypos][xpos];
}
std::pair<uint8_t, uint8_t> Level::get_base_pos() const
{
	if (map_generator != nullptr)
		return map_generator->get_base_pos();
	return std::make_pair(0, 0);
}
std::pair<uint8_t, uint8_t> Level::get_spawn_pos() const
{
	if (map_generator != nullptr)
		return map_generator->get_spawn_pos();
	return std::make_pair(0, 0);
}
void Level::set_actor(uint8_t xpos, uint8_t ypos, Actor *actor, bool jump)
{
	if (!map_created || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return;
	map_data[ypos][xpos].occupying_actor = actor;

	if (actor != nullptr && jump)
	{
		actor->set_grid_x(xpos); actor->set_x(xpos * 32);
		actor->set_grid_y(ypos); actor->set_y(ypos * 32);
	}
}
void Level::set_node(uint8_t xpos, uint8_t ypos, MapNode node)
{
	if (!map_created || xpos >= map_width || ypos >= map_height)
		return;
	map_data[ypos][xpos] = node;
}
void Level::set_turn(uint8_t turn)
{
	map_generator->set_turn(turn);
}
void Level::init_map_texture()
{
	map_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		(map_width + 1) * 32, (map_height + 1) * 32
	);
	if (map_texture == NULL)
	{
		map_texture = nullptr;
		logging.cerr(std::string("Unable to create blank texture! SDL Error: ") + SDL_GetError(), LOG_TEXTURE);
		return;
	}
	SDL_SetRenderTarget(engine.get_renderer(), map_texture);
	ui.draw_box(0, 0, map_width + 1, map_height + 1, false);
	SDL_SetRenderTarget(engine.get_renderer(), NULL);

	refresh_map_texture();
}
void Level::refresh_map_texture(bool animated_only)
{
	SDL_SetRenderTarget(engine.get_renderer(), map_texture);
	//SDL_Rect default_rect = { 0, 0, 16, 16 };

	//Texture *grass = engine.get_texture_manager()->load_texture("level/decor/grass.png");
	for (uint8_t y = 0; y < map_height; y++)
	{
		for (uint8_t x = 0; x < map_width; x++)
		{
			if (map_data[y][x].node_rendered)
				continue;
			if (animated_only && !map_data[y][x].wall_animated)
				continue;
			map_data[y][x].node_rendered = true;

			if (map_data[y][x].floor_texture != nullptr)
			{
				map_data[y][x].floor_texture->render(x * 32 + 16, y * 32 + 16, &map_data[y][x].floor_rect);
				/*if (grass != nullptr && engine.get_rng() % 5 == 0 &&
					map_data[y][x].floor_texture->get_name().find("_grass") != std::string::npos)
				{
					const SDL_Rect rect = { (engine.get_rng() % 2) * 16, (engine.get_rng() % 2) * 16, 16, 16 };
					grass->render(x * 32 + 16, y * 32 + 16, &rect);
				}*/
			}
			if (map_data[y][x].wall_texture != nullptr && map_data[y][x].wall_type != NT_INVISIBLE)
				map_data[y][x].wall_texture->render(x * 32 + 16, y * 32 + 16, &map_data[y][x].wall_rect);
		}
	}
	//if (grass != nullptr)
		//engine.get_texture_manager()->free_texture(grass->get_name());

	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
void Level::load_neighbor_rules()
{
	std::ifstream rules_file(engine.get_base_path() + "texture/level/rules.txt");
	if (rules_file.is_open())
	{
		uint8_t line_num = 0;
		std::string line;
		std::string prefix;

		while (std::getline(rules_file, line))
		{
			// Lines starting with a dot define the subfolder we're loading
			if (line[0] == '.')
			{
				prefix = line.substr(1, line.length() - 1);
				line_num = 0;
			}
			else if (line[0] != '\n')
			{
				// Lines starting with 'f' (free) are unused tiles in the texture
				if (line[0] != 'f')
					neighbor_rules[prefix + line] = line_num;
				line_num += 1;
			}
		}
		rules_file.close();
	}
}
void Level::correct_frame(uint8_t xpos, uint8_t ypos, NodeType node_type)
{
	if (node_type == NT_INVISIBLE)
		return;
	if (node_type == NT_BASE)
	{
		map_data[ypos][xpos].wall_rect = { 0, 0, 16, 16 };
		return;
	}
	const std::string surroundings = get_surroundings(xpos, ypos, (node_type == NT_FLOOR ? true : false));

	std::string prefix;
	switch (node_type)
	{
		case NT_HILL: prefix = "hill"; break;
		case NT_HOLE: prefix = "hole"; break;
		case NT_TREE: prefix = "tree"; break;
		case NT_WALL: prefix = "wall"; break;
		case NT_WOOD: prefix = "wood"; break;
		case NT_ROAD: prefix = "map"; break;
		case NT_RIVER: prefix = "map"; break;
		default: prefix = "floor"; break;
	}
	bool got_pair;
	uint8_t matches = 0;
	uint8_t loop_matches;
	int8_t char_num;

	std::pair<std::string, uint8_t> default_pair;
	std::pair<std::string, uint8_t> found_pair;

	for (std::pair<std::string, uint8_t> pair : neighbor_rules)
	{
		const std::string key = pair.first;
		if (key == prefix + "--------")
		{
			default_pair = pair;
			continue;
		}
		if (key.substr(0, prefix.length()) != prefix)
			continue;

		got_pair = true;
		char_num = -1;
		loop_matches = 0;

		const std::string loop_key = key.substr(prefix.length(), key.length() - prefix.length());
		for (char c : loop_key)
		{
			char_num += 1;
			if (c == surroundings[char_num])
				loop_matches += 1;
			else if (c != '-')
			{
				got_pair = false;
				break;
			}
		}
		if (got_pair && loop_matches > matches)
		{
			matches = loop_matches;
			found_pair = pair;
		}
	}
	if (matches == 0)
		found_pair = default_pair;

	uint8_t row;
	uint8_t column = found_pair.second;

	if (node_type == NT_FLOOR)
	{
		row = map_data[ypos][xpos].floor_texture->get_height() / 16;
		map_data[ypos][xpos].floor_rect = { 0, 0, 16, 16 };

		if (get_node_animated(map_data[ypos][xpos].floor_texture))
			row /= 2;
		while (column >= row)
		{
			column -= row;
			map_data[ypos][xpos].floor_rect.x += 16;
		}
		map_data[ypos][xpos].floor_rect.y = column * 16;
	}
	else
	{
		row = map_data[ypos][xpos].wall_texture->get_height() / 16;
		map_data[ypos][xpos].wall_rect = { 0, 0, 16, 16 };

		if (get_node_animated(map_data[ypos][xpos].wall_texture))
			row /= 2;
		while (column >= row)
		{
			column -= row;
			map_data[ypos][xpos].wall_rect.x += 16;
		}
		map_data[ypos][xpos].wall_rect.y = column * 16;
	}
}
NodeType Level::get_node_type(const std::string &texture_name) const
{
	if (texture_name.find("invis") != std::string::npos) return NT_INVISIBLE;
	else if (texture_name.find("/tree/") != std::string::npos) return NT_TREE;
	else if (texture_name.find("/hill/") != std::string::npos) return NT_HILL;
	else if (texture_name.find("/wall/") != std::string::npos) return NT_WALL;
	else if (texture_name.find("/wood/") != std::string::npos) return NT_WOOD;
	else if (texture_name.find("/map/") != std::string::npos)
	{
		if (texture_name.find("base_") != std::string::npos)
			return NT_BASE;
		else if (texture_name.find("road_") != std::string::npos)
			return NT_ROAD;
	}
	return NT_FLOOR;
}
bool Level::get_node_animated(const Texture *node_texture) const
{
	if (node_texture != nullptr && node_texture->get_height() > node_texture->get_width())
		return true;
	return false;
}
std::string Level::get_surroundings(uint8_t xpos, uint8_t ypos, bool check_floor) const
{
	std::string result = "";
	const int8_t offset_x[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int8_t offset_y[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

	if (check_floor)
	{
		if (map_data[ypos][xpos].wall_type == NT_WOOD)
			return "00000000";

		const std::string name = map_data[ypos][xpos].floor_texture->get_name();
		std::string base_name = "";
		if (name.find("base") != std::string::npos)
		{
			// Get the prefix of the texture name (e.g "dark1")
			const std::size_t split = name.find('_');
			if (split == std::string::npos)
				return "00000000";
			base_name = name.substr(0, split);
		}
		for (uint8_t i = 0; i < 8; i++)
		{
			if (xpos + offset_x[i] < 0 || xpos + offset_x[i] >= map_width ||
				ypos + offset_y[i] < 0 || ypos + offset_y[i] >= map_height)
			{
				result += '1';
				continue;
			}
			const MapNode mn = map_data[ypos + offset_y[i]][xpos + offset_x[i]];

			if (mn.floor_texture == nullptr)
				result += '0';
			else if (base_name != "" && mn.floor_texture->get_name().find(base_name) != std::string::npos &&
				mn.floor_texture->get_name().find("_wood") == std::string::npos &&
				mn.floor_texture->get_name().find("_stone") == std::string::npos &&
				mn.wall_type != NT_HOLE && mn.wall_type != NT_WALL && mn.wall_type != NT_WOOD)
				result += '1';
			else if (base_name == "" && mn.floor_texture == map_data[ypos][xpos].floor_texture)
				result += '1';
			else result += '0';
		}
	}
	else for (uint8_t i = 0; i < 8; i++)
	{
		if (xpos + offset_x[i] < 0 || xpos + offset_x[i] >= map_width ||
			ypos + offset_y[i] < 0 || ypos + offset_y[i] >= map_height)
			result += '1';
		else if (map_data[ypos + offset_y[i]][xpos + offset_x[i]].wall_texture == map_data[ypos][xpos].wall_texture)
			result += '1';
		else if (map_data[ypos][xpos].wall_type == NT_ROAD &&
			map_data[ypos + offset_y[i]][xpos + offset_x[i]].wall_type == NT_BASE)
			result += '1';
		else result += '0';
	}
	return result;
}
