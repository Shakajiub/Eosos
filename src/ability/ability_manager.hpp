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

#ifndef ABILITY_MANAGER_HPP
#define ABILITY_MANAGER_HPP

#include <vector>

class Ability;
class Hero;

class AbilityManager
{
public:
	AbilityManager();
	~AbilityManager();

	void free();
	void render_ui(Hero *hero) const;
	void load_ability(const std::string &name);

	bool get_overlap(Hero *hero, int16_t mouse_x, int16_t mouse_y) const;
	bool get_click(Hero *hero, int16_t mouse_x, int16_t mouse_y) const;

private:
	std::vector<Ability*> abilities;
};

#endif // ABILITY_MANAGER_HPP
