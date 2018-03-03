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
#include "button.hpp"

#include "camera.hpp"
#include "texture_manager.hpp"
#include "texture.hpp"
#include "ui.hpp"

Button::Button() : btn_enabled(true), consider_camera(false), states(0), btn_texture(nullptr)
{
	btn_rect = { 0, 0, 32, 32 };
}
Button::~Button()
{
	free();
}
void Button::free()
{
	if (btn_texture != nullptr)
	{
		SDL_DestroyTexture(btn_texture);
		btn_texture = nullptr;
	}
}
void Button::init(ButtonType type, uint8_t value)
{
	btn_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 64, (type == BTN_WIDGET_TOGGLE) ? 64 : 32
	);
	if (btn_texture == NULL)
	{
		btn_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return;
	}
	SDL_SetRenderTarget(engine.get_renderer(), btn_texture);

	if (type == BTN_WIDGET_TOGGLE)
	{
		states = 2; // Two states; maximize & minimize
		SDL_Rect temp_rect = { 48, 48, 16, 16 };

		ui.get_background()->render(0, 0, &temp_rect);
		ui.get_background()->render(0, 32, &temp_rect);
		temp_rect.x += 64;
		ui.get_background()->render(32, 0, &temp_rect);
		ui.get_background()->render(32, 32, &temp_rect);

		Texture *cross = engine.get_texture_manager()->load_texture("core/texture/ui/icon/cross_small.png", true, false);
		if (cross != nullptr)
		{
			cross->set_color(COLOR_PLUM);
			cross->render(0, 0); cross->render(32, 0);
			engine.get_texture_manager()->free_texture(cross->get_name());
		}
		Texture *square = engine.get_texture_manager()->load_texture("core/texture/ui/icon/square.png", true, false);
		if (square != nullptr)
		{
			square->set_color(COLOR_PLUM);
			square->render(0, 32); square->render(32, 32);
			engine.get_texture_manager()->free_texture(square->get_name());
		}
	}
	else if (type == BTN_DIRECTION_KEY)
	{
		states = 1;

		// Fill the texture with "nothing" (full transparency)
		SDL_SetTextureBlendMode(btn_texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(engine.get_renderer(), 0, 0, 0, 0);
		SDL_RenderClear(engine.get_renderer());
		SDL_SetRenderDrawColor(engine.get_renderer(), 20, 12, 28, 255);

		Texture *arrow = nullptr;
		if (value < 4)
			arrow = engine.get_texture_manager()->load_texture("core/texture/ui/icon/arrow_orthogonal.png", true);
		else arrow = engine.get_texture_manager()->load_texture("core/texture/ui/icon/arrow_diagonal.png", true);

		if (arrow != nullptr)
		{
			SDL_RendererFlip flip = SDL_FLIP_NONE;
			double angle = 0.0;

			// Flip and twist the arrow depending on the given value parameter
			switch (value) // 0=up, 1=down, 2=left, 3=right, 4=up+left, 5=up+right, 6=down+left, 7=down+right
			{
				case 1: case 7: flip = SDL_FLIP_VERTICAL; break;
				case 2: flip = SDL_FLIP_VERTICAL; angle = 90.0; break;
				case 3: angle = 90.0; break;
				case 4: flip = SDL_FLIP_HORIZONTAL; break;
				case 6: flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL); break;
				default: break;
			}
			arrow->set_color(COLOR_PLUM); arrow->render(0, 0, nullptr, 2, flip, angle);
			arrow->set_color(COLOR_BERRY); arrow->render(32, 0, nullptr, 2, flip, angle);

			engine.get_texture_manager()->free_texture(arrow->get_name());
		}
	}
	else std::cout << "trying to initialize unknown button type!" << std::endl;

	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
void Button::render() const
{
	if (btn_texture != nullptr)
	{
		const SDL_Rect quad =
		{
			x - (consider_camera ? camera.get_cam_x() : 0),
			y - (consider_camera ? camera.get_cam_y() : 0), 32, 32
		};
		SDL_RenderCopyEx(engine.get_renderer(), btn_texture, &btn_rect, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
}
bool Button::get_overlap(int16_t xpos, int16_t ypos)
{
	const int16_t check_x = x - (consider_camera ? camera.get_cam_x() : 0);
	const int16_t check_y = y - (consider_camera ? camera.get_cam_y() : 0);

	if (xpos > check_x && xpos < (check_x + 32) && ypos > check_y && ypos < (check_y + 32))
		btn_rect.x = 32;
	else btn_rect.x = 0;

	return btn_rect.x != 0;
}
void Button::set_state(uint8_t state)
{
	if (state + 1 > states)
	{
		state = 0; // Fall down to default state
		std::cout << "attempting to set unknown button state!" << std::endl;
	}
	btn_rect.y = state * 32;
}
