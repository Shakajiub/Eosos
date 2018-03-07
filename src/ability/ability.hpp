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

#ifndef ABILITY_HPP
#define ABILITY_HPP

class Hero;

class Ability
{
public:
	Ability();
	~Ability();

	void free();

	virtual bool init() = 0;
	virtual void apply(Hero *hero) = 0;

	void render(uint16_t xpos, uint16_t ypos) const;
	bool init_texture(const std::string &icon, SDL_Color color);

	std::string get_ability_name() const { return ability_name; }

	bool get_hovered() const { return hovered; }
	void set_hovered(bool h) { hovered = h; }

protected:
	bool hovered;

	SDL_Texture *ability_texture;
	std::string ability_name;

	std::pair<uint8_t, uint8_t> cooldown;
};

#endif // ABILITY_HPP
