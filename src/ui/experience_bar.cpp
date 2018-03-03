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
#include "experience_bar.hpp"

#include "options.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

ExperienceBar::ExperienceBar() : bar_texture(nullptr)
{

}
ExperienceBar::~ExperienceBar()
{
	free();
}
void ExperienceBar::free()
{
	if (bar_texture != nullptr)
	{
		SDL_DestroyTexture(bar_texture);
		bar_texture = nullptr;
	}
}
void ExperienceBar::init()
{
	free();

	bar_texture = SDL_CreateTexture(engine.get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 96, 32);
	if (bar_texture == NULL)
	{
		bar_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}
	refresh_texture();
}
void ExperienceBar::render() const
{
	if (bar_texture != nullptr)
	{
		const SDL_Rect clip = { 0, 0, 96, 32 };
		const SDL_Rect quad = { x, y, 96, 32 };

		SDL_RenderCopyEx(engine.get_renderer(), bar_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
}
bool ExperienceBar::get_overlap(int16_t xpos, int16_t ypos) const
{
	if (xpos > x && xpos < (x + 96) && ypos > y && ypos < (y + 32))
		return true;
	return false;
}
void ExperienceBar::refresh_texture()
{
	if (ui.get_background() == nullptr)
	{
		std::cout << "no proper background for experience bar!" << std::endl;
		return;
	}
	SDL_SetRenderTarget(engine.get_renderer(), bar_texture);

	for (uint8_t i = 0; i < 3; i++)
	{
		SDL_Rect temp_rect = { 0, 48, 16, 16 };
		temp_rect.x += i * 16;

		if (options.get_b("ui-highlight"))
			temp_rect.x += 64;
		ui.get_background()->render(i * 32, 0, &temp_rect);
	}
	const uint8_t prev_scale = ui.get_bitmap_font()->get_scale();

	ui.get_bitmap_font()->set_scale(2); // TODO - Get the actual feeling & experience amount from the player
	ui.get_bitmap_font()->set_color(COLOR_PEACH);
	ui.get_bitmap_font()->render_char(8, 6, 3);
	ui.get_bitmap_font()->render_char(24, 6, 4);
	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
	ui.get_bitmap_font()->render_char(42, 8, '4');
	ui.get_bitmap_font()->render_char(58, 8, '2');
	ui.get_bitmap_font()->render_char(74, 8, '%');
	ui.get_bitmap_font()->set_scale(prev_scale);

	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
