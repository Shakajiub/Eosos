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
#include "texture.hpp"

#include "camera.hpp"
#include "texture_manager.hpp"

#include <fstream> // for std::ifstream

typedef struct
{
	Texture *sub_texture;
	NodeType sub_type;
	bool sub_animated;
}
SubNode;

Level::Level() : map_created(false), map_texture(nullptr)
{

}
Level::~Level()
{
	free();
}
void Level::free()
{
	map_created = false;

	map_data.clear();
	neighbor_rules.clear();

	if (map_texture != nullptr)
	{
		SDL_DestroyTexture(map_texture);
		map_texture = nullptr;
	}
	for (Texture *t : textures)
		engine.get_texture_manager()->free_texture(t->get_name());

	textures.clear();
}
void Level::create()
{
	std::unordered_map<char, SubNode> nodes;

	std::string floors[3] = {
		"core/texture/level/floor/dark2_base.png",
		"core/texture/level/floor/dark2_grass.png",
		"core/texture/level/floor/dark2_hill.png"
	};
	char ids[3] = { 'b', 'g', 'h' };

	for (uint8_t i = 0; i < 3; i++)
	{
		SubNode temp = { engine.get_texture_manager()->load_texture(floors[i]) };
		if (temp.sub_texture != nullptr)
		{
			textures.push_back(temp.sub_texture);
			temp.sub_type = get_node_type(temp.sub_texture->get_name());
			temp.sub_animated = get_node_animated(temp.sub_texture);
		}
		nodes[ids[i]] = temp;
	}
	map_width = 30;
	map_height = 20;

	for (uint8_t y = 0; y < map_height; y++)
	{
		std::vector<MapNode> map_line;
		for (uint8_t x = 0; x < map_width; x++)
		{
			MapNode temp_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
			const uint8_t i = engine.get_rng() % 3;
			temp_node.floor_texture = nodes[ids[i]].sub_texture;
			temp_node.wall_type = nodes[ids[i]].sub_type;
			temp_node.wall_animated = nodes[ids[i]].sub_animated;
			map_line.push_back(temp_node);
		}
		map_data.push_back(map_line);
	}
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
}
void Level::render() const
{
	if (map_texture != nullptr)
	{
		const int16_t cam_x = camera.get_cam_x();
		const int16_t cam_y = camera.get_cam_y();
		const uint16_t cam_w = camera.get_cam_w();
		const uint16_t cam_h = camera.get_cam_h();

		// Only render the part of the texture that's visible on the screen
		SDL_Rect clip = { 0, 0, map_width * 32, map_height * 32 };
		SDL_Rect quad = { -cam_x, -cam_y, cam_w, cam_h };

		// Special case for when the map width is less than the camera width
		if (clip.w >= map_width * 32)
		{
			clip.w = map_width * 32; quad.w = clip.w;
			if (quad.x < 0)
			{
				clip.x += std::abs(quad.x); quad.w += quad.x;
				clip.w = quad.w; quad.x = 0;
			}
			else if (quad.x + map_width * 32 > cam_w)
			{
				clip.w -= (quad.x + map_width * 32) - cam_w; quad.w = clip.w;
			}
		}
		// Camera moving right, move clip.x to the right
		// When over the right map edge, decrease both clip & quad width
		else if (quad.x < 0)
		{
			clip.x += std::abs(quad.x); quad.x = 0;
			if (clip.x > clip.w)
			{
				clip.w -= (clip.x - clip.w); quad.w = clip.w;
			}
		}
		// When over the left map edge, decrease both clip & quad width (clip.x stays at 0)
		else if (quad.x > 0)
		{
			clip.w -= quad.x; quad.w = clip.w;
		}
		// Special case for when the map height is less than the camera height
		if (clip.h >= map_height * 32)
		{
			clip.h = map_height * 32; quad.h = clip.h;
			if (quad.y < 0)
			{
				clip.y += std::abs(quad.y); quad.h += quad.y;
				clip.h = quad.h; quad.y = 0;
			}
			else if (quad.y + map_height * 32 > cam_h)
			{
				clip.h -= (quad.y + map_height * 32) - cam_h; quad.h = clip.h;
			}
		}
		// Camera moving down, move clip.y down
		// When over the bottom map edge, decrease both clip & quad height
		else if (quad.y < 0)
		{
			clip.y += std::abs(quad.y); quad.y = 0;
			if (clip.y > clip.h)
			{
				clip.h -= (clip.y - clip.h); quad.h = clip.h;
			}
		}
		// When over the top map edge, decrease both clip & quad height (clip.y stays at 0)
		else if (quad.y > 0)
		{
			clip.h -= quad.y; quad.h = clip.h;
		}
		SDL_RenderCopyEx(engine.get_renderer(), map_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
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
void Level::init_map_texture()
{
	map_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		map_width * 32, map_height * 32
	);
	if (map_texture == NULL)
	{
		map_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}
	refresh_map_texture();
}
void Level::refresh_map_texture(bool animated_only)
{
	SDL_SetRenderTarget(engine.get_renderer(), map_texture);
	SDL_Rect default_rect = { 0, 0, 16, 16 };

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
				map_data[y][x].floor_texture->render(x * 32, y * 32, &map_data[y][x].floor_rect);

			if (map_data[y][x].wall_texture != nullptr && map_data[y][x].wall_type != NT_INVISIBLE)
				map_data[y][x].wall_texture->render(x * 32, y * 32, &map_data[y][x].wall_rect);
		}
	}
	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
void Level::load_neighbor_rules()
{
	std::ifstream rules_file(engine.get_base_path() + "core/texture/level/rules.txt");
	if (rules_file.is_open())
	{
		uint8_t line_num;
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
	const std::string surroundings = get_surroundings(xpos, ypos, (node_type == NT_FLOOR ? true : false));
	std::string prefix;

	switch (node_type)
	{
		case NT_HILL: prefix = "hill"; break;
		case NT_HOLE: prefix = "hole"; break;
		case NT_TREE: prefix = "tree"; break;
		case NT_WALL: prefix = "wall"; break;
		case NT_WOOD: prefix = "wood"; break;
		case NT_ROAD: case NT_RIVER: prefix = "map"; break;
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
	// This is terrible, but at least it's only called once for each node definition

	if (texture_name.find("/tree/") != std::string::npos)
		return NT_TREE;
	else if (texture_name.find("/hill/") != std::string::npos)
		return NT_HILL;
	else if (texture_name.find("/hole/") != std::string::npos)
		return NT_HOLE;
	else if (texture_name.find("/wall/") != std::string::npos)
		return NT_WALL;
	else if (texture_name.find("/wood/") != std::string::npos)
		return NT_WOOD;
	else if (texture_name.find("/map/") != std::string::npos)
	{
		if (texture_name.find("road_") != std::string::npos)
			return NT_ROAD;
		else if (texture_name.find("river_") != std::string::npos)
			return NT_RIVER;
		else if (texture_name.find("wall_") != std::string::npos)
			return NT_WALL;
		else std::cout << "invalid map texture type '" << texture_name << "'!" << std::endl;
	}
	else if (texture_name.find("invis") != std::string::npos)
		return NT_INVISIBLE;
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
			result += '0';
		else if (map_data[ypos + offset_y[i]][xpos + offset_x[i]].wall_texture == map_data[ypos][xpos].wall_texture)
			result += '1';
		else result += '0';
	}
	return result;
}