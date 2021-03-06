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

#ifndef ABILITY_SPROUT_HPP
#define ABILITY_SPROUT_HPP

#include "ability.hpp"

#include <vector>

class Texture;

class AbilitySprout : public Ability
{
public:
	AbilitySprout();
	~AbilitySprout();

	void free();

	virtual bool init();
	virtual void apply(Hero *hero, bool cancel);
	virtual void render(uint16_t xpos, uint16_t ypos, SDL_Keycode key) const;
	virtual bool get_click(uint16_t mouse_x, uint16_t mouse_y);
	virtual void clear(Hero *hero);

private:
	std::vector<std::pair<uint8_t, uint8_t> > valid_nodes;
	Texture *target_texture;
	Hero *temp_hero;
};

#endif // ABILITY_SPROUT_HPP
