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
#include "monster.hpp"
#include "object.hpp"
#include "texture.hpp"

#include "player.hpp"
#include "camera.hpp"
#include "functions_general.hpp"
#include "lua_manager.hpp"
#include "texture_manager.hpp"

#include <algorithm> // for std::find
#include <fstream> // for std::fstream

typedef struct
{
	Texture *sub_texture;
	NodeType sub_type;
	bool sub_animated;
}
SubNode; // Temporary storage for both layers of a map node

typedef struct
{
	char map_char;
	std::string script;
	std::string type;
}
ScriptDef; // A script definition as it appears at the top of a level file
// Built from a line like this: "e-5-core/script/data/monster/zombie.lua|random\n",
// where map_char is '5', with '|' dividing the 'script' and 'type'

typedef struct
{
	std::string data;
	uint8_t grid_x;
	uint8_t grid_y;
}
PositionData; // Temporary data for a position on the map
// For example, 'data' could be the type of a monster to spawn on the position 'grid_x', 'grid_y',
// or the metadata of an object that will be spawned on the position, like the text of a sign

typedef struct
{
	std::string id;
	std::string name;
	std::string texture;
	std::string script;
	int8_t skills[7];
}
MonsterData; // Data received for a monster to spawn from external script

Level::Level() :
	map_loaded(false), map_dark(false), generator_init(false),
	map_texture(nullptr), blank_tile(nullptr), generator_script(nullptr)
{

}
Level::~Level()
{
	free();
}
void Level::free()
{
	map_loaded = false;
	for (uint8_t y = 0; y < map_height; y++)
	{
		for (uint8_t x = 0; x < map_width; x++)
		{
			map_data[y][x].occupying_actor = nullptr;
			map_data[y][x].occupying_object = nullptr;
			map_data[y][x].floor_texture = nullptr;
			map_data[y][x].wall_texture = nullptr;
		}
	}
	map_data.clear();
	neighbor_rules.clear();

	for (uint8_t i = 0; i < monsters.size(); i++)
	{
		if (monsters[i] != nullptr)
			delete monsters[i];
	}
	monsters.clear();

	for (uint8_t i = 0; i < objects.size(); i++)
	{
		if (objects[i] != nullptr)
			delete objects[i];
	}
	objects.clear();

	if (map_texture != nullptr)
	{
		SDL_DestroyTexture(map_texture);
		map_texture = nullptr;
	}
	blank_tile = nullptr;
	for (Texture *t : textures)
		engine.get_texture_manager()->free_texture(t->get_name());

	textures.clear();
	decor_IDs.clear();
	decorations.clear();

	generator_init = false;
	if (generator_script != nullptr)
	{
		engine.get_lua_manager()->free_script(generator_script_name);
		generator_script = nullptr;
	}
}
void Level::update()
{

}
void Level::render() const
{
	if (map_texture != nullptr)
		render_map_texture();

	for (Object *o : objects)
	{
		if (get_node_discovered(o->get_grid_x(), o->get_grid_y()))
			o->render();
	}
}
void Level::animate()
{
	if (!map_loaded) return;

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
			// Animate any objects on the node
			if (map_data[y][x].occupying_object != nullptr)
				map_data[y][x].occupying_object->animate();
		}
	}
	if (refresh_texture)
		refresh_map_texture(false, true);
}
void Level::load_map(const std::string &map_name)
{
	// TODO - This can obviously fail on a corrupted/invalid file, so make sure to handle that
	// TODO - This is also a rather long function, might make it easier to maintain if split into sub-functions

	std::ifstream level_file(engine.get_base_path() + "core/level/" + map_name);
	if (!level_file.is_open())
	{
		std::cout << "could not open level: " << map_name << std::endl;
		map_loaded = false;
		return;
	}
	std::unordered_map<char, SubNode> nodes;
	std::vector<ScriptDef> monster_defs;
	std::vector<ScriptDef> object_defs;
	std::vector<PositionData> monster_spawns;
	std::vector<PositionData> object_spawns;
	std::vector<PositionData> object_data;
	std::vector<MonsterData> monster_data;

	bool floor_layer = true;
	uint8_t map_x = 0, map_y = 0;
	uint8_t player_x = UINT8_MAX;
	uint8_t player_y = UINT8_MAX;
	std::string line;

	// Start parsing the given level file
	while (std::getline(level_file, line))
	{
		if (line[0] == 'l') // Lines starting with an 'l' (for "level") declare our nodes
		{
			// If a node with the given key (third character of the line) does not exist, we need to create it
			auto it = nodes.find(line[2]);
			if (it == nodes.end())
			{
				SubNode temp = { engine.get_texture_manager()->load_texture(line.substr(4, line.length() - 4)) };
				if (temp.sub_texture != nullptr)
				{
					textures.push_back(temp.sub_texture);
					temp.sub_type = get_node_type(temp.sub_texture->get_name());
					temp.sub_animated = get_node_animated(temp.sub_texture);
					nodes[line[2]] = temp;
				}
			}
		}
		else if (line[0] == 'm' || line[0] == 'o') // 'm' for "monster", 'o' for "object"
		{
			const std::size_t split = line.find('|');
			if (split != std::string::npos)
			{
				ScriptDef temp = { line[2] };
				temp.script = line.substr(4, split - 4);
				temp.type = line.substr(split + 1);

				if (line[0] == 'm')
				{
					if (temp.type == "random")
					{
						for (uint8_t i = 0; i < 4; i++)
						{
							temp.type = "random" + std::to_string(i);
							monster_defs.push_back(temp);
						}
					}
					else monster_defs.push_back(temp);
				}
				else object_defs.push_back(temp);
				std::cout << "definition loaded: '" << temp.map_char << "' - \"" << temp.script << "\" | \"" << temp.type << "\"" << std::endl;
			}
			else std::cout << "monster/object definition is missing '|' delimiter! script line:\n" << line << std::endl;
		}
		else if (line[0] == 'd') // 'd' for "decoration"
		{
			Texture *temp_texture = engine.get_texture_manager()->load_texture(line.substr(2, line.length() - 2));
			decor_IDs.push_back((uint8_t)textures.size());
			textures.push_back(temp_texture);
		}
		else if (line[0] == 'i') // 'i' for "information"
		{
			const std::size_t pos_split = line.find(',');
			if (pos_split != std::string::npos)
			{
				const std::size_t data_split = line.find('|');
				if (data_split != std::string::npos)
				{
					PositionData temp;
					temp.data = line.substr(data_split + 1);

					// We need to subtract 1 from both positions, since in lua they start at 1 (here they start at 0)
					temp.grid_x = std::stoi(line.substr(2, pos_split)) - 1;
					temp.grid_y = std::stoi(line.substr(pos_split + 1, data_split)) - 1;
					object_data.push_back(temp);
					std::cout << "information loaded: (" << (int)(temp.grid_x) << ", " << (int)(temp.grid_y) << ") - \"" << temp.data << "\"" << std::endl;
				}
				else std::cout << "information definition is missing '|' delimiter! script line:\n" << line << std::endl;
			}
			else std::cout << "information definition is missing ',' delimiter! script line:\n" << line << std::endl;
		}
		else if (line[0] == 's') // 's' for "setting"
		{
			const std::size_t split = line.find('|');
			if (split != std::string::npos)
			{
				const std::string setting = line.substr(2, split - 2);
				const std::string value = line.substr(split + 1);

				if (setting == "map_dark")
					map_dark = (value == "true");

				std::cout << "level setting: '" << setting << "' set to: '" << value << "'" << std::endl;
			}
			else std::cout << "setting definition is missing '|' delimiter! script line:\n" << line << std::endl;
		}
		else if (line[0] != '\n') // Other lines define the map itself
		{
			std::vector<MapNode> map_line;
			MapNode temp_map_node = { nullptr, nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false, false, false, 0 };
			for (char c : line)
			{
				if (nodes.find(c) != nodes.end()) // If the character is defined as a node, update temp_map_node
				{
					if (!floor_layer)
					{
						temp_map_node.wall_texture = nodes[c].sub_texture;
						temp_map_node.wall_type = nodes[c].sub_type;
						temp_map_node.wall_animated = nodes[c].sub_animated;
					}
					else temp_map_node.floor_texture = nodes[c].sub_texture;
				}
				else if (c == '@') // The at-sign tells where the player will spawn
				{
					player_x = map_x;
					player_y = map_y;
				}
				else // And if it's a monster or an object, store the map location for later
				{
					// They cannot be spawned right now, since the map isn't actually entirely loaded yet,
					// and there might be errors later on, so we just store the locations for now
					std::vector<std::string> md_types;
					for (ScriptDef md : monster_defs)
					{
						// Multiple definitions for a single character mostly come into play for random
						// monsters of a certain type, like zombies ('random1', 'random2', 'random3' etc.)
						if (md.map_char == c)
							md_types.push_back(md.type);
					}
					if (md_types.size() > 0)
					{
						PositionData temp = { md_types[engine.get_rng() % md_types.size()], map_x, map_y };
						monster_spawns.push_back(temp);
						std::cout << "monster spawn created: (" << (int)(map_x) << ", " << (int)(map_y) << ") '" << temp.data << "'" << std::endl;
					}
					// If there was no monster definitions for the character, loop the object definitions.
					else for (ScriptDef od : object_defs)
					{
						if (od.map_char == c)
						{
							PositionData temp = { std::to_string(c), map_x, map_y };
							object_spawns.push_back(temp);
							std::cout << "object spawn created: (" << (int)(map_x) << ", " << (int)(map_y) << ") '" << temp.data << "'" << std::endl;
							break;
						}
					}
				}
				if (!floor_layer) // Every other character ends a node definition
				{
					map_line.push_back(temp_map_node);
					temp_map_node = { nullptr, nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false, false, false, 0 };
					map_x += 1;
				}
				floor_layer = !floor_layer;
			}
			map_data.push_back(map_line);
			map_y += 1; map_x = 0;
		}
	}
	level_file.close();
	map_loaded = true;

	map_width = (uint8_t)map_data[0].size();
	map_height = (uint8_t)map_data.size();

	std::cout << "map loaded, size: " << (int)(map_width) << ", " << (int)(map_height) << std::endl;

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

	// Determine which scripts we need to load for all the monsters on the map
	std::unordered_map<std::string, std::vector<std::string> > scripts;
	for (ScriptDef md : monster_defs)
	{
		auto it = scripts.find(md.script);
		if (it == scripts.end())
		{
			std::vector<std::string> monster_types;
			monster_types.push_back(md.type);
			scripts[md.script] = monster_types;
		}
		else it->second.push_back(md.type);
	}
	monster_defs.clear();

	// Load all of the monster data that we need from the scripts
	for (auto script : scripts)
	{
		lua_State *temp_script = engine.get_lua_manager()->load_script(script.first);
		if (temp_script == nullptr)
			continue;

		// We only load each script once, but run the getter function for each desired monster separately
		for (uint8_t i = 0; i < script.second.size(); i++)
		{
			lua_getglobal(temp_script, "get_monster_data");
			lua_pushstring(temp_script, script.second[i].c_str());

			if (lua_pcall(temp_script, 1, 1, 0) == LUA_OK)
			{
				std::string temp_name;
				MonsterData md; md.id = script.second[i];

				for (lua_pushnil(temp_script); lua_next(temp_script, -2); lua_pop(temp_script, 1))
				{
					const std::string key = lua_tostring(temp_script, -2);
					switch (djb_hash(key.c_str()))
					{
						case djb_hash("name"):
							temp_name = lua_tostring(temp_script, -1);
							std::replace(temp_name.begin(), temp_name.end(), '_', ' ');
							md.name = temp_name;
							break;
						case djb_hash("texture"):
							md.texture = lua_tostring(temp_script, -1);
							break;
						case djb_hash("script"):
							md.script = lua_tostring(temp_script, -1);
							break;
						case djb_hash("str"): md.skills[A_STR] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("dex"): md.skills[A_DEX] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("con"): md.skills[A_CON] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("int"): md.skills[A_INT] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("wis"): md.skills[A_WIS] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("cha"): md.skills[A_CHA] = (int8_t)lua_tointeger(temp_script, -1); break;
						case djb_hash("lck"): md.skills[A_LCK] = (int8_t)lua_tointeger(temp_script, -1); break;

						default: std::cout << "undefined enemy key: " << key << std::endl;
					}
				}
				monster_data.push_back(md);
				std::cout << "monster data loaded for: " << md.id << std::endl;
			}
			else std::cout << "lua error: " << lua_tostring(temp_script, -1) << std::endl;
		}
		engine.get_lua_manager()->free_script(script.first);
	}
	scripts.clear();

	// Spawn all of the monsters we stored earlier with the recently loaded monster data
	for (PositionData pd : monster_spawns)
	{
		for (MonsterData md : monster_data)
		{
			if (pd.data == md.id)
			{
				Monster *temp_monster = new Monster;
				temp_monster->init(md.name, pd.grid_x, pd.grid_y, md.texture);
				temp_monster->init_script(md.script);
				temp_monster->init_healthbar("core/texture/ui/healthbar.png");
				temp_monster->set_skills(md.skills);
				set_actor(pd.grid_x, pd.grid_y, temp_monster);
				monsters.push_back(temp_monster); break;
			}
		}
	}
	monster_spawns.clear();
	monster_data.clear();

	// Spawn all of the objects we stored earlier
	for (PositionData pd : object_spawns)
	{
		for (ScriptDef od : object_defs)
		{
			if (std::to_string(od.map_char) == pd.data)
			{
				std::string data = "";
				for (PositionData oi : object_data)
				{
					// Check if there's any data to assign for this object
					if (oi.grid_x == pd.grid_x && oi.grid_y == pd.grid_y)
					{
						data = oi.data;
						break;
					}
				}
				// Objects are good to go, just like that
				spawn_object(pd.grid_x, pd.grid_y, od.script, od.type, data);
				break;
			}
		}
	}
	object_defs.clear();
	object_spawns.clear();
	object_data.clear();

	if (player_x != UINT8_MAX && player_y != UINT8_MAX) // Move the player to the starting position
	{
		player->set_position(player_x, player_y);
		camera.update_position(player_x * 32, player_y * 32, true);
		set_actor(player_x, player_y, player);
		do_fov(player_x, player_y, player->get_visibility_range());

		player->set_health(std::make_pair(9, 9));
	}
	else while (true) // If the map doesn't setup the player, throw them somewhere random
	{
		const uint8_t xpos = engine.get_rng() % map_width;
		const uint8_t ypos = engine.get_rng() % map_height;

		if (!get_wall(xpos, ypos, true))
		{
			player->set_position(xpos, ypos);
			camera.update_position(xpos * 32, ypos * 32, true);
			set_actor(xpos, ypos, player);
			do_fov(xpos, ypos, player->get_visibility_range()); break;
		}
	}
}
void Level::generate_map(const std::string &script_name)
{
	const uint32_t start_time = engine.get_current_time();
	if (generator_script == nullptr)
	{
		const std::string new_name = "core/script/level/" + script_name;
		if (new_name != generator_script_name)
			generator_depth = 0;
		else generator_depth += 1;

		generator_script_name = new_name;
		generator_script = engine.get_lua_manager()->load_script(generator_script_name);
	}
	if (generator_script != nullptr)
	{
		if (!generator_init)
		{
			lua_register(generator_script, "get_base_path", lua_get_base_path);
			generator_init = true;
		}
		lua_getglobal(generator_script, "generate");
		lua_pushnumber(generator_script, generator_depth);

		if (lua_pcall(generator_script, 1, 0, 0) != LUA_OK)
			std::cout << "lua error: " << lua_tostring(generator_script, -1) << std::endl;

		load_map("temp.txt");
	}
	const uint32_t duration = SDL_GetTicks() - start_time;
	std::cout << "level generation took " << std::to_string(duration) << " milliseconds" << std::endl;
}
void Level::spawn_object(uint8_t xpos, uint8_t ypos, const std::string &script, const std::string &type, const std::string &data)
{
	if (get_object(xpos, ypos) != nullptr)
		return;

	Object *temp_obj = new Object;
	if (temp_obj->construct(xpos, ypos, script, type))
	{
		if (data.length() > 0)
			temp_obj->save_data(data);

		set_object(xpos, ypos, temp_obj);
		objects.push_back(temp_obj);
	}
	else delete temp_obj;
}
void Level::update_objects()
{
	bool refresh_fov = false;
	std::vector<Object*> to_erase;

	for (Object *obj : objects)
	{
		obj->update();
		if (obj->get_delete())
			to_erase.push_back(obj);
	}
	for (Object *obj : to_erase)
	{
		if (obj->get_opaque())
			refresh_fov = true;

		const uint8_t xpos = obj->get_grid_x();
		const uint8_t ypos = obj->get_grid_y();

		std::vector<Object*>::iterator pos = std::find(objects.begin(), objects.end(), obj);
		if (pos != objects.end())
			objects.erase(pos);
		delete obj;
		map_data[ypos][xpos].occupying_object = nullptr;
	}
	if (refresh_fov)
		do_fov(player->get_grid_x(), player->get_grid_y(), player->get_visibility_range());
}
void Level::damage_node(uint8_t xpos, uint8_t ypos, int8_t damage, const std::string &source)
{
	Actor *to_damage = get_actor(xpos, ypos);
	if (to_damage != nullptr)
		to_damage->take_damage(this, damage, source);
}
/*void Level::save_object_data(uint8_t xpos, uint8_t ypos, const std::string &data)
{
	for (Object *o : objects)
	{
		if (o->get_grid_x() == xpos && o->get_grid_y() == ypos)
		{
			o->save_data(data);
			break;
		}
	}
}*/
bool Level::get_node_discovered(uint8_t xpos, uint8_t ypos) const
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return false;
	return map_data[ypos][xpos].node_discovered;
}
bool Level::get_wall(int8_t xpos, int8_t ypos, bool check_occupying) const
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return true;
	if (check_occupying)
	{
		if (map_data[ypos][xpos].occupying_actor != nullptr)
			return true;

		// BUG - If there's a "passable" object on top of a wall, you can walk through it. I probably wont fix this,
		// as it enables a "summon door" spell, so the responsibility of not creating objects on top of walls is on
		// the object's spawner/script. Now to make sure it wont be abusable in-game ...

		if (map_data[ypos][xpos].occupying_object != nullptr && !map_data[ypos][xpos].occupying_object->get_passable())
			return true;
	}
	return map_data[ypos][xpos].wall_texture != nullptr;
}
bool Level::get_wall_light(int8_t xpos, int8_t ypos, uint8_t distance) const
{
	// This function is used to determine whether or not LIGHT should pass through the given position,
	// returning true when the passage it blocked (similar to regular 'get_wall'), and false when it's passable.

	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return true;
	if (map_data[ypos][xpos].occupying_object != nullptr)
		return map_data[ypos][xpos].occupying_object->get_opaque();

	if (map_data[ypos][xpos].wall_texture != nullptr)
	{
		if (map_data[ypos][xpos].wall_type == NT_HOLE) // Light goes over holes (and lakes) without a problem
			return false;
		if (map_data[ypos][xpos].wall_type == NT_TREE && distance < 2) // Through trees if the source is close enough
			return false;
		return true;
	}
	return false;
}
Actor* Level::get_actor(uint8_t xpos, uint8_t ypos) const
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return nullptr;
	return map_data[ypos][xpos].occupying_actor;
}
Object* Level::get_object(uint8_t xpos, uint8_t ypos) const
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return nullptr;
	return map_data[ypos][xpos].occupying_object;
}
void Level::set_actor(uint8_t xpos, uint8_t ypos, Actor *actor)
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return;
	map_data[ypos][xpos].occupying_actor = actor;
}
void Level::set_object(uint8_t xpos, uint8_t ypos, Object *object)
{
	if (!map_loaded || xpos < 0 || ypos < 0 || xpos >= map_width || ypos >= map_height)
		return;
	map_data[ypos][xpos].occupying_object = object;
}
void Level::erase_actor(uint8_t xpos, uint8_t ypos)
{
	Monster *to_erase = dynamic_cast<Monster*>(get_actor(xpos, ypos));
	if (to_erase != nullptr)
	{
		std::vector<Monster*>::iterator pos = std::find(monsters.begin(), monsters.end(), to_erase);
		if (pos != monsters.end())
			monsters.erase(pos);
		delete to_erase;
		map_data[ypos][xpos].occupying_actor = nullptr;
	}
}
void Level::erase_object(uint8_t xpos, uint8_t ypos)
{
	Object *to_erase = dynamic_cast<Object*>(get_object(xpos, ypos));
	if (to_erase != nullptr)
		to_erase->set_delete(true);
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
	blank_tile = engine.get_texture_manager()->load_texture("core/texture/color/black.png");
	textures.push_back(blank_tile);
	refresh_map_texture(true);
}
void Level::render_map_texture() const
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
void Level::refresh_map_texture(bool first_time, bool animated_only)
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

			if (first_time && !map_data[y][x].node_discovered)
			{
				blank_tile->render(x * 32, y * 32, &default_rect);
				continue;
			}
			if (map_data[y][x].floor_texture != nullptr)
			{
				map_data[y][x].floor_texture->render(x * 32, y * 32, &map_data[y][x].floor_rect);
				decorate_map_node(x, y);
			}
			else blank_tile->render(x * 32, y * 32, &default_rect);

			if (map_data[y][x].wall_texture != nullptr && map_data[y][x].wall_type != NT_INVISIBLE)
				map_data[y][x].wall_texture->render(x * 32, y * 32, &map_data[y][x].wall_rect);
		}
	}
	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
