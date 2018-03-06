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
#include "ability.hpp"

#include "texture_manager.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Ability::Ability() : hovered(false), ability_texture(nullptr)
{
	cooldown = std::make_pair(0, 0);
}
Ability::~Ability()
{
	free();
}
void Ability::free()
{
	if (ability_texture != nullptr)
	{
		SDL_DestroyTexture(ability_texture);
		ability_texture = nullptr;
	}
}
void Ability::render(uint16_t xpos, uint16_t ypos) const
{
	if (ability_texture != nullptr)
	{
		const SDL_Rect rect = { hovered ? 48 : 0, 0, 48, 48 };
		const SDL_Rect quad = { xpos, ypos, 48, 48 };

		SDL_RenderCopyEx(engine.get_renderer(), ability_texture, &rect, &quad, 0.0, nullptr, SDL_FLIP_NONE);

		ui.get_bitmap_font()->set_scale(2);
		ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
		if (cooldown.first > 0)
		{
			const uint8_t c = (cooldown.first > 4) ? 155 : 160 - cooldown.first;
			ui.get_bitmap_font()->render_char(xpos + 30, ypos + 28, c);
		}
		else ui.get_bitmap_font()->render_char(xpos + 30, ypos + 28, hotkey_name[0]);
		ui.get_bitmap_font()->set_scale(1);
	}
}
bool Ability::init_texture(const std::string &icon, SDL_Color color, SDL_Keycode code)
{
	if (ability_texture != nullptr)
		SDL_DestroyTexture(ability_texture);

	ability_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 96, 48
	);
	if (ability_texture == NULL)
	{
		ability_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	if (ui.get_background() != nullptr)
	{
		SDL_SetRenderTarget(engine.get_renderer(), ability_texture);

		SDL_Rect center = { 20, 20, 8, 8 };
		SDL_Rect corners[4] =
		{
			{ 0, 0, 16, 8 }, // Top left
			{ 40, 0, 8, 16 }, // Top right
			{ 0, 32, 8, 16 }, // Bottom left
			{ 32, 40, 16, 8 } // Bottom right
		};
		Texture *temp_texture = engine.get_texture_manager()->load_texture(icon, true);
		if (temp_texture != nullptr)
			temp_texture->set_color(color);

		hotkey = code;
		hotkey_name = " ";

		if (hotkey != NULL)
			hotkey_name = SDL_GetKeyName(hotkey);

		for (uint8_t i = 0; i < 2; i++)
		{
			if (i == 1)
			{
				for (uint8_t j = 0; j < 4; j++)
					corners[j].x += 64;
				center.x += 64;
			}
			ui.get_background()->render(i * 48 + 16, 16, &center);

			ui.get_background()->render(i * 48, 0, &corners[0]);
			ui.get_background()->render(i * 48 + 32, 0, &corners[1]);
			ui.get_background()->render(i * 48, 16, &corners[2]);
			ui.get_background()->render(i * 48 + 16, 32, &corners[3]);

			if (temp_texture != nullptr)
				temp_texture->render(i * 48 + 8, 8);
		}
		if (temp_texture != nullptr)
			engine.get_texture_manager()->free_texture(temp_texture->get_name());

		SDL_SetRenderTarget(engine.get_renderer(), NULL);
		return true;
	}
	return false;
}
