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
#include "scenario.hpp"
#include "actor.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "actor_manager.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <cmath> // for std::floor

Scenario::Scenario() :
	state(GAME_IN_PROGRESS), base_health(20), anim_timer(0), current_depth(1), hovered_actor(nullptr),
	current_level(nullptr), node_highlight(nullptr), base_healthbar(nullptr), frames(0), display_fps(0),
	frame_counter(0)
{

}
Scenario::~Scenario()
{
	free();
}
void Scenario::free()
{
	engine.get_actor_manager()->free();

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
	hovered_actor = nullptr;

	ui.free();
}
void Scenario::init()
{
	ui.init_bitmap_font();
	ui.init_background();
	ui.init_message_log();

	state = GAME_IN_PROGRESS;
	current_depth = 1;
	base_health = 20;

	engine.get_actor_manager()->init();
	camera.update_position(-320, -160, true);

	current_level = new Level;
	current_level->create(current_depth);

	if (engine.get_sound_manager() != nullptr)
	{
		engine.get_sound_manager()->clear_playlist(PT_KOBOLD);
		engine.get_sound_manager()->add_to_playlist(PT_KOBOLD, "music/Kobold_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_KOBOLD, "music/Kobold_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_KOBOLD, "music/Kobold_03.mid");

		engine.get_sound_manager()->clear_playlist(PT_DWARF);
		engine.get_sound_manager()->add_to_playlist(PT_DWARF, "music/Dwarf_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DWARF, "music/Dwarf_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DWARF, "music/Dwarf_03.mid");

		engine.get_sound_manager()->clear_playlist(PT_DEMON);
		engine.get_sound_manager()->add_to_playlist(PT_DEMON, "music/Demon_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DEMON, "music/Demon_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DEMON, "music/Demon_03.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DEMON, "music/Demon_04.mid");

		engine.get_sound_manager()->clear_playlist(PT_BOSS);
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "music/Boss_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "music/Boss_02.mid");
		engine.get_sound_manager()->add_to_playlist(PT_BOSS, "music/Boss_03.mid");

		engine.get_sound_manager()->clear_playlist(PT_DEFEAT);
		engine.get_sound_manager()->add_to_playlist(PT_DEFEAT, "music/Boss_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_DEFEAT, "music/Boss_02.mid");

		engine.get_sound_manager()->clear_playlist(PT_VICTORY);
		engine.get_sound_manager()->add_to_playlist(PT_VICTORY, "music/Fanfare_01.mid");
		engine.get_sound_manager()->add_to_playlist(PT_VICTORY, "music/Fanfare_02.mid");

		engine.get_sound_manager()->set_playlist(PT_KOBOLD);
	}
	node_highlight = engine.get_texture_manager()->load_texture("ui/highlight.png", true);
	if (node_highlight != nullptr)
		node_highlight->set_color(DAWN_BERRY);

	base_healthbar = engine.get_texture_manager()->load_texture("ui/health_boss.png");

	const std::string pntrs[2] = { "pointer", "pointer_sword" };
	for (auto pntr : pntrs)
	{
		Texture *temp_pntr = engine.get_texture_manager()->load_texture("ui/" + pntr + ".png");
		if (temp_pntr != nullptr)
			pointers.push_back(temp_pntr);
	}
	ui.clear_message_box(true);
	ui.spawn_message_box("Level #1", "");
}
bool Scenario::update()
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
			if (engine.handle_window_event(event.window.event))
				return true;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					engine.get_scene_manager()->set_scene("menu");
					return true;

				case SDLK_RETURN: case SDLK_RETURN2: case SDLK_KP_ENTER:
					if (state == GAME_BOSS_WON || state == GAME_END)
					{
						current_depth += 1;
						current_level->create(current_depth);
						ui.clear_message_box(true);
						if (state == GAME_END)
						{
							ui.spawn_message_box("You may continue", "But the enemies won't get stronger");
							ui.get_message_log()->add_message("Maybe in a later version?");
						}
						else ui.spawn_message_box("Level #" + std::to_string(current_depth), "");
						state = GAME_IN_PROGRESS;
						return true;
					}
					break;
				default:
					if (engine.handle_keyboard_input(event.key.keysym.sym))
						break;
					if (ui.get_level_up_box() == nullptr)
						engine.get_actor_manager()->input_keyboard_down(event.key.keysym.sym, current_level);
					break;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (!ui.get_click(event.button.x, event.button.y))
				engine.get_actor_manager()->input_mouse_button_down(event, current_level);
		}
	}
	//SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
	SDL_GetMouseState(&mouse_x, &mouse_y);

	if (current_level != nullptr)
	{
		const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
		const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

		if (!ui.get_overlap(mouse_x, mouse_y) && map_x >= 0 && map_y >= 0 &&
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
		engine.get_actor_manager()->animate();
	}
	if (engine.get_actor_manager()->update(current_level))
		hovered_actor = nullptr;
	if (engine.get_actor_manager()->get_next_turn())
		next_turn();

	if (current_level != nullptr)
	{
		if (current_level->get_damage_base() > 0)
		{
			if (base_health > 0)
			{
				if (base_health - current_level->get_damage_base() >= 0)
					base_health -= 1;
				else base_health = 0;

				if (base_health == 0)
				{
					const std::string defeats[1] = {
						"Press %A[Escape]%F to continue",
					};
					ui.spawn_message_box("Defeat", defeats[engine.get_rng() % 1], true);
					engine.get_actor_manager()->clear_heroes(current_level);
					state = GAME_OVER;
				}
			}
			current_level->set_damage_base(0);
		}
		else if (current_level->get_victory())
		{
			current_level->set_victory(false);
			if (state != GAME_OVER)
			{
				if (current_depth == 4)
				{
					state = GAME_END;
					ui.spawn_message_box("Victory!", "By defeating %APlatino%F you have won the game!");
					ui.get_message_log()->add_message("Victory! Press %A[Escape]%F to continue");
					engine.get_sound_manager()->set_playlist(PT_VICTORY);
				}
				else
				{
					state = GAME_BOSS_WON;
					ui.spawn_message_box("Boss defeated!", "Press %A[Enter]%F to continue");
					engine.get_sound_manager()->set_playlist(PT_VICTORY);
				}
			}
		}
	}
	ui.update();

	camera.update();
	engine.get_sound_manager()->update();
	return true;
}
void Scenario::render() const
{
	SDL_RenderClear(engine.get_renderer());

	if (current_level != nullptr)
	{
		current_level->render();
		engine.get_actor_manager()->render(current_level);
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
	ui.get_bitmap_font()->set_color(DAWN_PEPPERMINT);
	engine.get_actor_manager()->render_ui();

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
void Scenario::next_turn()
{
	if (current_level != nullptr)
		current_level->next_turn();
}