void Level::decorate_map_node(uint8_t xpos, uint8_t ypos)
{
	for (MapDecor decor : decorations)
	{
		if (decor.grid_x == xpos && decor.grid_y == ypos)
		{
			decor.texture->render(xpos * 32, ypos * 32, &decor.rect);
			return;
		}
	}
	if (map_data[ypos][xpos].decor_initialized)
		return;
	map_data[ypos][xpos].decor_initialized = true;

	if (decor_IDs.size() > 0 && engine.get_rng() % 10 == 0)
	{
		MapDecor temp_decor = { nullptr, xpos, ypos };
		temp_decor.texture = textures[decor_IDs[engine.get_rng() % decor_IDs.size()]];

		if (temp_decor.texture != nullptr)
		{
			temp_decor.rect = { 0, 0, 16, 16 };
			temp_decor.rect.x = (engine.get_rng() % temp_decor.texture->get_tiles_horizontal()) * 16;
			temp_decor.rect.y = (engine.get_rng() % temp_decor.texture->get_tiles_vertical()) * 16;
			temp_decor.texture->render(xpos * 32, ypos * 32, &temp_decor.rect);
			decorations.push_back(temp_decor);
		}
	}
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
	// This is not very good, but it's only called once for each node definition at the top of a map file

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
void Level::cast_light(uint8_t x, uint8_t y, uint8_t radius, uint8_t row,
	float start_slope, float end_slope, uint8_t xx, uint8_t xy, uint8_t yx, uint8_t yy)
{
	// This is based on Björn Bergström's recursive shadowcasting algorithm:
	// http://www.roguebasin.com/index.php?title=FOV_using_recursive_shadowcasting

	if (start_slope < end_slope) return;
	float next_start = start_slope;

	for (uint8_t i = row; i <= radius; i++)
	{
		bool blocked = false;
		for (int8_t dx = -i, dy = -i; dx <= 0; dx++)
		{
			const float l_slope = (float)((dx - 0.5) / (dy + 0.5));
			const float r_slope = (float)((dx + 0.5) / (dy - 0.5));

			if (start_slope < r_slope) continue;
			if (end_slope > l_slope) break;

			const int8_t sax = dx * xx + dy * xy;
			const int8_t say = dx * yx + dy * yy;

			if ((sax < 0 && std::abs(sax) > x) || (say < 0 && std::abs(say) > y))
				continue;
			const uint8_t ax = x + sax;
			const uint8_t ay = y + say;

			if (ax >= map_width || ay >= map_height)
				continue;
			if ((dx * dx + dy * dy) < (radius * radius))
			{
				if (!map_data[ay][ax].node_discovered || map_dark)
					map_data[ay][ax].node_rendered = false;
				map_data[ay][ax].node_discovered = true;
			}
			if (blocked)
			{
				if (get_wall_light(ax, ay, i))
				{
					next_start = r_slope;
					continue;
				}
				else
				{
					blocked = false;
					start_slope = next_start;
				}
			}
			else if (get_wall_light(ax, ay, i))
			{
				blocked = true;
				next_start = r_slope;
				cast_light(x, y, radius, i + 1, start_slope, l_slope, xx, xy, yx, yy);
			}
		}
		if (blocked) break;
	}
}
static const int8_t octants[4][8] =
{
	{ 1, 0, 0, -1, -1, 0, 0, 1 }, // First 4 octants (loop 0-3) = up
	{ 0, 1, -1, 0, 0, -1, 1, 0 }, // Last 4 octants (loop 4-7) = down
	{ 0, 1, 1, 0, 0, -1, -1, 0 }, // Middle 4 octants (loop 2-5) = right
	{ 1, 0, 0, 1, -1, 0, 0, -1 }  // First & last 2 octants (0,1,6,7) = left
};
void Level::do_fov(uint8_t xpos, uint8_t ypos, uint8_t radius, bool force)
{
	if (!map_dark) // Only recalculate the fov for a square if the radius is bigger than previously
	{
		if (!force && map_data[ypos][xpos].los_calculated >= radius)
			return;
		map_data[ypos][xpos].los_calculated = radius;
	}
	// Start by making the light source position visible
	if (!map_data[ypos][xpos].node_discovered || map_dark)
		map_data[ypos][xpos].node_rendered = false;
	map_data[ypos][xpos].node_discovered = true;

	for (uint8_t i = 0; i < 8; i++) // Cast light for each octant separately
		cast_light(xpos, ypos, radius, 1, 1.0, 0.0, octants[0][i], octants[1][i], octants[2][i], octants[3][i]);

	if (map_dark) // Set all nodes that we didn't discover back to dark
	{
		for (uint8_t y = 0; y < map_height; y++)
		{
			for (uint8_t x = 0; x < map_width; x++)
			{
				if (map_data[y][x].node_rendered)
				{
					map_data[y][x].node_discovered = false;
					map_data[y][x].node_rendered = false;
				}
			}
		}
	}
	refresh_map_texture(map_dark);
}
