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

#ifndef WIDGET_HPP
#define WIDGET_HPP

class Widget
{
public:
	Widget();
	~Widget();

	virtual void free() = 0;
	virtual void render() const = 0;

	virtual bool input_keyboard_down(SDL_Keycode key);
	virtual bool input_mouse_button_down(SDL_Event eve);
	virtual bool input_joy_button_down(uint8_t index, uint8_t value);
	virtual bool input_joy_hat_motion(uint8_t index, uint8_t value);
	virtual bool input_text(const std::string &input);

	virtual bool get_overlap(int16_t mouse_x, int16_t mouse_y);
	virtual bool get_click(int16_t mouse_x, int16_t mouse_y);

	bool get_activated() const { return widget_activated; }

	void set_activated(bool activated) { widget_activated = activated; }
	void set_name(const std::string &name) { widget_name = name; }

protected:
	bool widget_activated;
	std::string widget_name;
};

#endif // WIDGET_HPP
