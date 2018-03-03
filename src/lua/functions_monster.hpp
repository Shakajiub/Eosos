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

#ifndef FUNCTIONS_MONSTER_HPP
#define FUNCTIONS_MONSTER_HPP

// This function registers all of the functions below for a given script
void register_monster_functions(lua_State *script);

// These are public incase we want to choose specifically which functions to register
int lua_end_turn(lua_State *script);
int lua_get_position(lua_State *script);
int lua_get_player_visible(lua_State *script);
int lua_get_node_downhill(lua_State *script);
int lua_get_in_dijkstra_map(lua_State *script);
int lua_get_state_var(lua_State *script);
int lua_set_state_var(lua_State *script);
int lua_load_bubble(lua_State *script);
int lua_turn_around(lua_State *script);

#endif // FUNCTIONS_MONSTER_HPP
