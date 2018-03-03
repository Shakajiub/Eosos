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
#include "dungeon.hpp"
#include "actor.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "monster.hpp"
#include "mount.hpp"
#include "player.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "item.hpp"
#include "particle_manager.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "health_indicator.hpp"
#include "ui.hpp"

#include <fstream>

uint16_t distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	return (uint16_t)SDL_sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}
Dungeon::Dungeon() :
	current_actor(nullptr), current_level(nullptr), node_highlight(nullptr),
	frames(0), display_fps(0), frame_counter(0), anim_timer(0), regen_level(false)
{

}
Dungeon::~Dungeon()
{
	free();
}
void Dungeon::free()
{
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

	current_actor = nullptr;
}
void Dungeon::init()
{
	current_level = new Level;
	if (options.get_s("debug-generator") != "none")
		current_level->generate_map(options.get_s("debug-generator"));
	else current_level->load_map(options.get_s("debug-level"));

	node_highlight = engine.get_texture_manager()->load_texture("core/texture/ui/highlight.png", true);
	if (node_highlight != nullptr)
		node_highlight->set_color(COLOR_BERRY);

	const std::string pntrs[2] = { "pointer", "pointer_sword" };
	for (auto pntr : pntrs)
	{
		Texture *temp_pntr = engine.get_texture_manager()->load_texture("core/texture/ui/" + pntr + ".png");
		if (temp_pntr != nullptr)
			pointers.push_back(temp_pntr);
	}
	current_actor = player;
	current_actor->start_turn();

	ui.init_message_log();
	//ui.init_experience_bar();
	ui.init_healthbar(HB_PLAYER);
	ui.init_healthbar(HB_OTHER);
	ui.init_inventory(INV_TEMP);
	ui.init_inventory(INV_EQUIP);
	ui.init_inventory(INV_BACKPACK);

	player->init_pathfinder(this);
	player->init_ability_manager(this);
	player->init_inventory();
	//player->init_mount();

	engine.get_sound_manager()->add_to_playlist(PT_AMBIANCE, "core/sound/FantasyMusica/Cave_01.ogg");
	engine.get_sound_manager()->add_to_playlist(PT_AMBIANCE, "core/sound/FantasyMusica/Cave_02.ogg");

	engine.get_sound_manager()->add_to_playlist(PT_COMBAT, "core/sound/FantasyMusica/Battle_01.ogg");
	engine.get_sound_manager()->add_to_playlist(PT_COMBAT, "core/sound/FantasyMusica/Battle_02.ogg");
	engine.get_sound_manager()->add_to_playlist(PT_COMBAT, "core/sound/FantasyMusica/Battle_03.ogg");
	engine.get_sound_manager()->add_to_playlist(PT_COMBAT, "core/sound/FantasyMusica/Battle_04.ogg");
}
bool Dungeon::update()
{
	frames += 1;
	frame_counter += engine.get_dt();

	if (frame_counter > 1000)
	{
		display_fps = frames;
		frames = 0;
		frame_counter = 0;
	}
	if (regen_level)
	{
		if (current_level->get_map_loaded())
		{
			current_level->free();
			current_level->generate_map(options.get_s("debug-generator"));
			current_actor = player;
		}
		regen_level = false;
		return true;
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
			Item *test = nullptr;
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					if (!player->input_keyboard_down(event.key.keysym.sym))
						return false;
					break;
				case SDLK_i:
					test = new Item;
					test->init("core/script/data/item/test_items.lua", "random");
					ui.set_temp_item(test);
					break;
				case SDLK_TAB: ui.toggle_inventory(); break;
				case SDLK_d: if (SDL_GetModState() & KMOD_CTRL) options.load(); break;
				case SDLK_g: regen_level = true; break;
				case SDLK_c: engine.get_sound_manager()->skip_song(); break;
				default: player->input_keyboard_down(event.key.keysym.sym); break;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			player->input_keyboard_up(event.key.keysym.sym);
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (!ui.get_click(event.button.x, event.button.y))
			{
				if (current_actor != player)
					std::cout << "can only interact with the UI on the player's turn!" << std::endl;
				else if (player->get_action() != nullptr)
					std::cout << "can't interact with the UI during an action!" << std::endl;
				else player->input_mouse_button_down(this, event);
			}
		}
		/*else if (event.type == SDL_JOYHATMOTION)
		{
			std::cout << "joyhatmotion! (" << event.jhat.hat << ", " << event.jhat.value << ")" << std::endl;
			player->input_controller_down(SDL_JOYHATMOTION, event.jhat.hat, event.jhat.value);
		}
		else if (event.type == SDL_JOYAXISMOTION)
		{
			//std::cout << "joyaxismotion! (" << event.jaxis.axis << ", " << event.jaxis.value << ")" << std::endl;
		}*/
		else if (event.type == SDL_JOYBUTTONDOWN)
		{
			std::cout << "joybuttondown! (" << (int)event.jbutton.which << ", " << (int)event.jbutton.button << ", " << (int)event.jbutton.state << ")" << std::endl;

			switch (event.jbutton.button)
			{
				case 15: ui.toggle_inventory(); break;
				default:
					if (!ui.input_controller(event.jbutton.button, event.jbutton.state))
						player->input_controller_down(event.jbutton.button, event.jbutton.state);
					break;
			}
		}
		else if (event.type == SDL_JOYBUTTONUP)
			player->input_controller_up();
	}
	// As long as the current actor is done with their turn, pass it on to the next one
	while (current_actor->take_turn(this))
	{
		//if (current_actor == player)
			//camera.update_position(player->get_grid_x() * 32, player->get_grid_y() * 32);

		const uint16_t current_ID = current_actor->get_ID();
		Actor *temp_actor = nullptr;

		for (Monster *m : current_level->get_monsters())
		{
			if (!m->get_state_var("awakened"))
			{
				// Monsters are not automatically activated on map load, they need to get
				// close enough to the player to actually be considered in the turn cycle

				if (distance(m->get_grid_x(), m->get_grid_y(), player->get_grid_x(), player->get_grid_y()) < 10)
					m->set_state_var("awakened", true);
			}
			else if (m->get_ID() > current_ID && (temp_actor == nullptr || m->get_ID() < temp_actor->get_ID()))
				temp_actor = m;
		}
		if (temp_actor == nullptr) // We can consider this the place where a "new" turn-cycle starts
		{
			current_actor = player;

			if (current_level != nullptr) // Objects get updated once per turn
				current_level->update_objects();
		}
		else current_actor = temp_actor;
		current_actor->start_turn();
	}
	player->update(this);
	for (Monster *m : current_level->get_monsters())
		m->update(this);

	engine.get_sound_manager()->update();
	engine.get_particle_manager()->update();

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
		player->action_idle();
		for (Monster *m : current_level->get_monsters())
			m->action_idle();
	}
	ui.update();
	camera.update();

	return true;
}
void Dungeon::render() const
{
	SDL_RenderClear(engine.get_renderer());

	if (current_level != nullptr)
	{
		current_level->render();

		Actor *temp_actor = nullptr;
		for (uint8_t y = 0; y < current_level->get_map_height(); y++)
		{
			for (uint8_t x = 0; x < current_level->get_map_width(); x++)
			{
				temp_actor = current_level->get_actor(x, y);
				if (!current_level->get_node_discovered(x, y))
				{
					if (temp_actor != nullptr && current_level->get_node_discovered(x, y - 1))
						temp_actor->render_bubble();
				}
				else if (temp_actor != nullptr)
				{
					temp_actor->render();
					temp_actor->render_bubble();
				}
			}
		}
	}
	int mouse_x, mouse_y;
	if (options.get_b("controller-enabled"))
	{
		mouse_x = engine.get_mouse_x();
		mouse_y = engine.get_mouse_y();
	}
	else SDL_GetMouseState(&mouse_x, &mouse_y);

	const int8_t map_x = (mouse_x + camera.get_cam_x()) / 32;
	const int8_t map_y = (mouse_y + camera.get_cam_y()) / 32;

	if (node_highlight != nullptr)
	{
		if (!ui.get_overlap(mouse_x, mouse_y) && current_level->get_node_discovered(map_x, map_y))
		{
			node_highlight->render(
				(map_x < 0 ? map_x - 1 : map_x) * 32 - camera.get_cam_x(),
				(map_y < 0 ? map_y - 1 : map_y) * 32 - camera.get_cam_y()
			);
		}
	}
	engine.get_particle_manager()->render();

	ui.render(mouse_x, mouse_y);
	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
	ui.get_bitmap_font()->render_text(16, 176, "FPS: " + std::to_string(display_fps));
	ui.get_bitmap_font()->render_text(16, 176 + ui.get_bitmap_font()->get_height(),
		"Moves: " + std::to_string(player->get_moves().first) + "/" + std::to_string(player->get_moves().second)
	);
	if (pointers.size() > 0)
	{
		Actor *temp_actor = current_level->get_actor(map_x, map_y);
		if (temp_actor != nullptr && temp_actor != player && current_level->get_node_discovered(map_x, map_y))
		{
			if (ui.get_healthbar(HB_OTHER)->get_target() != temp_actor)
				ui.set_healthbar_target(HB_OTHER, temp_actor);
			pointers[1]->render(mouse_x, mouse_y);
		}
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
