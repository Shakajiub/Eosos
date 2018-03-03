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

#ifndef LUA_MANAGER_HPP
#define LUA_MANAGER_HPP

#include <unordered_map>

class LuaManager
{
public:
	LuaManager();
	~LuaManager();

	void free();

	lua_State* load_script(const std::string &script_name);
	void init_script(const std::string &script_name);
	void free_script(const std::string &script_name);

	uint16_t get_reference_count(const std::string &script_name) const;

private:
	std::unordered_map<std::string, lua_State*> script_map;
	std::unordered_map<std::string, uint16_t> reference_count;
};

#endif // LUA_MANAGER_HPP
