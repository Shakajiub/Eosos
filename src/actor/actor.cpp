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
#include "ability.hpp"
#include "dungeon.hpp"
#include "level.hpp"
#include "texture.hpp"

#include "monster.hpp"
#include "mount.hpp"
#include "player.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "object.hpp"
#include "particle_manager.hpp"
#include "sound_manager.hpp"
#include "texture_manager.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include "bitmap_font.hpp"

uint16_t Actor::ID = 0;

Actor::Actor() :
	current_action(nullptr), in_camera(false), turn_done(false), has_mount(false), render_angle(0.0), actor_ID(ID++),
	move_direction(0), anim_frames(0), anim_timer(0), name("???"), texture(nullptr), bubble(nullptr), mount(nullptr),
	bubble_timer(0), visibility_range(2)
{
	for (uint8_t i = 0; i < 7; i++)
		skills[i] = 1;

	health = std::make_pair(9, 9);
	moves = std::make_pair(0, 0);

	facing_right = (engine.get_rng() % 2 == 0);
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
	if (mount != nullptr)
	{
		delete mount;
		mount = nullptr;
	}
	for (auto &it : abilities) if (it.second != nullptr)
		delete it.second;
	abilities.clear();
}
bool Actor::init(const std::string &actor_name, uint8_t xpos, uint8_t ypos, Texture *sheet)
{
	if (sheet == nullptr)
		return false;

	name = actor_name; texture = sheet;
	x = xpos * 32; y = ypos * 32;
	grid_x = xpos; grid_y = ypos;
	prev_x = xpos; prev_y = ypos;
	frame_rect = { 0, 0, 16, 16 };

	return true;
}
bool Actor::init(const std::string &actor_name, uint8_t xpos, uint8_t ypos, const std::string &texture_name)
{
	texture = engine.get_texture_manager()->load_texture(texture_name);
	return init(actor_name, xpos, ypos, texture);
}
Actor* Actor::find(Dungeon *scene, uint8_t xpos, uint8_t ypos)
{
	if (scene != nullptr && scene->get_level() != nullptr)
		return scene->get_level()->get_actor(xpos, ypos);
	return nullptr;
}
void Actor::update(Dungeon *scene)
{
	if (camera.get_in_camera_grid(grid_x, grid_y))
		in_camera = true;
	else in_camera = false;

	if (current_action != nullptr || !action_queue.empty())
	{
		if (current_action == nullptr)
		{
			current_action = action_queue.front();
			action_queue.pop();
		}
		bool delete_action = false;
		switch (current_action->action_type)
		{
			case AT_MOVE: if (!action_move(scene)) delete_action = true; break;
			case AT_ATTACK: if (!action_attack(scene)) delete_action = true; break;
			case AT_INTERACT: if (!action_interact(scene)) delete_action = true; break;
			default:
				delete_action = true;
				if (action_queue.empty())
					end_turn();
				break;
		}
		if (delete_action)
		{
			delete current_action;
			current_action = nullptr;

			prev_x = grid_x;
			prev_y = grid_y;
		}
	}
	if (mount != nullptr)
		mount->update(scene);
}
void Actor::render() const
{
	if (texture != nullptr && in_camera)
	{
		if (mount == nullptr) // No mount, just render normally
		{
			texture->render(
				x - camera.get_cam_x(),
				y - camera.get_cam_y(),
				&frame_rect, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, render_angle
			);
		}
		else // When rendering a mount, we need to "split" our own texture into two
		{
			const uint8_t half_width = frame_rect.w / 2;
			const SDL_Rect rect_left = { frame_rect.x, frame_rect.y, half_width, frame_rect.h };
			const SDL_Rect rect_right = { frame_rect.x + half_width, frame_rect.y, half_width, frame_rect.h };

			texture->render(
				x - camera.get_cam_x() + (facing_right ? frame_rect.w : 0),
				y - camera.get_cam_y() - frame_rect.h, // Here we raise ourselves half a tile to appear a bit off-tile
				&rect_left, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, render_angle
			);
			mount->render(); // And render the mount in the middle, to create the illusion of sitting on top of it

			texture->render(
				x - camera.get_cam_x() + (half_width * 2) + (facing_right ? -frame_rect.w : 0),
				y - camera.get_cam_y() - frame_rect.h, // Here we raise ourselves half a tile to appear a bit off-tile
				&rect_right, 2, facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, render_angle
			);
		}
	}
}
void Actor::start_turn()
{
	// This is where an actor counts how many turns it should take.
}
bool Actor::take_turn(Dungeon *scene)
{
	end_turn();
	turn_done = false;
	return true;
}
void Actor::end_turn()
{
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
void Actor::add_ability(const std::string &name, Ability *ability)
{
	auto it = abilities.find(name);
	if (it == abilities.end())
		abilities[name] = ability;
	else std::cout << "attempting to add already-existing ability '" << name << "'!" << std::endl;
}
bool Actor::use_ability(const std::string &name, uint8_t value)
{
	auto it = abilities.find(name);
	if (it != abilities.end() && it->second != nullptr)
	{
		it->second->apply(this, value);
		return true;
	}
	return false;
}
void Actor::add_action(ActionType at, uint8_t xpos, uint8_t ypos)
{
	Action *a = new Action;
	a->action_type = at;
	a->xpos = xpos;
	a->ypos = ypos;
	action_queue.push(a);
}
void Actor::attack(Dungeon *scene, Actor *other)
{
	if (scene == nullptr || scene->get_level() == nullptr ||
		other == nullptr || ui.get_message_log() == nullptr)
		return;

	MessageLog *ml = ui.get_message_log();

	const std::pair<uint8_t, bool> damage = get_melee_damage();
	const uint8_t armor_value = other->get_armor_value();

	uint8_t armor_reduction = 0;
	if (armor_value > 0)
	{
		// TODO - Better armor calculations (this was done in a very tired state)

		const uint8_t percent_armor = damage.first - (damage.first * (armor_value * 0.1f));
		const uint8_t simple_armor = damage.first - (armor_value / 10);

		armor_reduction = (percent_armor > simple_armor) ? percent_armor : simple_armor;

		if (armor_reduction >= damage.first)
			armor_reduction = damage.first - 1;
	}
	const uint8_t final_damage = damage.first - armor_reduction;
	const bool crit = damage.second;

	const int8_t final_health = other->health.first - final_damage;
	other->health.first = final_health;

	engine.get_particle_manager()->spawn_particle(
		other->x + 16, other->y + 16, std::to_string(final_damage), COLOR_BERRY, crit ? 2 : 1
	);
	if (other != player) // An actor that is not the player has been hit
	{
		if (final_health < 1) // And it has died!
		{
			scene->get_level()->erase_actor(other->get_grid_x(), other->get_grid_y());
			if (this == player)
				ml->add_message("You kill the " + other->name + "! (%6" + std::to_string(final_damage) + "%F damage)");
		}
		else if (this == player)
		{
			// Attacking a monster instantly aggroes it
			Monster *m = dynamic_cast<Monster*>(other);
			//if (m != NULL) m->set_state(AS_FOLLOW);

			if (player->get_behind(other))
				other->turn_around();

			ml->add_message("You " + std::string(crit ? "%ECRIT%F the " : "strike the ") + other->name + " for %6" + std::to_string(final_damage) + "%F damage!");
		}
	}
	else // The player has been hit
	{
		if (engine.get_sound_manager() != nullptr)
			engine.get_sound_manager()->set_playlist(PT_COMBAT);

		if (final_health < 1)
		{
			ml->add_message("You have died!", COLOR_BERRY);
			scene->get_level()->spawn_object(player->get_grid_x(), player->get_grid_y(), "core/script/object/gravestone.lua", "player");
		}
		else ml->add_message("The " + name + std::string(crit ? " %ECRITS%F" : " strikes") + " you for %6" + std::to_string(final_damage) + "%F damage!");
	}
}
std::pair<uint8_t, bool> Actor::get_melee_damage() const
{
	uint8_t damage = (skills[A_STR] > 1) ? skills[A_STR] : 1;

	bool crit = false;
	if (engine.get_rng() % 10 == 0)
	{
		crit = true;
		damage *= 2;
	}
	return std::make_pair(damage, crit);
}
uint8_t Actor::get_armor_value() const
{
	// This should return a "percentage" (a value between 0 and 100)
	// The percentage will directly decrease the amount of physical damage taken
	// 18 armor value on a 5 damage attack will reduce it to 4 damage
	// Every 10 will also always decrease the damage by 1, for example, 12 armor will reduce 3 dmg to 2
	// 20 armor will therefore reduce a 5 damage attack to 3

	return 0;
}
void Actor::take_damage(Level *level, int8_t damage, const std::string &source)
{
	health.first -= damage;
	engine.get_particle_manager()->spawn_particle(x + 16, y + 16, std::to_string(damage), COLOR_BERRY);

	if (this != player)
	{
		ui.get_message_log()->add_message("The " + name + " takes %6" + std::to_string(damage) + "%F damage from the " + source + "!");
		if (health.first < 1)
			level->erase_actor(grid_x, grid_y);
	}
	else
	{
		ui.get_message_log()->add_message("You take %6" + std::to_string(damage) + "%F damage from the " + source + "!");
		if (health.first < 1)
			level->spawn_object(grid_x, grid_y, "core/script/object/gravestone.lua", "player");
	}
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
void Actor::action_idle()
{
	if (current_action == nullptr)
	{
		if (frame_rect.y == 0)
			frame_rect.y = 16;
		else frame_rect.y = 0;
	}
}
bool Actor::action_move(Dungeon *scene)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0)
		{
			frame_rect.y = 16;
			turn_done = true;

			if (grid_x != current_action->xpos)
				facing_right = (grid_x < current_action->xpos);

			scene->get_level()->set_actor(grid_x, grid_y, nullptr);

			prev_x = grid_x;
			prev_y = grid_y;

			grid_x = current_action->xpos;
			grid_y = current_action->ypos;

			scene->get_level()->set_actor(grid_x, grid_y, this);

			if (!in_camera)
			{
				anim_frames = 16;
				break;
			}
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

		return false;
	}
	return true;
}
bool Actor::action_attack(Dungeon *scene)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0) // Start of the action
		{
			if (grid_x != current_action->xpos) // Determine facing direction
				facing_right = (grid_x < current_action->xpos);
		}
		else if (anim_frames == 6)
		{
			turn_done = true;
			attack(scene, Actor::find(scene, current_action->xpos, current_action->ypos));
		}
		if (anim_frames % 2 == 0) // Every other loop
		{
			if (anim_frames < 6) // Moving towards target
			{
				if (grid_x < current_action->xpos) x += 6;
				else if (grid_x > current_action->xpos) x -= 6;

				if (grid_y < current_action->ypos) y += 6;
				else if (grid_y > current_action->ypos) y -= 6;
			}
			else // Moving away from target
			{
				if (grid_x < current_action->xpos) x -= 6;
				else if (grid_x > current_action->xpos) x += 6;

				if (grid_y < current_action->ypos) y -= 6;
				else if (grid_y > current_action->ypos) y += 6;
			}
		}
		anim_frames += 1;
	}
	if (anim_frames >= 12) // End of the action
	{
		anim_timer = 0;
		anim_frames = 0;

		x = grid_x * 32;
		y = grid_y * 32;

		return false;
	}
	return true;
}
bool Actor::action_interact(Dungeon *scene)
{
	anim_timer += engine.get_dt();
	while (anim_timer > 18 || !in_camera)
	{
		anim_timer -= 18;
		if (anim_frames == 0) // Start of the action
		{
			if (grid_x != current_action->xpos) // Determine facing direction
				facing_right = (grid_x < current_action->xpos);
		}
		else if (anim_frames == 6 && !(grid_x == current_action->xpos && grid_y == current_action->ypos))
		{
			turn_done = true;
			scene->get_level()->get_object(current_action->xpos, current_action->ypos)->interact();
			//if (this == player)
				//scene->get_level()->do_fov(grid_x, grid_y, player->get_visibility_range(), true);
		}
		if (anim_frames % 2 == 0) // Every other loop
		{
			if (anim_frames < 6) // Moving towards target
			{
				if (grid_x < current_action->xpos) x += 6;
				else if (grid_x > current_action->xpos) x -= 6;

				if (grid_y < current_action->ypos) y += 6;
				else if (grid_y > current_action->ypos) y -= 6;
				else y -= 2; // Interacting with an object on the same node
			}
			else // Moving away from target
			{
				if (grid_x < current_action->xpos) x -= 6;
				else if (grid_x > current_action->xpos) x += 6;

				if (grid_y < current_action->ypos) y -= 6;
				else if (grid_y > current_action->ypos) y += 6;
				else y += 2; // Interacting with an object on the same node
			}
		}
		anim_frames += 1;
	}
	if (anim_frames >= 12) // End of the action
	{
		if (grid_x == current_action->xpos && grid_y == current_action->ypos)
		{
			turn_done = true;
			scene->get_level()->get_object(current_action->xpos, current_action->ypos)->interact();
		}
		anim_timer = 0;
		anim_frames = 0;

		x = grid_x * 32;
		y = grid_y * 32;

		return false;
	}
	return true;
}
void Actor::init_mount()
{
	mount = new Mount;
	if (mount->init("mount" + std::to_string(ID), grid_x, grid_y, "core/texture/actor/quadruped/horse.png"))
	{
		mount->set_rider(this);
		std::cout << "mount initialized" << std::endl;
	}
	else mount = nullptr;
}
void Actor::toggle_mount()
{
	if (mount != nullptr)
	{
		delete mount;
		mount = nullptr;
	}
	else init_mount();
}
bool Actor::get_behind(const Actor *other) const
{
	if (other != nullptr)
	{
		if ((other->facing_right && grid_x < other->grid_x) ||
			(!other->facing_right && grid_x > other->grid_x))
			return true;
	}
	return false;
}
bool Actor::get_player_visible(Dungeon *scene) const
{
	if (player->get_behind(this))
		return false;
	if (scene != nullptr && scene->get_level() != nullptr)
		return scene->get_level()->get_node_discovered(grid_x, grid_y);
	return false;
}
void Actor::set_skill(const uint8_t skill, const int8_t value)
{
	skills[skill] = value;

	if (skill == A_CON)
	{
		health.second = value - (value % 3);

		if (health.second < 3)
			health.second = 3;

		if (health.first > health.second)
			health.first = health.second;
	}
}
void Actor::set_position(uint8_t new_x, uint8_t new_y, Dungeon *scene)
{
	if (scene != nullptr)
		scene->get_level()->set_actor(grid_x, grid_y, nullptr);

	const uint8_t prev_x = grid_x;

	grid_x = new_x; x = grid_x * 32;
	grid_y = new_y; y = grid_y * 32;

	if (prev_x > grid_x && facing_right)
		facing_right = !facing_right;

	if (scene != nullptr)
	{
		if (this == player)
		{
			camera.update_position(x, y);
			scene->get_level()->do_fov(grid_x, grid_y, player->get_visibility_range(), true);
		}
		scene->get_level()->set_actor(grid_x, grid_y, this);
	}
}
