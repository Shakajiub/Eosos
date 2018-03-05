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
#include "actor_manager.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "options.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

Overworld::Overworld() :
	anim_timer(0), current_depth(0), current_turn(0), actor_manager(nullptr),
	current_level(nullptr), node_highlight(nullptr), frames(0), display_fps(0), frame_counter(0)
{

}
Overworld::~Overworld()
{
	free();
}
void Overworld::free()
{
	if (actor_manager != nullptr)
	{
		delete actor_manager;
		actor_manager = nullptr;
	}
	if (current_level != nullptr)
	{
		delete current_level;
		current_level = nullptr;
	}
	if (node_highlight != nullptr)
	{
		engine.get_texture_manager()->free_texture(node_highlight->get_name());
		node_highlight = nullptr;
	}
	for (Texture *t : pointers)
		engine.get_texture_manager()->free_texture(t->get_name());

	pointers.clear();
}
void Overworld::init()
{
	ui.init_background();

	current_level = new Level;
	current_level->create(current_depth);

	actor_manager = new ActorManager;
	actor_manager->spawn_actor(current_level, ACTOR_HERO, 10, 7, "core/texture/actor/player/orc/peon.png");
	//actor_manager->spawn_actor(current_level, ACTOR_HERO, 2, 5, "core/texture/actor/player/orc/mage.png");

	node_highlight = engine.get_texture_manager()->load_texture("core/texture/ui/highlight.png", true);
	if (node_highlight != nullptr)
		node_highlight->set_color(COLOR_BERRY);

	const std::string pntrs[1] = { "pointer" };
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
				case SDLK_g:
					if (current_level != nullptr)
					{
						current_turn = 0;
						current_depth += 1;
						current_level->create(current_depth);
						actor_manager->clear_actors();
						actor_manager->spawn_actor(current_level, ACTOR_HERO, 10, 7, "core/texture/actor/player/orc/peon.png");
					}
					break;
				default:
					if (actor_manager != nullptr)
						actor_manager->input_keyboard_down(event.key.keysym.sym, current_level);
					break;
			}
		}
	}
	if (actor_manager != nullptr)
	{
		actor_manager->update(current_level);
		if (actor_manager->get_next_turn())
			next_turn();
	}
	// Idle / map animations
	anim_timer += engine.get_dt();
	while (anim_timer > 100)
	{
		anim_timer -= 100;
		static uint8_t loop; loop += 1;
		if (loop < 4) continue; loop = 0;

		static bool animate_map; animate_map = !animate_map;
		if (animate_map)
		{
			if (current_level != nullptr)
				current_level->animate();
		}
		if (actor_manager != nullptr)
			actor_manager->animate();
	}
	ui.update();
	camera.update();

	return true;
}
void Overworld::render() const
{
	SDL_RenderClear(engine.get_renderer());

	if (current_level != nullptr)
	{
		current_level->render();
		if (actor_manager != nullptr)
			actor_manager->render(current_level);
	}
	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);

	if (node_highlight != nullptr)
	{
		const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
		const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

		if (!ui.get_overlap(mouse_x, mouse_y))
		{
			node_highlight->render(
				(map_x < 0 ? map_x - 1 : map_x) * 32 - camera.get_cam_x(),
				(map_y < 0 ? map_y - 1 : map_y) * 32 - camera.get_cam_y()
			);
		}
	}
	ui.render();
	ui.get_bitmap_font()->render_text(16, 16, "FPS: " + std::to_string(display_fps));
	ui.get_bitmap_font()->render_text(16, 27, "Turn: " + std::to_string(current_turn));

	if (pointers.size() > 0)
		pointers[0]->render(mouse_x, mouse_y);

	if (mouse_y == 0)
		camera.move_camera(1, current_level->get_map_width(), current_level->get_map_height());
	else if (mouse_y >= camera.get_cam_h() - 1)
		camera.move_camera(2, current_level->get_map_width(), current_level->get_map_height());
	if (mouse_x == 0)
		camera.move_camera(4, current_level->get_map_width(), current_level->get_map_height());
	else if (mouse_x >= camera.get_cam_w() - 1)
		camera.move_camera(8, current_level->get_map_width(), current_level->get_map_height());

	SDL_RenderPresent(engine.get_renderer());
}
void Overworld::next_turn()
{
	current_turn += 1;
	if (current_level != nullptr)
		current_level->next_turn(current_turn, actor_manager);
}
