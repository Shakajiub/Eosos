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
#include "menu.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Menu::Menu() : pointer(nullptr)
{

}
Menu::~Menu()
{
	free();
}
void Menu::free()
{
	if (pointer != nullptr)
	{
		engine.get_texture_manager()->free_texture(pointer->get_name());
		pointer = nullptr;
	}
}
void Menu::init()
{
	free();

	engine.get_sound_manager()->clear_playlist(PT_MENU);
	engine.get_sound_manager()->add_to_playlist(PT_MENU, "music/Menu_01.mid");
	engine.get_sound_manager()->add_to_playlist(PT_MENU, "music/Menu_02.mid");
	engine.get_sound_manager()->add_to_playlist(PT_MENU, "music/Menu_03.mid");

	pointer = engine.get_texture_manager()->load_texture("ui/pointer.png");
}
bool Menu::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			return false;

		else if (event.type == SDL_WINDOWEVENT)
		{
			if (engine.handle_window_event(event.window.event))
				return true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					return false;

				case SDLK_RETURN: case SDLK_RETURN2: case SDLK_KP_ENTER:
					engine.get_scene_manager()->set_scene("scenario");
					break;

				default:
					engine.handle_keyboard_input(event.key.keysym.sym);
					break;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			engine.get_scene_manager()->set_scene("scenario");
		}
	}
	return true;
}
void Menu::render() const
{
	SDL_RenderClear(engine.get_renderer());

	ui.get_bitmap_font()->set_color(DAWN_PEPPERMINT);
	ui.get_bitmap_font()->set_scale(3);
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - 144, 64, "Eosos - 7DRL");
	ui.get_bitmap_font()->set_scale(2);
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - 176, 112, "Press %A[Enter]%F to start");
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - 136, 134, "%A[Escape]%F to quit");
	ui.get_bitmap_font()->set_scale(1);

	ui.get_bitmap_font()->render_text(16, camera.get_cam_h() - 38, "DragonDePlatino");
	ui.get_bitmap_font()->render_text(16, camera.get_cam_h() - 27, "     Quale");
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - 104, camera.get_cam_h() - 27, "Jere Oikarinen (Shakajiub)");
	ui.get_bitmap_font()->render_text(camera.get_cam_w() - 160, camera.get_cam_h() - 38, "    Beau Buckley");
	ui.get_bitmap_font()->render_text(camera.get_cam_w() - 160, camera.get_cam_h() - 27, " fantasymusica.org");

	ui.get_bitmap_font()->set_color(DAWN_SLATE);
	ui.get_bitmap_font()->render_text(16, camera.get_cam_h() - 60, "      Art");
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - 80, camera.get_cam_h() - 38, "Design & Programming");
	ui.get_bitmap_font()->render_text(camera.get_cam_w() - 160, camera.get_cam_h() - 60, "       Music");

	if (pointer != nullptr)
	{
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		pointer->render(mouse_x, mouse_y);
	}
	SDL_RenderPresent(engine.get_renderer());
}
