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
#include "widget.hpp"

Widget::Widget() : widget_activated(false), widget_name("???")
{

}
Widget::~Widget()
{

}
bool Widget::input_keyboard_down(SDL_Keycode key)
{

}
bool Widget::input_mouse_button_down(SDL_Event eve)
{

}
bool Widget::input_joy_button_down(uint8_t index, uint8_t value)
{

}
bool Widget::input_joy_hat_motion(uint8_t index, uint8_t value)
{

}
bool Widget::input_text(const std::string &input)
{

}
bool Widget::get_overlap(int16_t mouse_x, int16_t mouse_y)
{
	return false;
}
bool Widget::get_click(int16_t mouse_x, int16_t mouse_y)
{
	return false;
}
