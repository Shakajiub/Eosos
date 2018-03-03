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
#include "functions_monster.hpp"

#include "monster.hpp"
#include "player.hpp"
#include "dijkstra.hpp"
#include "dungeon.hpp"
#include "camera.hpp"
#include "functions_general.hpp"
#include "scene_manager.hpp"

Monster* get_current_monster()
{
	Monster *current_monster = nullptr;
	Dungeon *scene = engine.get_scene_manager()->get_scene("test");

	if (scene != nullptr)
	{
		Actor *a = scene->get_current_actor();
		if (a != nullptr)
		{
			Monster *m = dynamic_cast<Monster*>(a);
			if (m != NULL)
				current_monster = m;
		}
	}
	return current_monster;
}
void register_monster_functions(lua_State *script)
{
	if (script != nullptr)
	{
		lua_register(script, "end_turn", lua_end_turn);
		lua_register(script, "get_position", lua_get_position);
		lua_register(script, "get_player_visible", lua_get_player_visible);
		lua_register(script, "get_node_downhill", lua_get_node_downhill);
		lua_register(script, "get_in_dijkstra_map", lua_get_in_dijkstra_map);
		lua_register(script, "get_state_var", lua_get_state_var);
		lua_register(script, "set_state_var", lua_set_state_var);
		lua_register(script, "load_bubble", lua_load_bubble);
		lua_register(script, "turn_around", lua_turn_around);
	}
}
int lua_end_turn(lua_State *script)
{
	Monster *current_monster = get_current_monster();
	if (current_monster != nullptr)
		current_monster->set_turn_done(true);

	return 0;
}
int lua_get_position(lua_State *script)
{
	uint8_t pos_x = 0, pos_y = 0;

	Monster *current_monster = get_current_monster();
	if (current_monster != nullptr)
	{
		pos_x = current_monster->get_grid_x();
		pos_y = current_monster->get_grid_y();
	}
	lua_pushnumber(script, pos_x);
	lua_pushnumber(script, pos_y);
	return 2;
}
int lua_get_player_visible(lua_State *script)
{
	bool player_visible = false;

	Monster *current_monster = get_current_monster();
	if (current_monster != nullptr)
		player_visible = current_monster->get_player_visible(engine.get_scene_manager()->get_scene("test"));

	lua_pushboolean(script, player_visible);
	return 1;
}
int lua_get_node_downhill(lua_State *script)
{
	uint8_t new_x = 0, new_y = 0;
	if (check_arguments(script, 2, "get_node_downhill"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		// Get a suitable position to move towards from the player's dijkstra map
		Dungeon *scene = engine.get_scene_manager()->get_scene("test");
		std::pair<uint8_t, uint8_t> move_pos = player->get_dijkstra()->get_node_downhill(scene, grid_x, grid_y);

		new_x = move_pos.first;
		new_y = move_pos.second;
	}
	lua_pushnumber(script, new_x);
	lua_pushnumber(script, new_y);
	return 2;
}
int lua_get_in_dijkstra_map(lua_State *script)
{
	bool in_map = false;
	if (check_arguments(script, 2, "get_in_dijkstra_map"))
	{
		const uint8_t grid_x = (uint8_t)lua_tointeger(script, -2);
		const uint8_t grid_y = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		// Check if the given position is included in the player's dijkstra map
		in_map = player->get_dijkstra()->check_in_map(grid_x, grid_y);
	}
	lua_pushboolean(script, in_map);
	return 1;
}
int lua_get_state_var(lua_State *script)
{
	bool value = false;
	if (check_arguments(script, 1, "get_state_var"))
	{
		const std::string var = lua_tostring(script, -1);
		lua_pop(script, 1);

		Monster *current_monster = get_current_monster();
		if (current_monster != nullptr)
			value = current_monster->get_state_var(var);
	}
	lua_pushboolean(script, value);
	return 1;
}
int lua_set_state_var(lua_State *script)
{
	if (check_arguments(script, 2, "set_state_var"))
	{
		const std::string var = lua_tostring(script, -2);
		const bool value = lua_toboolean(script, -1);
		lua_pop(script, 2);

		Monster *current_monster = get_current_monster();
		if (current_monster != nullptr)
			current_monster->set_state_var(var, value);
	}
	return 0;
}
int lua_load_bubble(lua_State *script)
{
	if (check_arguments(script, 2, "load_bubble"))
	{
		const std::string bubble_name = lua_tostring(script, -2);
		const uint8_t bubble_timer = (uint8_t)lua_tointeger(script, -1);
		lua_pop(script, 2);

		Monster *current_monster = get_current_monster();
		if (current_monster != nullptr)
			current_monster->load_bubble(bubble_name, bubble_timer);
	}
	return 0;
}
int lua_turn_around(lua_State *script)
{
	Monster *current_monster = get_current_monster();
	if (current_monster != nullptr)
		current_monster->turn_around();

	return 0;
}
