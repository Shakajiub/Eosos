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

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

class MessageBox
{
public:
	MessageBox();
	~MessageBox();

	void free();
	bool init(const std::string &title, const std::string &message, uint16_t xpos, uint16_t ypos);
	void render() const;

	bool get_overlap(int16_t mouse_x, int16_t mouse_y) const;
	bool get_click(int16_t mouse_x, int16_t mouse_y) const;

private:
	int16_t x, y;
	uint8_t width, height;

	std::string box_title;
	std::string box_message;
	SDL_Texture *box_background;
};

#endif // MESSAGE_BOX_HPP
