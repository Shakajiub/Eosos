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

#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include "widget.hpp"

class TextInput : public Widget
{
public:
	TextInput::TextInput();
	TextInput::~TextInput();

	virtual void free();
	virtual void render() const;

	virtual void input_keyboard_down(SDL_Keycode key);
	virtual void input_text(const std::string &input);

	std::strin& get_text() const { return box_text; }
	void set_text(const std::string text) { box_text = text; }

private:
	std::string box_text;
};

#endif // TEXT_INPUT_HPP
