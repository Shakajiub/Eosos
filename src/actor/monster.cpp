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
#include "monster.hpp"

#include "player.hpp"
#include "functions_monster.hpp"
#include "functions_general.hpp"
#include "lua_manager.hpp"

#include "camera.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

Monster::Monster() : monster_script(nullptr), healthbar(nullptr)
{

}
Monster::~Monster()
{
	if (monster_script != nullptr)
	{
		engine.get_lua_manager()->free_script(monster_script_name);
		monster_script = nullptr;
	}
	if (healthbar != nullptr)
	{
		engine.get_texture_manager()->free_texture(healthbar->get_name());
		healthbar = nullptr;
	}
	state_vars.clear();
}
void Monster::update(Dungeon *scene)
{
	Actor::update(scene);
}
void Monster::render() const
{
	Actor::render();

	if (healthbar != nullptr && in_camera)
	{
		const uint8_t hp_percent = (float)health.first / (float)health.second * 14;
		const SDL_Rect temp_rect = { 28 - (hp_percent * 2), 0, 3, 16 };

		healthbar->render(
			x - camera.get_cam_x() + (facing_right ? 0 : 26),
			y - camera.get_cam_y(),
			&temp_rect, 2, SDL_FLIP_NONE, render_angle
		);
	}
}
bool Monster::take_turn(Dungeon *scene)
{
	if (turn_done)
	{
		end_turn();
		turn_done = false;
		return true;
	}
	else if (action_queue.empty() && current_action == nullptr)
	{
		lua_getglobal(monster_script, "take_turn");

		if (lua_pcall(monster_script, 0, 0, 0) != LUA_OK)
		{
			std::cout << "lua error: " << lua_tostring(monster_script, -1) << std::endl;
			end_turn(); return true;
		}
		if (action_queue.empty())
		{
			end_turn();
			turn_done = false;
			return true;
		}
	}
	return false;
}
void Monster::init_script(const std::string &script)
{
	monster_script = engine.get_lua_manager()->load_script(script);
	if (monster_script != nullptr)
	{
		if (engine.get_lua_manager()->get_reference_count(script) < 2)
		{
			lua_register(monster_script, "get_player_pos", lua_get_player_pos);
			lua_register(monster_script, "get_node_blocked", lua_get_node_blocked);
			lua_register(monster_script, "add_action", lua_add_action);
			register_monster_functions(monster_script);
		}
		monster_script_name = script;
	}
}
void Monster::init_healthbar(const std::string &hb_texture)
{
	healthbar = engine.get_texture_manager()->load_texture(hb_texture);
}
bool Monster::get_state_var(const std::string &state)
{
	if (state_vars.find(state) != state_vars.end())
		return state_vars[state];
	return false;
}
void Monster::set_state_var(const std::string &state, bool value)
{
	state_vars[state] = value;

	if (value && state == "awakened" && monster_script != nullptr)
	{
		lua_getglobal(monster_script, "awaken");
		lua_pushnumber(monster_script, grid_x);
		lua_pushnumber(monster_script, grid_y);

		if (lua_pcall(monster_script, 2, 0, 0) != LUA_OK)
			std::cout << "lua error: " << lua_tostring(monster_script, -1) << std::endl;
	}
}
