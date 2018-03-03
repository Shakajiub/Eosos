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

class Actor;
class Monster;
class Object;
class Texture;

enum NodeType
{
	NT_NONE, NT_FLOOR, NT_HILL, NT_HOLE,
	NT_TREE, NT_WALL, NT_WOOD, NT_ROAD,
	NT_RIVER, NT_INVISIBLE
};
typedef struct
{
	Actor *occupying_actor;
	Object *occupying_object;
	Texture *floor_texture;
	Texture *wall_texture;
	SDL_Rect floor_rect;
	SDL_Rect wall_rect;
	NodeType wall_type;
	bool wall_animated;
	bool node_discovered;
	bool node_rendered;
	bool decor_initialized;
	uint8_t los_calculated;
}
MapNode;

typedef struct
{
	Texture *texture;
	uint8_t grid_x, grid_y;
	SDL_Rect rect;
}
MapDecor;

class Level
{
public:
	Level();
	~Level();

	void free();
	void update();
	void render() const;

	void animate();
	void load_map(const std::string &map_name);
	void generate_map(const std::string &script_name);
	void spawn_object(uint8_t xpos, uint8_t ypos, const std::string &script, const std::string &type, const std::string &data = "");
	void update_objects();
	//void save_object_data(uint8_t xpos, uint8_t ypos, const std::string &data);

	void damage_node(uint8_t xpos, uint8_t ypos, int8_t damage, const std::string &source);

	uint8_t get_map_width() const { return map_width; }
	uint8_t get_map_height() const { return map_height; }

	bool get_map_loaded() const { return map_loaded; }
	bool get_map_dark() const { return map_dark; }
	bool get_node_discovered(uint8_t xpos, uint8_t ypos) const;
	bool get_wall(int8_t xpos, int8_t ypos, bool check_occupying = false) const;
	bool get_wall_light(int8_t xpos, int8_t ypos, uint8_t distance) const;

	Actor* get_actor(uint8_t xpos, uint8_t ypos) const;
	Object* get_object(uint8_t xpos, uint8_t ypos) const;
	std::vector<Monster*> get_monsters() const { return monsters; }

	void set_actor(uint8_t xpos, uint8_t ypos, Actor *actor);
	void set_object(uint8_t xpos, uint8_t ypos, Object *object);

	void erase_actor(uint8_t xpos, uint8_t ypos);
	void erase_object(uint8_t xpos, uint8_t ypos);
	void do_fov(uint8_t xpos, uint8_t ypos, uint8_t radius, bool force = false);

private:
	void init_map_texture();
	void render_map_texture() const;
	void refresh_map_texture(bool first_time = false, bool animated_only = false);
	void decorate_map_node(uint8_t xpos, uint8_t ypos);

	void load_neighbor_rules();
	void correct_frame(uint8_t xpos, uint8_t ypos, NodeType node_type);

	NodeType get_node_type(const std::string &texture_name) const;
	bool get_node_animated(const Texture *node_texture) const;
	std::string get_surroundings(uint8_t xpos, uint8_t ypos, bool check_floor) const;

	bool map_loaded, map_dark;
	uint8_t map_width, map_height;

	std::vector<std::vector<MapNode> > map_data;
	std::unordered_map<std::string, uint8_t> neighbor_rules;
	std::vector<Monster*> monsters;
	std::vector<Object*> objects;

	SDL_Texture *map_texture;
	Texture *blank_tile;

	std::vector<Texture*> textures;
	std::vector<uint8_t> decor_IDs;
	std::vector<MapDecor> decorations;

	bool generator_init;
	uint8_t generator_depth;
	std::string generator_script_name;
	lua_State *generator_script;

	void cast_light(uint8_t x, uint8_t y, uint8_t radius, uint8_t row,
		float start_slope, float end_slope, uint8_t xx, uint8_t xy, uint8_t yx, uint8_t yy);
};

#endif // LEVEL_HPP
