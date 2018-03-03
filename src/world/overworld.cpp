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
#include "overworld.hpp"
#include "texture.hpp"

#include "options.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Overworld::Overworld()
{

}
Overworld::~Overworld()
{
	free();
}
void Overworld::free()
{
	for (Texture *t : pointers)
		engine.get_texture_manager()->free_texture(t->get_name());
	pointers.clear();
}
void Overworld::init()
{
	const std::string pntrs[2] = { "pointer", "pointer_sword" };
	for (auto pntr : pntrs)
	{
		Texture *temp_pntr = engine.get_texture_manager()->load_texture("core/texture/ui/" + pntr + ".png");
		if (temp_pntr != nullptr)
			pointers.push_back(temp_pntr);
	}
}
bool Overworld::update()
{
	frames += 1;
	frame_counter += engine.get_dt();

	if (frame_counter > 1000)
	{
		display_fps = frames;
		frames = 0;
		frame_counter = 0;
	}
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			return false;
		else if (event.type == SDL_WINDOWEVENT)
		{
			bool stop_loop = false;
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_MINIMIZED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					engine.get_scene_manager()->set_window_focus(false);
					engine.get_sound_manager()->pause_music();
					stop_loop = true;
					break;
				default: break;
			}
			if (stop_loop)
				return true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					return false;
				case SDLK_d:
					if (SDL_GetModState() & KMOD_CTRL)
						options.load();
					break;
				default: break;
			}
		}
	}
	ui.update();
	return true;
}
void Overworld::render() const
{
	SDL_RenderClear(engine.get_renderer());

	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);

	ui.render();
	ui.get_bitmap_font()->render_text(16, 16, "FPS: " + std::to_string(display_fps));

	if (pointers.size() > 0)
		pointers[0]->render(mouse_x, mouse_y);

	SDL_RenderPresent(engine.get_renderer());
}
