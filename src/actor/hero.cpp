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
#include "hero.hpp"
#include "level.hpp"
#include "astar.hpp"
#include "texture.hpp"

#include "mount.hpp"
#include "camera.hpp"
#include "texture_manager.hpp"
#include "bitmap_font.hpp"
#include "message_log.hpp"
#include "ui.hpp"

Hero::Hero() :
	auto_move_path(false), command_this_turn(false), hero_class(HC_PEON), max_moves(2), hp_shake(0), hb_timer(100),
	pathfinder(nullptr), ui_texture(nullptr), health_texture(nullptr), sleep_timer(0), ability_activated(false)
{
	health = std::make_pair(3, 3);
	prev_health = health.first;
	name = "Peon";
}
Hero::~Hero()
{
	free();
}
void Hero::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
	if (ui_texture != nullptr)
	{
		SDL_DestroyTexture(ui_texture);
		ui_texture = nullptr;
	}
	if (health_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(health_texture->get_name());
		health_texture = nullptr;
	}
	abilities.clear();
}
bool Hero::init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	if (!Actor::init(at, xpos, ypos, texture_name))
		return false;

	return (init_ui_texture() && init_pathfinder());
}
void Hero::update(Level *level)
{
	Actor::update(level);

	if (hp_shake > 0)
		hp_shake -= 1;

	if (prev_health != health.first)
	{
		prev_health = health.first;
		hp_shake = 10;
	}
	if (health.first < health.second)
	{
		hb_timer += engine.get_dt();
		if (hb_timer > health.first * 250)
			hb_timer = 0;
	}
	else hb_timer = 100;
}
void Hero::render_ui(uint16_t xpos, uint16_t ypos) const
{
	Actor::render_ui(xpos, ypos);

	if (pathfinder != nullptr && (hovered != HOVER_NONE))
		pathfinder->render(moves.first);

	if (ui_texture != nullptr)
	{
		const SDL_Rect rect = { (hovered != HOVER_NONE) ? 48 : 0, 0, 48, 48 };
		const SDL_Rect quad = { xpos, ypos, 48, 48 };

		SDL_RenderCopyEx(engine.get_renderer(), ui_texture, &rect, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
	if (health_texture != nullptr && health.second > 0)
	{
		SDL_Rect rect = { (hb_timer < 100) ? 64 : 0, 0, 16, 16 };

		int8_t hearts = health.second / 3;
		int8_t hp_left = health.first;
		uint16_t render_x = xpos + 50;
		uint16_t render_y = ypos + 8;

		if (status == STATUS_POISON)
			rect.y = 16;
		else if (status == STATUS_WITHER)
			rect.y = 48;

		while (hearts > 0)
		{
			if (hp_left == 2) rect.x = (hb_timer < 100) ? 80 : 16;
			else if (hp_left == 1) rect.x = (hb_timer < 100) ? 96 : 32;
			else if (hp_left < 1) rect.x = 48;

			if (hp_shake > 0)
				health_texture->render(render_x, render_y + (engine.get_rng() % hp_shake), &rect);
			else health_texture->render(render_x, render_y, &rect);

			hearts -= 1;
			hp_left -= 3;
			render_x += 32;
		}
	}
	if (moves.first > 0)
	{
		ui.get_bitmap_font()->render_text(camera.get_cam_w() - 96, 16,
			"Moves: " + std::to_string(moves.first) + "/" + std::to_string(moves.second)
		);
		const std::string xp = "XP: " + std::to_string(experience) + "/" + std::to_string(combat_level * 10);
		ui.get_bitmap_font()->render_text(camera.get_cam_w() - 16 - (xp.length() * 8), 27, xp);
	}
}
void Hero::start_turn()
{
	if (sleep_timer == 0)
	{
		turn_done = false;

		const uint8_t temp_moves = (mount != nullptr) ? max_moves + 1 : max_moves;
		moves = std::make_pair(temp_moves, temp_moves);

		if (!get_auto_move())
			camera.update_position(grid_x * 32, grid_y * 32);
	}
}
bool Hero::take_turn(Level *level)
{
	if (sleep_timer > 0)
	{
		sleep_timer -= 1;
		return true;
	}
	hovered = HOVER_MAP;
	if (turn_done)
	{
		if (pathfinder != nullptr && pathfinder->get_path_found())
		{
			if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
				pathfinder->step();
			else pathfinder->clear_path();
		}
		command_this_turn = false;
		if (moves.first > 0)
			turn_done = false;
	}
	if (actions_empty() && moves.first > 0)
	{
		if (auto_move_path && pathfinder != nullptr)
		{
			step_pathfinder(level);
			if (!pathfinder->get_path_found())
				auto_move_path = false;
		}
	}
	return Actor::take_turn(level);
}
void Hero::end_turn()
{
	Actor::end_turn();
	hovered = HOVER_NONE;
}
uint8_t Hero::get_damage() const
{
	if (hero_class == HC_JUGGERNAUT)
		return current_action.action_value;

	return 1;
}
bool Hero::init_ui_texture()
{
	clear_ui_texture();

	health_texture = engine.get_texture_manager()->load_texture("core/texture/ui/health_hearts.png");
	if (health_texture == nullptr)
		return false;

	ui_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 96, 48
	);
	if (ui_texture == NULL)
	{
		ui_texture = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	if (ui.get_background() != nullptr)
	{
		SDL_SetRenderTarget(engine.get_renderer(), ui_texture);

		SDL_Rect center = { 20, 20, 8, 8 };
		SDL_Rect corners[4] =
		{
			{ 0, 0, 16, 8 }, // Top left
			{ 40, 0, 8, 16 }, // Top right
			{ 0, 32, 8, 16 }, // Bottom left
			{ 32, 40, 16, 8 } // Bottom right
		};
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

			if (texture != nullptr)
				texture->render(i * 48 + 8, 8, &frame_rect, 2, SDL_FLIP_HORIZONTAL);
		}
		SDL_SetRenderTarget(engine.get_renderer(), NULL);
		return true;
	}
	return false;
}
bool Hero::init_pathfinder()
{
	pathfinder = new AStar;
	return pathfinder->init();
}
bool Hero::init_class(HeroClass hc)
{
	const uint8_t hp = health.second;
	std::string class_texture = "core/texture/actor/orc_peon.png";
	hero_class = hc;

	switch (hero_class)
	{
		case HC_MAGE:
			name = "Mage";
			class_texture = "core/texture/actor/orc_mage.png";
			break;
		case HC_NINJA:
			name = "Ninja";
			class_texture = "core/texture/actor/orc_ninja.png";
			proj_type = PROJECTILE_SHURIKEN;
			add_ability("shoot");
			break;
		case HC_BARBARIAN:
			name = "Barbarian";
			class_texture = "core/texture/actor/orc_barbarian.png";
			health = std::make_pair(hp + 3, hp + 3);
			break;
		case HC_JUGGERNAUT:
			name = "Juggernaut";
			class_texture = "core/texture/actor/orc_juggernaut.png";
			break;
		default: hero_class = HC_PEON; break;
	}
	if (texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(texture->get_name());
		texture = nullptr;
	}
	texture = engine.get_texture_manager()->load_texture(class_texture);

	if (texture != nullptr)
		return init_ui_texture();
	else return false;
}
void Hero::step_pathfinder(Level *level)
{
	Actor *temp_actor = level->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	if (temp_actor != nullptr)
	{
		if (temp_actor->get_actor_type() == ACTOR_MONSTER || temp_actor->get_actor_type() == ACTOR_PROP)
		{
			add_action(ACTION_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y(), moves.first);
			moves.first = 0;
		}
		else if (temp_actor->get_actor_type() == ACTOR_MOUNT)
		{
			if (mount == nullptr)
			{
				add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
				set_mount(dynamic_cast<Mount*>(temp_actor));
				add_ability("dismount");
				moves.first = 0;
			}
			else if (ui.get_message_log() != nullptr)
				ui.get_message_log()->add_message(name + ": \"I already have a mount!\"", COLOR_LEAF);
		}
		else
		{
			if (temp_actor != this)
			{
				load_bubble("failure", 1);
				if (ui.get_message_log() != nullptr)
					ui.get_message_log()->add_message(name + ": \"My path is blocked!\"", COLOR_LEAF);
			}
			camera.update_position(grid_x * 32, grid_y * 32);
		}
		pathfinder->clear_path();
	}
	else
	{
		moves.first -= 1;
		add_action(ACTION_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());

		if (command_this_turn && !get_auto_move())
			camera.update_position(pathfinder->get_goto_x() * 32, pathfinder->get_goto_y() * 32);
	}
}
void Hero::clear_pathfinder()
{
	if (pathfinder->get_path_found())
		pathfinder->clear_path();
}
void Hero::clear_ui_texture()
{
	if (ui_texture != nullptr)
	{
		SDL_DestroyTexture(ui_texture);
		ui_texture = nullptr;
	}
	if (health_texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(health_texture->get_name());
		health_texture = nullptr;
	}
}
void Hero::input_keyboard_down(SDL_Keycode key, Level *level)
{
	if (!actions_empty() || moves.first <= 0 || ability_activated)
		return;

	int8_t offset_x = 0, offset_y = 0;
	switch (key)
	{
		case SDLK_UP: case SDLK_KP_8: case SDLK_k: offset_y = -1; break;
		case SDLK_DOWN: case SDLK_KP_2: case SDLK_j: offset_y = 1; break;
		case SDLK_LEFT: case SDLK_KP_4: case SDLK_h: offset_x = -1; break;
		case SDLK_RIGHT: case SDLK_KP_6: case SDLK_l: offset_x = 1; break;
		case SDLK_KP_7: case SDLK_y: offset_x = -1; offset_y = -1; break;
		case SDLK_KP_9: case SDLK_u: offset_x = 1; offset_y = -1; break;
		case SDLK_KP_1: case SDLK_b: offset_x = -1; offset_y = 1; break;
		case SDLK_KP_3: case SDLK_n: offset_x = 1; offset_y = 1; break;
		case SDLK_KP_5: case SDLK_SPACE:

			if (level->get_wall_type(grid_x, grid_y) == NT_BASE && health.first < health.second)
			{
				add_health(1);
				add_action(ACTION_INTERACT, grid_x, grid_y);
			}
			else turn_done = true;
			moves.first = 0;
			break;
		default: break;
	}
	if (offset_x != 0 || offset_y != 0)
	{
		Actor *temp_actor = level->get_actor(grid_x + offset_x, grid_y + offset_y);
		if (temp_actor != nullptr)
		{
			if (temp_actor->get_actor_type() == ACTOR_MONSTER || temp_actor->get_actor_type() == ACTOR_PROP)
			{
				add_action(ACTION_ATTACK, grid_x + offset_x, grid_y + offset_y, moves.first);
				moves.first = 0;
			}
			else if (temp_actor->get_actor_type() == ACTOR_MOUNT)
			{
				if (mount == nullptr)
				{
					add_action(ACTION_MOVE, grid_x + offset_x, grid_y + offset_y);
					set_mount(dynamic_cast<Mount*>(temp_actor));
					add_ability("dismount");
					moves.first = 0;
				}
				else if (ui.get_message_log() != nullptr)
					ui.get_message_log()->add_message(name + ": \"I already have a mount!\"", COLOR_LEAF);
			}
		}
		else if (!level->get_wall(grid_x + offset_x, grid_y + offset_y, true))
		{
			moves.first -= 1;
			add_action(ACTION_MOVE, grid_x + offset_x, grid_y + offset_y);
			camera.update_position((grid_x + offset_x) * 32, (grid_y + offset_y) * 32);
		}
	}
}
void Hero::input_mouse_button_down(SDL_Event eve, Level *level)
{
	if (pathfinder != nullptr && actions_empty() || moves.first > 0)
	{
		const int16_t map_x = (eve.button.x + camera.get_cam_x()) / 32;
		const int16_t map_y = (eve.button.y + camera.get_cam_y()) / 32;

		if (map_x == grid_x && map_y == grid_y)
		{
			if (bubble_timer > 0)
			{
				clear_bubble();
				return;
			}
			if (level->get_wall_type(grid_x, grid_y) == NT_BASE && health.first < health.second)
			{
				add_health(1);
				add_action(ACTION_INTERACT, grid_x, grid_y);
			}
			else turn_done = true;
			moves.first = 0;
			return;
		}
		if (pathfinder->get_path_found())
		{
			// If we don't click the end of the path, recalculate it (if we're not moving)
			if (map_x != pathfinder->get_last_x() || map_y != pathfinder->get_last_y())
			{
				pathfinder->clear_path();
				if (!auto_move_path)
					pathfinder->find_path(level, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y, ACTOR_HERO);
				auto_move_path = false;
			}
			else // If we click the end of a path, start moving there automatically
			{
				command_this_turn = true;
				auto_move_path = true;
			}
		}
		// Otherwise just calculate the new path
		else pathfinder->find_path(level, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y, ACTOR_HERO);
	}
}
bool Hero::get_auto_move() const
{
	if (auto_move_path)
	{
		if (pathfinder->get_path_found())
			return pathfinder->get_length() > 1;
	}
	return false;
}
void Hero::set_sleep_timer(uint8_t timer)
{
	if (sleep_timer > 0 && timer == 0)
		load_bubble("question", 1);

	else if (timer > 0)
	{
		load_bubble("sleep", 5);
		moves = std::make_pair(0, 0);
	}
	else clear_bubble();
	sleep_timer = timer;
}
