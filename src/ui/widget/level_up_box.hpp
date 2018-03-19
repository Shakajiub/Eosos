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

#ifndef LEVEL_UP_BOX_HPP
#define LEVEL_UP_BOX_HPP

#include "widget.hpp"

#include <vector>

class Hero;
class Texture;

typedef struct
{
	bool overlap;
	Texture *texture;
	std::string title;
	std::string message;
}
LevelOption;

class LevelUpBox : public Widget
{
public:
	LevelUpBox();
	~LevelUpBox();

	bool init(Hero *hero);

	virtual void free();
	virtual void render() const;

	virtual bool input_keyboard_down(SDL_Keycode key);

	virtual bool get_overlap(int16_t mouse_x, int16_t mouse_y);
	virtual bool get_click(int16_t mouse_x, int16_t mouse_y);

private:
	bool mouse_input;
	Hero *temp_hero;

	SDL_Texture *selection_box;
	std::vector<LevelOption> level_options;
	int8_t selected_option;
};

#endif // LEVEL_UP_BOX_HPP
