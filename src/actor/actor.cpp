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
#include "level.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "texture_manager.hpp"

uint16_t Actor::ID = 0;

Actor::Actor() :
	actor_type(ACTOR_NULL), actor_ID(ID++), delete_me(false), in_camera(false), turn_done(false),
	hovered(false), anim_frames(0), anim_timer(0), texture(nullptr), bubble(nullptr), bubble_timer(0)
{
	facing_right = (engine.get_rng() % 2 == 0);
	current_action = { ACTION_NULL, 0, 0 };
	moves = std::make_pair(0, 0);
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
	if (bubble != nullptr)
	{
		engine.get_texture_manager()->free_texture(bubble->get_name());
		bubble = nullptr;
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
		texture->render(
			x - camera.get_cam_x(),
			y - camera.get_cam_y(),
			&frame_rect, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, 0.0
		);
		render_bubble();
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
void Actor::add_action(ActionType at, uint8_t xpos, uint8_t ypos)
{
	Action a = { at, xpos, ypos };
	action_queue.push(a);
}
bool Actor::actions_empty() const
{
	return (action_queue.empty() && current_action.type == ACTION_NULL);
}
void Actor::action_idle()
{
	if (current_action.type == ACTION_NULL)
	{
		if (frame_rect.y == 0)
			frame_rect.y = 16;
		else frame_rect.y = 0;
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

			level->set_actor(grid_x, grid_y, nullptr);
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
				temp_actor->set_delete(true);
			//level->set_actor(current_action.xpos, current_action.ypos, nullptr);
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
void Actor::load_bubble(const std::string &bubble_name, uint8_t timer)
{
	if (bubble != nullptr)
		engine.get_texture_manager()->free_texture(bubble->get_name());

	bubble = engine.get_texture_manager()->load_texture("core/texture/ui/bubble/" + bubble_name + ".png");
	if (bubble != nullptr)
		bubble_timer = timer;
}
void Actor::render_bubble() const
{
	if (bubble != nullptr)
		bubble->render(x - camera.get_cam_x(), y - camera.get_cam_y() - 32);
}
