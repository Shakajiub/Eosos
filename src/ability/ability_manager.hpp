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

class Dungeon;

class AbilityManager
{
public:
	AbilityManager();
	~AbilityManager();

	void free();

	void init_abilities(Dungeon *scene);
	void render_abilities() const;
	bool overlap_abilities(int16_t xpos, int16_t ypos) const;
	bool click_abilities() const;
	void trigger_abilities(uint8_t trigger);
	void cooldown_abilities(uint8_t amount = 1);

	bool keyboard_input(SDL_Keycode key);
};

#endif // ABILITY_MANAGER_HPP
