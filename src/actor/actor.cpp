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
#include "actor.hpp"
#include "mount.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "texture_manager.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <algorithm> // Actor::remove_ability()

uint16_t Actor::ID = 0;

Actor::Actor() :
	actor_type(ACTOR_NULL), actor_ID(ID++), delete_me(false), in_camera(false), turn_done(false),
	name("???"), hovered(HOVER_NONE), anim_frames(0), anim_timer(0), texture(nullptr), bubble(nullptr),
	status_icon(nullptr), status(STATUS_NONE), bubble_timer(0), combat_level(1), experience(0),
	projectile(nullptr), proj_name("???"), mount(nullptr)
{
	facing_right = (engine.get_rng() % 2 == 0);
	current_action = { ACTION_NULL, 0, 0, 0 };

	moves = std::make_pair(0, 0);
	health = std::make_pair(1, 1);
}
Actor::~Actor()
{
	free();
}
void Actor::free()
{
	if (texture != nullptr)
	{
		engine.get_texture_manager()->free_texture(texture->get_name());
		texture = nullptr;
	}
	if (projectile != nullptr)
	{
		engine.get_texture_manager()->free_texture(projectile->get_name());
		projectile = nullptr;
	}
	if (bubble != nullptr)
	{
		engine.get_texture_manager()->free_texture(bubble->get_name());
		bubble = nullptr;
	}
	if (status_icon != nullptr)
	{
		engine.get_texture_manager()->free_texture(status_icon->get_name());
		status_icon = nullptr;
	}
	if (mount != nullptr)
	{
		mount->set_rider(nullptr);
		mount = nullptr;
	}
	if (!action_queue.empty())
		std::queue<Action>().swap(action_queue);
}
bool Actor::init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	texture = engine.get_texture_manager()->load_texture(texture_name);
	if (texture == nullptr)
		return false;

	actor_type = at;
	x = xpos * 32; y = ypos * 32;
	grid_x = xpos; grid_y = ypos;
	prev_x = xpos; prev_y = ypos;
	frame_rect = { 0, 0, 16, 16 };
	bubble_rect = { 0, 0, 16, 16 };

	abilities.clear();
	add_ability("sleep");

	return true;
}
void Actor::update(Level *level)
{
	if (camera.get_in_camera_grid(grid_x, grid_y))
		in_camera = true;
	else in_camera = false;

	if (current_action.type == ACTION_NULL && !action_queue.empty())
	{
		current_action = action_queue.front();
		action_queue.pop();
	}
	if (current_action.type != ACTION_NULL)
	{
		bool clear_action = false;
		switch (current_action.type)
		{
			case ACTION_MOVE: clear_action = action_move(level); break;
			case ACTION_ATTACK: clear_action = action_attack(level); break;
			case ACTION_SHOOT: clear_action = action_shoot(level); break;
			case ACTION_INTERACT: clear_action = action_interact(); break;
			default: clear_action = true; break;
		}
		if (clear_action)
			current_action.type = ACTION_NULL;
	}
}
void Actor::render() const
{
	if (texture != nullptr && in_camera && !delete_me)
	{
		if (mount == nullptr) // No mount, just render normally
		{
			texture->render(
				x - camera.get_cam_x(), y - camera.get_cam_y(), &frame_rect,
				2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, 0.0
			);
		}
		else // When rendering a mount, we need to "split" our own texture into two
		{
			const uint8_t half_width = frame_rect.w / 2;
			const SDL_Rect rect_left = { frame_rect.x, frame_rect.y, half_width, frame_rect.h };
			const SDL_Rect rect_right = { frame_rect.x + half_width, frame_rect.y, half_width, frame_rect.h };

			texture->render(
				x - camera.get_cam_x() + (facing_right ? frame_rect.w : 0),
				y - camera.get_cam_y() - frame_rect.h, // Raise ourselves half a tile to appear on "top" of the mount
				&rect_left, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, 0.0
			);
			mount->render(); // And render the mount in the middle, to create the illusion of sitting on top of it

			texture->render(
				x - camera.get_cam_x() + (half_width * 2) + (facing_right ? -frame_rect.w : 0),
				y - camera.get_cam_y() - frame_rect.h, // Raise ourselves half a tile to appear on "top" of the mount
				&rect_right, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, 0.0
			);
		}
		if (bubble != nullptr)
			bubble->render(x - camera.get_cam_x(), y - camera.get_cam_y() - 32, &bubble_rect);

		if (status_icon != nullptr)
			status_icon->render(x - camera.get_cam_x(), y - camera.get_cam_y(), &bubble_rect);
	}
}
void Actor::render_ui(uint16_t xpos, uint16_t ypos) const
{
	if (projectile != nullptr)
	{
		projectile->render(
			proj_x - camera.get_cam_x(), proj_y - camera.get_cam_y(), nullptr, 2,
			facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, proj_angle
		);
	}
}
void Actor::start_turn()
{
	// Called when our turn to move begins.
	turn_done = true;
}
bool Actor::take_turn(Level *level)
{
	// Called continuously every frame when it's our turn.
	// Return true once we're done with our turn.
	return (turn_done && action_queue.empty());
}
void Actor::end_turn()
{
	// Called when our turn to move ends.
	if (bubble_timer > 0)
	{
		bubble_timer -= 1;
		if (bubble_timer == 0 && bubble != nullptr)
		{
			engine.get_texture_manager()->free_texture(bubble->get_name());
			bubble = nullptr;
		}
	}
}
uint8_t Actor::get_damage() const
{
	std::cout << "warning! Actor::get_damage() has no override!" << std::endl;
	return 1;
}
void Actor::add_action(ActionType at, uint8_t xpos, uint8_t ypos, int8_t value)
{
	Action a = { at, xpos, ypos, value };
	action_queue.push(a);
}
bool Actor::actions_empty() const
{
	return (action_queue.empty() && current_action.type == ACTION_NULL);
}
void Actor::action_idle()
{
	if (bubble_rect.y == 0)
		bubble_rect.y = 16;
	else bubble_rect.y = 0;

	if (actor_type == ACTOR_HERO && moves.first <= 0)
		frame_rect.y = 16;

	else if (current_action.type == ACTION_NULL)
	{
		if (frame_rect.y == 0)
			frame_rect.y = 16;
		else frame_rect.y = 0;
		bubble_rect = frame_rect;
	}
}
bool Actor::action_move(Level *level)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0)
		{
			turn_done = true;

			if (grid_x != current_action.xpos)
				facing_right = (grid_x < current_action.xpos);

			if (mount != nullptr && current_action.action_value == 1)
			{
				level->set_actor(grid_x, grid_y, mount);
				mount->set_rider(nullptr);
				set_mount(nullptr);
			}
			else level->set_actor(grid_x, grid_y, nullptr);

			prev_x = grid_x;
			prev_y = grid_y;
			grid_x = current_action.xpos;
			grid_y = current_action.ypos;

			level->set_actor(grid_x, grid_y, this);

			if (!in_camera)
			{
				anim_frames = 16;
				break;
			}
			frame_rect.y = 16;
		}
		else if (anim_frames == 4)
			frame_rect.y = 0;
		else if (anim_frames == 12)
			frame_rect.y = 16;

		if (anim_frames % 2 == 0)
		{
			if (anim_frames < 8)
				y -= anim_frames;
			else y += anim_frames - 8;
		}
		if (prev_x < grid_x) x += ((grid_x - prev_x) * 16) / 8;
		else if (prev_x > grid_x) x -= ((prev_x - grid_x) * 16) / 8;
		if (prev_y < grid_y) y += ((grid_y - prev_y) * 16) / 8;
		else if (prev_y > grid_y) y -= ((prev_y - grid_y) * 16) / 8;

		anim_frames += 1;
	}
	if (anim_frames >= 16)
	{
		frame_rect.y = 0;
		anim_timer = 0;
		anim_frames = 0;
		x = grid_x * 32;
		y = grid_y * 32;
		return true;
	}
	return false;
}
bool Actor::action_attack(Level *level)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0)
		{
			if (grid_x != current_action.xpos)
				facing_right = (grid_x < current_action.xpos);
		}
		else if (anim_frames == 6)
		{
			turn_done = true;

			Actor *temp_actor = level->get_actor(current_action.xpos, current_action.ypos);
			if (temp_actor != nullptr)
				attack(temp_actor);
		}
		if (anim_frames % 2 == 0)
		{
			if (anim_frames < 6) // Moving towards target
			{
				if (grid_x < current_action.xpos) x += 6;
				else if (grid_x > current_action.xpos) x -= 6;
				if (grid_y < current_action.ypos) y += 6;
				else if (grid_y > current_action.ypos) y -= 6;
			}
			else // Moving away from target
			{
				if (grid_x < current_action.xpos) x -= 6;
				else if (grid_x > current_action.xpos) x += 6;
				if (grid_y < current_action.ypos) y -= 6;
				else if (grid_y > current_action.ypos) y += 6;
			}
		}
		anim_frames += 1;
	}
	if (anim_frames >= 12)
	{
		anim_timer = 0;
		anim_frames = 0;
		x = grid_x * 32;
		y = grid_y * 32;
		return true;
	}
	return false;
}
bool Actor::action_shoot(Level *level)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0)
		{
			if (projectile == nullptr)
				projectile = engine.get_texture_manager()->load_texture(proj_name);

			if (grid_x != current_action.xpos)
				facing_right = (grid_x < current_action.xpos);

			proj_angle = facing_right ? 45.0 : -45.0;
		}
		if (anim_frames < 8)
		{
			if (grid_x < current_action.xpos) x += (anim_frames < 4) ? -2 : 2;
			else if (grid_x > current_action.xpos) x += (anim_frames < 4) ? 2 : -2;
			if (grid_y < current_action.ypos) y += (anim_frames < 4) ? -2 : 2;
			else if (grid_y > current_action.ypos) y += (anim_frames < 4) ? 2 : -2;

			proj_x = x;
			proj_y = y;
		}
		else
		{
			if (anim_frames % 2 == 0)
			{
				if (anim_frames < 16)
					proj_y -= 4;
				else proj_y += 4;
			}
			if (grid_x < current_action.xpos) proj_x += ((current_action.xpos - grid_x) * 16) / 8;
			else if (grid_x > current_action.xpos) proj_x -= ((grid_x - current_action.xpos) * 16) / 8;
			if (grid_y < current_action.ypos) proj_y += ((current_action.ypos - grid_y) * 16) / 8;
			else if (grid_y > current_action.ypos) proj_y -= ((grid_y - current_action.ypos) * 16) / 8;

			proj_angle += facing_right ? 6.0 : -6.0;
		}
		anim_frames += 1;
	}
	if (anim_frames >= 24)
	{
		if (projectile != nullptr)
		{
			engine.get_texture_manager()->free_texture(projectile->get_name());
			projectile = nullptr;
		}
		Actor *temp_actor = level->get_actor(current_action.xpos, current_action.ypos);
		if (temp_actor != nullptr)
			attack(temp_actor);

		turn_done = true;
		anim_timer = 0;
		anim_frames = 0;
		return true;
	}
	return false;
}
bool Actor::action_interact()
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames % 2 == 0)
		{
			if (anim_frames < 6)
				y -= 2;
			else y += 2;
		}
		anim_frames += 1;
	}
	if (anim_frames >= 12)
	{
		turn_done = true;
		anim_timer = 0;
		anim_frames = 0;
		x = grid_x * 32;
		y = grid_y * 32;
		return true;
	}
	return false;
}
void Actor::add_ability(const std::string &ability)
{
	if (!has_ability(ability))
		abilities.push_back(ability);
}
void Actor::remove_ability(const std::string &ability)
{
	abilities.erase(std::remove(abilities.begin(), abilities.end(), ability), abilities.end());
}
bool Actor::has_ability(const std::string &ability) const
{
	for (auto a : abilities)
	{
		if (a == ability)
			return true;
	}
	return false;
}
void Actor::attack(Actor *other)
{
	if (other == nullptr || other->get_actor_type() == ACTOR_MOUNT, ui.get_message_log() == nullptr)
		return;

	MessageLog *ml = ui.get_message_log();

	uint8_t damage = get_damage();
	const bool crit = engine.get_rng() % 20 == 0;
	if (crit) damage *= 2;

	const int8_t result_health = other->health.first - damage;
	other->health.first = result_health;

	if (result_health < 1)
	{
		other->set_delete(true);
		ml->add_message("The " + name + " kills the " + other->name + "! (%6" + std::to_string(damage) + "%F damage)");

		experience += other->combat_level;
		if (experience >= combat_level * 10)
		{
			set_status(STATUS_LEVELUP);
			add_ability("level-up");
		}
	}
	else ml->add_message("The " + name + std::string(crit ? " %ECRITS%F the " : " strikes the ") + other->name + " for %6" + std::to_string(damage) + "%F damage!");
}
void Actor::load_bubble(const std::string &bubble_name, uint8_t timer)
{
	if (bubble != nullptr)
		engine.get_texture_manager()->free_texture(bubble->get_name());

	bubble = engine.get_texture_manager()->load_texture("core/texture/ui/bubble/" + bubble_name + ".png");
	if (bubble != nullptr)
		bubble_timer = timer;
}
void Actor::clear_bubble()
{
	if (bubble != nullptr)
		engine.get_texture_manager()->free_texture(bubble->get_name());

	bubble_timer = 0;
	bubble = nullptr;
}
void Actor::set_status(StatusType st)
{
	if (status_icon != nullptr)
	{
		engine.get_texture_manager()->free_texture(status_icon->get_name());
		status_icon = nullptr;
	}
	if (st == STATUS_LEVELUP)
		status_icon = engine.get_texture_manager()->load_texture("core/texture/ui/status/level_up.png");

	if (status_icon != nullptr)
		status = st;
}
void Actor::set_mount(Mount *m)
{
	mount = m;

	if (mount != nullptr)
		mount->set_rider(this);
}
