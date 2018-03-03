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

#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "actor.hpp"
#include <unordered_map>

class Monster : public Actor
{
public:
	Monster();
	~Monster();

	virtual void update(Dungeon *scene);
	virtual void render() const;
	virtual bool take_turn(Dungeon *scene);

	void init_script(const std::string &script);
	void init_healthbar(const std::string &hb_texture);

	bool get_state_var(const std::string &state);
	void set_state_var(const std::string &state, bool value);
	void set_turn_done(bool done) { turn_done = done; }

private:
	std::unordered_map<std::string, bool> state_vars;
	std::string monster_script_name;
	lua_State *monster_script;
	Texture *healthbar;
};

#endif // MONSTER_HPP
