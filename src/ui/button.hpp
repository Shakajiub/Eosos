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

#ifndef BUTTON_HPP
#define BUTTON_HPP

enum ButtonType
{
	BTN_WIDGET_TOGGLE,
	BTN_DIRECTION_KEY
};
class Button
{
public:
	Button();
	~Button();

	void free();
	void init(ButtonType type, uint8_t value = 0);
	void render() const;

	bool get_enabled() const { return btn_enabled; }
	bool get_overlap(int16_t xpos, int16_t ypos);
	bool get_click() const { return btn_rect.x != 0; }

	void set_enabled(bool enabled) { btn_enabled = enabled; }
	void set_consider_camera(bool consider) { consider_camera = consider; }
	void set_position(int16_t xpos, int16_t ypos) { x = xpos; y = ypos; }
	void set_state(uint8_t state);

private:
	bool btn_enabled;
	bool consider_camera;
	int16_t x, y;
	uint8_t states;

	SDL_Rect btn_rect;
	SDL_Texture *btn_texture;
};

#endif // BUTTON_HPP
