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
#include "bitmap_font.hpp"
#include "texture.hpp"

#include "logging.hpp"
#include "options.hpp"
#include "texture_manager.hpp"

SDL_Color char_to_color(char hex)
{
	switch (hex)
	{
		case '0': return DAWN_BLACK; case '1': return DAWN_PLUM;  case '2': return DAWN_MIDNIGHT;
		case '3': return DAWN_IRON;  case '4': return DAWN_EARTH; case '5': return DAWN_MOSS;
		case '6': return DAWN_BERRY; case '7': return DAWN_OLIVE; case '8': return DAWN_CORNFLOWER;
		case '9': return DAWN_OCHER; case 'A': return DAWN_SLATE; case 'B': return DAWN_LEAF;
		case 'C': return DAWN_PEACH; case 'D': return DAWN_SKY;   case 'E': return DAWN_MAIZE;
		default:  return DAWN_PEPPERMINT;
	}
	return DAWN_BLACK; // Should not ever get here
}
BitmapFont::BitmapFont() : font_width(0), font_height(0), font_scale(1), font_bitmap(nullptr)
{

}
BitmapFont::~BitmapFont()
{
	free();
}
void BitmapFont::free()
{
	if (font_bitmap != nullptr)
	{
		delete font_bitmap;
		font_bitmap = nullptr;
	}
}
bool BitmapFont::build(const std::string &texture_name)
{
	free();

	font_bitmap = new Texture;
	if (!font_bitmap->load_from_file(texture_name, true))
	{
		delete font_bitmap;
		font_bitmap = nullptr;
		return false;
	}
	font_width = font_bitmap->get_width() / 16;
	font_height = font_bitmap->get_height() / 16;

	uint8_t current_char = 0;
	for (uint8_t rows = 0; rows < 16; rows++)
	{
		for (uint8_t cols = 0; cols < 16; cols++)
		{
			font_chars[current_char].x = font_width * cols;
			font_chars[current_char].y = font_height * rows;
			font_chars[current_char].w = font_width;
			font_chars[current_char].h = font_height;
			current_char += 1;
		}
	}
	font_bitmap->set_color(DAWN_PEPPERMINT);

	logging.cout(std::string("Font loaded, size: ") + std::to_string(font_width) + ", " + std::to_string(font_height), LOG_ENGINE);
	return true;
}
void BitmapFont::render_text(int16_t xpos, int16_t ypos, const std::string &text, uint8_t line_length) const
{
	if (font_bitmap == nullptr)
		return;

	std::string final_text = text;
	if (line_length > 0) // Split the text into multiple lines
	{
		uint8_t space_pos = 0;
		uint8_t prev_space = 0;
		uint8_t prev_length = 0;

		bool text_split = false;
		while (!text_split)
		{
			prev_space = space_pos;
			space_pos = (uint8_t)final_text.find(' ', prev_space + 1);

			if (space_pos == 255)
				text_split = true;

			else if (space_pos > line_length + prev_length + 2)
			{
				final_text[prev_space] = '\n';
				prev_length = prev_space;
			}
		}
	}
	const uint8_t original_x = (uint8_t)xpos;
	const uint16_t screen_x = xpos; xpos = 0;
	const uint16_t screen_y = ypos; ypos = 0;

	// Draw the text one character at a time
	for (uint8_t i = 0; i < (uint8_t)final_text.length(); i++)
	{
		if (final_text[i] == ' ')
			xpos += 1;

		else if (final_text[i] == '\n')
		{
			ypos += 1;
			xpos = 0;
		}
		else if (final_text[i] == '%')
		{
			font_bitmap->set_color(char_to_color(final_text[i + 1]));
			i += 1; // Skip the color-defining character
		}
		else
		{
			font_bitmap->render(
				screen_x + (xpos * font_width), screen_y + (ypos * font_height),
				&font_chars[(uint8_t)final_text[i]], font_scale
			);
			xpos += 1;
		}
	}
}
void BitmapFont::render_char(int16_t xpos, int16_t ypos, uint8_t character) const
{
	font_bitmap->render(xpos, ypos, &font_chars[character], font_scale);
}
void BitmapFont::draw_frame(uint8_t xpos, uint8_t ypos, uint8_t width, uint8_t height) const
{
	// TODO - Fix the rendering positions

	render_char(xpos, ypos, 218); // Top left corner
	render_char(xpos + width, ypos, 191); // Top right corner
	render_char(xpos, ypos + height, 192); // Bottom left corner
	render_char(xpos + width, ypos + height, 217); // Bottom right corner

	for (uint8_t bar_x = xpos + 1; bar_x < xpos + width; bar_x++)
		render_char(bar_x, ypos, 196); // Top bar
	for (uint8_t bar_x = xpos + 1; bar_x < xpos + width; bar_x++)
		render_char(bar_x, ypos + height, 196); // Bottom bar

	for (uint8_t bar_y = ypos + 1; bar_y < ypos + height; bar_y++)
		render_char(xpos, bar_y, 179); // Left bar
	for (uint8_t bar_y = ypos + 1; bar_y < ypos + height; bar_y++)
		render_char(xpos + width, bar_y, 179); // Right bar
}
void BitmapFont::set_scale(int16_t new_scale)
{
	font_width = font_bitmap->get_width() / 16;
	font_height = font_bitmap->get_height() / 16;

	if (new_scale > 0)
	{
		font_scale = (uint8_t)new_scale;
		font_width *= new_scale;
		font_height *= new_scale;
	}
}
void BitmapFont::set_color(SDL_Color color)
{
	if (font_bitmap != nullptr)
		font_bitmap->set_color(color);
}
