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

#ifndef BITMAPFONT_HPP
#define BITMAPFONT_HPP

class Texture;

class BitmapFont
{
public:
	BitmapFont();
	~BitmapFont();

	void free();
	bool build(const std::string &texture_name);

	void render_text(int16_t xpos, int16_t ypos, const std::string &text, uint8_t line_length = 0) const;
	void render_char(int16_t xpos, int16_t ypos, uint8_t character) const;
	void draw_frame(uint8_t xpos, uint8_t ypos, uint8_t width, uint8_t height) const;

	uint8_t get_width() const { return font_width; }
	uint8_t get_height() const { return font_height; }
	uint8_t get_scale() const { return font_scale; }

	void set_scale(int16_t new_scale);
	void set_color(SDL_Color color);

private:
	uint8_t font_width, font_height, font_scale;
	SDL_Rect font_chars[256];
	Texture *font_bitmap;
};

#endif // BITMAPFONT_HPP
