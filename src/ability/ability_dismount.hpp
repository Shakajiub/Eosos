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

#ifndef ABILITY_DISMOUNT_HPP
#define ABILITY_DISMOUNT_HPP

#include "ability.hpp"

#include <vector>

class Texture;

class AbilityDismount : public Ability
{
public:
	AbilityDismount();
	~AbilityDismount();

	void free();

	virtual bool init();
	virtual void apply(Hero *hero);
	virtual void render(uint16_t xpos, uint16_t ypos, SDL_Keycode key) const;
	virtual bool get_click(uint16_t mouse_x, uint16_t mouse_y);
	virtual void clear();

private:
	std::vector<std::pair<uint8_t, uint8_t> > valid_nodes;
	Texture *target_texture;
	Hero *temp_hero;
};

#endif // ABILITY_DISMOUNT_HPP
