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
#include "lua_manager.hpp"

LuaManager::LuaManager()
{

}
LuaManager::~LuaManager()
{
	free();
}
void LuaManager::free()
{
	for (auto it : script_map)
		lua_close(it.second);

	script_map.clear();
	reference_count.clear();

	std::cout << "all scripts erased" << std::endl;
}
lua_State* LuaManager::load_script(const std::string &script_name)
{
	// If we haven't already loaded the script, do so
	auto it = script_map.find(script_name);
	if (it == script_map.end())
	{
		lua_State *temp_script = luaL_newstate();
		luaL_openlibs(temp_script);

		// Make sure the script exists
		if (luaL_loadfile(temp_script, (engine.get_base_path() + script_name).c_str()) != LUA_OK)
		{
			std::cout << "could not load script: " << script_name << std::endl;
			std::cout << "lua error: " << lua_tostring(temp_script, -1) << std::endl;
			lua_close(temp_script);
			return nullptr;
		}
		else script_map[script_name] = temp_script;
		reference_count[script_name] = 1;

		init_script(script_name);
		std::cout << "script loaded: " << script_name << std::endl;
	}
	// Otherwise, just increase it's reference count
	else reference_count[script_name] += 1;
	return script_map[script_name];
}
void LuaManager::init_script(const std::string &script_name)
{
	auto it = script_map.find(script_name);
	if (it == script_map.end())
	{
		std::cout << "trying to initialize unloaded script: " << script_name << std::endl;
		return;
	}
	if (luaL_dofile(it->second, (engine.get_base_path() + script_name).c_str()) == LUA_OK)
	{
		lua_getglobal(it->second, "init");
		if (lua_pcall(it->second, 0, 0, 0) != LUA_OK)
			std::cout << "lua error: " << lua_tostring(it->second, -1) << std::endl;
	}
	else std::cout << "lua error: " << lua_tostring(it->second, -1) << std::endl;
}
void LuaManager::free_script(const std::string &script_name)
{
	// If given script exists and nothing else is using it, delete it
	auto it = script_map.find(script_name);
	if (it != script_map.end())
	{
		reference_count[script_name] -= 1;
		if (reference_count[script_name] == 0)
		{
			lua_close(it->second);
			script_map.erase(it);
			std::cout << "script freed: " << script_name << std::endl;
		}
	}
}
uint16_t LuaManager::get_reference_count(const std::string &script_name) const
{
	auto it = reference_count.find(script_name);
	if (it != reference_count.end())
		return it->second;
	else return 0;
}
