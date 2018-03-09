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
#include "actor.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "options.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

#include <cmath> // for std::floor

Overworld::Overworld() :
	base_health(20), anim_timer(0), current_depth(1), actor_manager(nullptr),
	hovered_actor(nullptr), current_level(nullptr), node_highlight(nullptr),
	base_healthbar(nullptr), frames(0), display_fps(0), frame_counter(0)
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
	if (base_healthbar != nullptr)
	{
		engine.get_texture_manager()->free_texture(base_healthbar->get_name());
		base_healthbar = nullptr;
	}
	for (Texture *t : pointers)
		engine.get_texture_manager()->free_texture(t->get_name());
	pointers.clear();
}
void Overworld::init()
{
	ui.init_background();
	ui.init_message_log();

	actor_manager = new ActorManager;
	actor_manager->init();

	current_level = new Level;
	current_level->create(actor_manager, current_depth);

	if (engine.get_sound_manager() != nullptr)
	{
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Beach_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Beach_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Boss_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Boss_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Castle_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Castle_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Castle_03.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Cave_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Cave_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Fanfare_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Fanfare_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Fanfare_03.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Night_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Night_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Night_03.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Night_04.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Plains_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Plains_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Plains_03.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Snow_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Snow_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Swamp_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "core/sound/music/Swamp_02.mid");
		engine.get_sound_manager()->set_playlist(PT_BOSS);
	}
	node_highlight = engine.get_texture_manager()->load_texture("core/texture/ui/highlight.png", true);
	if (node_highlight != nullptr)
		node_highlight->set_color(COLOR_BERRY);

	base_healthbar = engine.get_texture_manager()->load_texture("core/texture/ui/health_boss.png");

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
				case SDLK_g:
					if (current_level != nullptr)
					{
						current_depth += 1;
						current_level->create(actor_manager, current_depth);
					}
					break;
				case SDLK_c:
					if (engine.get_sound_manager() != nullptr)
					{
						if (!engine.get_sound_manager()->get_paused())
							engine.get_sound_manager()->skip_song();
					}
					break;
				case SDLK_x:
					if (engine.get_sound_manager() != nullptr)
					{
						if (engine.get_sound_manager()->get_paused())
							engine.get_sound_manager()->resume_music(true);
						else engine.get_sound_manager()->pause_music(true);
					}
					break;
				default:
					if (actor_manager != nullptr && ui.get_level_up_box() == nullptr)
						actor_manager->input_keyboard_down(event.key.keysym.sym, current_level);
					break;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (!ui.get_click(actor_manager, event.button.x, event.button.y))
			{
				if (actor_manager != nullptr)
					actor_manager->input_mouse_button_down(event, current_level);
			}
		}
	}
	SDL_GetMouseState(&mouse_x, &mouse_y);
	if (current_level != nullptr)
	{
		const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
		const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

		if (!ui.get_overlap(actor_manager, mouse_x, mouse_y) && map_x >= 0 && map_y >= 0 &&
			map_x < current_level->get_map_width() && map_y < current_level->get_map_height())
		{
			Actor *temp_actor = current_level->get_actor(map_x, map_y);
			if (temp_actor != hovered_actor)
			{
				if (hovered_actor != nullptr && hovered_actor->get_hovered() != HOVER_UI)
					hovered_actor->set_hovered(HOVER_NONE);

				hovered_actor = temp_actor;

				if (hovered_actor != nullptr)
					hovered_actor->set_hovered(HOVER_MAP);
			}
		}
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
	if (actor_manager != nullptr/* && ui.get_level_up_box() == nullptr*/)
	{
		if (actor_manager->update(current_level))
			hovered_actor = nullptr;
		if (actor_manager->get_next_turn())
			next_turn();
	}
	ui.update();
	camera.update();
	engine.get_sound_manager()->update();

	if (current_level != nullptr && current_level->get_damage_base())
	{
		current_level->set_damage_base(false);
		if (base_health > 0)
		{
			base_health -= 1;
			if (base_health == 0)
			{
				const std::string defeats[1] = {
					"Better luck next time...",
				};
				ui.spawn_message_box("Defeat", defeats[engine.get_rng() % 1], true);
				if (actor_manager != nullptr)
					actor_manager->clear_heroes(current_level);
			}
		}
	}
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
	int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
	int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

	if (node_highlight != nullptr && current_level != nullptr)
	{
		if (std::floor(mouse_x + camera.get_cam_x()) < 0) map_x = -1;
		if (std::floor(mouse_y + camera.get_cam_y()) < 0) map_y = -1;

		if (map_x >= 0 && map_x < current_level->get_map_width() &&
			map_y >= 0 && map_y < current_level->get_map_height())
		{
			node_highlight->render(map_x * 32 - camera.get_cam_x(), map_y * 32 - camera.get_cam_y());
		}
	}
	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);

	if (actor_manager != nullptr)
		actor_manager->render_ui();

	if (current_level != nullptr)
		current_level->render_ui();

	if (base_healthbar != nullptr)
	{
		SDL_Rect background = { 0, 0, 16, 16 };
		SDL_Rect foreground = { 0, 16, 16, 16 };

		int8_t hearts = 5;
		int8_t hp_left = base_health;
		uint16_t render_x = (camera.get_cam_w() / 2) - 80;
		uint16_t render_y = 16;

		while (hearts > 0)
		{
			if (hearts == 5) background.x = 0;
			else if (hearts == 1) background.x = 32;
			else background.x = 16;

			if (hp_left == 3) foreground.x = 16;
			else if (hp_left == 2) foreground.x = 32;
			else if (hp_left == 1) foreground.x = 48;

			base_healthbar->render(render_x, render_y, &background);
			if (hp_left > 0)
				base_healthbar->render(render_x, render_y, &foreground);

			hearts -= 1;
			hp_left -= 4;
			render_x += 32;
		}
	}
	ui.render();

	//ui.get_bitmap_font()->render_text(16, 16, "FPS: " + std::to_string(display_fps));
	//ui.get_bitmap_font()->render_text(16, 27, "Turn: " + std::to_string(current_turn));

	if (pointers.size() > 1)
	{
		Actor *temp_actor = nullptr;
		if (current_level != nullptr)
			temp_actor = current_level->get_actor(map_x, map_y);

		if (temp_actor != nullptr && temp_actor->get_actor_type() == ACTOR_MONSTER)
			pointers[1]->render(mouse_x, mouse_y);
		else pointers[0]->render(mouse_x, mouse_y);
	}
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
	if (current_level != nullptr)
		current_level->next_turn(actor_manager);
}
