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

#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <vector>
#include <unordered_map>

class ActorManager;
class Actor;
class Texture;
class Generator;

enum NodeType
{
	NT_NONE, NT_FLOOR, NT_HILL, NT_HOLE,
	NT_TREE, NT_WALL, NT_WOOD, NT_ROAD,
	NT_RIVER, NT_INVISIBLE
};
typedef struct
{
	Actor *occupying_actor;
	Texture *floor_texture;
	Texture *wall_texture;
	SDL_Rect floor_rect;
	SDL_Rect wall_rect;
	NodeType wall_type;
	bool wall_animated;
	bool node_rendered;
}
MapNode;

class Level
{
public:
	Level();
	~Level();

	void free();
	void create(uint8_t depth);
	void render() const;
	void animate();
	void next_turn(uint16_t turn, ActorManager *am);

	uint8_t get_map_width() const { return map_width; }
	uint8_t get_map_height() const { return map_height; }

	Actor* get_actor(uint8_t xpos, uint8_t ypos) const;
	bool get_wall(int8_t xpos, int8_t ypos, bool check_occupying = false) const;

	void set_actor(uint8_t xpos, uint8_t ypos, Actor *actor);

private:
	void init_map_texture();
	void refresh_map_texture(bool animated_only = false);

	void load_neighbor_rules();
	void correct_frame(uint8_t xpos, uint8_t ypos, NodeType node_type);

	NodeType get_node_type(const std::string &texture_name) const;
	bool get_node_animated(const Texture *node_texture) const;
	std::string get_surroundings(uint8_t xpos, uint8_t ypos, bool check_floor) const;

	bool map_created;
	uint8_t map_width, map_height;

	std::vector< std::vector<MapNode> > map_data;
	std::unordered_map<std::string, uint8_t> neighbor_rules;
	std::vector<Texture*> textures;

	SDL_Texture *map_texture;
	Generator *map_generator;
};

#endif // LEVEL_HPP
