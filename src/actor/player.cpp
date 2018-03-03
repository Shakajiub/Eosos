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
#include "player.hpp"
#include "ability_manager.hpp"
#include "astar.hpp"
#include "dijkstra.hpp"
#include "item.hpp"

#include "ability.hpp"
#include "camera.hpp"
#include "options.hpp"
#include "lua_manager.hpp"
#include "object.hpp"
#include "dungeon.hpp"
#include "level.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "inventory.hpp"
#include "ui.hpp"

Player *player = new Player;

uint8_t direction_to_index(uint8_t direction)
{
	switch (direction)
	{
		case 1: return 0; case 2: return 1;
		case 4: return 2; case 8: return 3;
		case 5: return 4; case 9: return 5;
		case 6: return 6; case 10: return 7;
		default: return UINT8_MAX;
	}
}
Player::Player() :
	auto_move_path(false), input_direction(0), turn_timer(0), total_armor(0), ability_manager(nullptr),
	pathfinder(nullptr), dijkstra(nullptr), activated_ability(nullptr), class_script(nullptr), interact_below(false)
{
	has_mount = true;

	weapon_values.clear();
	weapon_values = { 0, 0, 0 };
}
Player::~Player()
{
	if (ability_manager != nullptr)
	{
		delete ability_manager;
		ability_manager = nullptr;
	}
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
	if (dijkstra != nullptr)
	{
		delete dijkstra;
		dijkstra = nullptr;
	}
	if (class_script != nullptr)
	{
		engine.get_lua_manager()->free_script(class_script_name);
		class_script = nullptr;
	}
}
void Player::update(Dungeon *scene)
{
	Actor::update(scene);
}
void Player::render() const
{
	if (health.first > 0)
		Actor::render();
	if (pathfinder != nullptr)
		pathfinder->render();
	if (dijkstra != nullptr)
		dijkstra->render_map();

	// TEMPORARY - Draw debug amage and armor values
	std::string damage = std::to_string(weapon_values[0]) + "d" + std::to_string(weapon_values[1]);
	if (weapon_values[2] > 0)
		damage += "+" + std::to_string(weapon_values[2]);
	ui.get_bitmap_font()->render_text(16, 250, damage);
	ui.get_bitmap_font()->render_text(16, 250 + ui.get_bitmap_font()->get_height(), std::to_string(total_armor));
}
void Player::start_turn()
{
	if (mount != nullptr)
		moves = std::make_pair(2, 2);
	else moves = std::make_pair(1, 1);
}
bool Player::take_turn(Dungeon *scene)
{
	if (turn_done)
	{
		if (pathfinder != nullptr && pathfinder->get_path_found())
		{
			if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
				pathfinder->step(scene);
			else pathfinder->clear_path();
		}
		scene->get_level()->do_fov(grid_x, grid_y, visibility_range);

		if (prev_x != grid_x || prev_y != grid_y) // We have moved to another node
		{
			ui.toggle_inventory(0, 0); // Close any temporary inventory
			camera.update_position(grid_x * 32, grid_y * 32); // Move camera to our position
		}
		moves.first -= 1;
		if (moves.first < 1)
		{
			if (dijkstra != nullptr)
				dijkstra->build_map(scene);

			if (scene->get_level() != nullptr &&
				scene->get_level()->get_map_dark() &&
				ui.get_inventory(INV_EQUIP) != nullptr)
				ui.get_inventory(INV_EQUIP)->modify_durability(ET_LIGHT, -1);

			if (ability_manager != nullptr)
				ability_manager->cooldown_abilities(1);

			end_turn();
			turn_done = false;
			return true;
		}
		else turn_done = false;
		return false;
	}
	else if (action_queue.empty() && current_action == nullptr)
	{
		if (pathfinder != nullptr && auto_move_path)
		{
			step_pathfinder(scene);
			if (!pathfinder->get_path_found())
				auto_move_path = false;
		}
		else if (interact_below)
		{
			const Object *temp_obj = scene->get_level()->get_object(grid_x, grid_y);
			if (temp_obj != nullptr)
				add_action(AT_INTERACT, grid_x, grid_y);

			interact_below = false;
		}
		else if (input_direction != 0)
		{
			if (activated_ability != nullptr)
			{
				if (activated_ability->get_target_type() == AT_DIRECTION &&
					activated_ability->validate_value(direction_to_index(input_direction)))
					activated_ability->apply(this, input_direction);

				input_direction = 0;
				return false;

				// TODO - Handle other targetting types
			}
			const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
			const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
			const uint8_t i = direction_to_index(input_direction);

			const Actor *temp_actor = scene->get_level()->get_actor(grid_x + offset_x[i], grid_y + offset_y[i]);
			const Object *temp_obj = scene->get_level()->get_object(grid_x + offset_x[i], grid_y + offset_y[i]);

			if (i == UINT8_MAX)
			{
				input_direction = 0;
				return false;
			}
			// If there's an actor on the node we want to move to, ATTACK WITHOUT DOUBT
			if (temp_actor != nullptr)
				add_action(AT_ATTACK, grid_x + offset_x[i], grid_y + offset_y[i]);

			// If there's an object, interact with it if necessary
			else if (temp_obj != nullptr)
			{
				if (temp_obj->get_passable() && !(SDL_GetModState() & KMOD_CTRL))
					add_action(AT_MOVE, grid_x + offset_x[i], grid_y + offset_y[i]);
				else add_action(AT_INTERACT, grid_x + offset_x[i], grid_y + offset_y[i]);
			}
			// If there's no wall, just move
			else if (!scene->get_level()->get_wall(grid_x + offset_x[i], grid_y + offset_y[i]))
			{
				add_action(AT_MOVE, grid_x + offset_x[i], grid_y + offset_y[i]);
				ability_manager->trigger_abilities(PT_MOVEMENT);
			}
			input_direction = 0;
		}
	}
	return false;
}
void Player::end_turn()
{
	Actor::end_turn();

	// Reduce the durability of all equipped light sources by 1
	//if (ui.get_inventory(INV_EQUIP) != nullptr)
		//ui.get_inventory(INV_EQUIP)->modify_durability(ET_LIGHT, -1);
}
bool Player::init_class_script(const std::string &script)
{
	class_script = engine.get_lua_manager()->load_script(script);

	if (class_script != nullptr)
		class_script_name = script;

	return class_script != nullptr;
}
std::string Player::get_class_texture(const std::string &race)
{
	std::string appearance = "core/texture/actor/player/human/human_1.png";
	if (class_script != nullptr)
	{
		// Get the texture for our race-class combo
		lua_getglobal(class_script, "get_class_appearance");
		lua_pushstring(class_script, race.c_str());

		if (lua_pcall(class_script, 1, 1, 0) == LUA_OK)
		{
			appearance = lua_tostring(class_script, -1);
			lua_pop(class_script, 1);
		}
		else std::cout << "lua error: " << lua_tostring(class_script, -1) << std::endl;
	}
	return appearance;
}
void Player::init_ability_manager(Dungeon *scene)
{
	ability_manager = new AbilityManager;
	ability_manager->init_abilities(scene);
}
void Player::init_inventory()
{
	if (class_script != nullptr && ui.get_inventory(INV_BACKPACK) != nullptr)
	{
		lua_getglobal(class_script, "get_initial_items");
		if (lua_pcall(class_script, 0, 1, 0) == LUA_OK)
		{
			Item *temp = nullptr;

			for (lua_pushnil(class_script); lua_next(class_script, -2); lua_pop(class_script, 1))
			{
				const std::string value = lua_tostring(class_script, -1);
				const std::size_t split = value.find('|');

				if (split != std::string::npos)
				{
					const std::string item_script = value.substr(0, split);
					const std::string item_name = value.substr(split + 1);

					temp = new Item;
					temp->init(item_script, item_name);

					ui.get_inventory(INV_BACKPACK)->push_item(temp);
				}
			}
			temp = nullptr;
		}
		else std::cout << "lua error: " << lua_tostring(class_script, -1) << std::endl;
	}
	else std::cout << "can't spawn initial inventory!" << std::endl;
}
void Player::init_pathfinder(Dungeon *scene)
{
	pathfinder = new AStar;
	pathfinder->init();

	dijkstra = new Dijkstra;
	dijkstra->build_map(scene);
}
void Player::step_pathfinder(Dungeon *scene)
{
	if (scene->get_current_actor() != this)
		return;

	const Actor *temp_actor = scene->get_level()->get_actor(pathfinder->get_goto_x(), pathfinder->get_goto_y());
	const Object *temp_obj = scene->get_level()->get_object(pathfinder->get_goto_x(), pathfinder->get_goto_y());

	if (temp_actor != nullptr)
	{
		if (grid_x == pathfinder->get_goto_x() && grid_y == pathfinder->get_goto_y())
			return;
		add_action(AT_ATTACK, pathfinder->get_goto_x(), pathfinder->get_goto_y());
		pathfinder->clear_path();
	}
	else if (temp_obj != nullptr && (!temp_obj->get_passable() || (SDL_GetModState() & KMOD_CTRL)))
	{
		add_action(AT_INTERACT, pathfinder->get_goto_x(), pathfinder->get_goto_y());
		pathfinder->clear_path();
	}
	else add_action(AT_MOVE, pathfinder->get_goto_x(), pathfinder->get_goto_y());
}
bool Player::input_keyboard_down(SDL_Keycode key)
{
	if (auto_move_path)
		return true;
	bool got_input = true;

	switch (key)
	{
		case SDLK_UP: case SDLK_KP_8: case SDLK_k: input_direction = 1; break;
		case SDLK_DOWN: case SDLK_KP_2: case SDLK_j: input_direction = 2; break;
		case SDLK_LEFT: case SDLK_KP_4: case SDLK_h: input_direction = 4; break;
		case SDLK_RIGHT: case SDLK_KP_6: case SDLK_l: input_direction = 8; break;
		case SDLK_KP_7: case SDLK_y: input_direction = 5; break;
		case SDLK_KP_9: case SDLK_u: input_direction = 9; break;
		case SDLK_KP_1: case SDLK_b: input_direction = 6; break;
		case SDLK_KP_3: case SDLK_n: input_direction = 10; break;
		case SDLK_KP_5: case SDLK_SPACE: turn_done = true; break;
		case SDLK_f: turn_around(); break;
		case SDLK_x: interact_below = true; break;
		default: got_input = false; break;
	}
	if (!got_input && ability_manager != nullptr)
		got_input = ability_manager->keyboard_input(key);
	return got_input;
}
void Player::input_keyboard_up(SDL_Keycode key)
{
	input_direction = 0;
}
void Player::input_controller_down(uint8_t index, uint8_t value)
{
	if (value == 1) switch (index)
	{
		case 4: input_direction = 1; break;
		case 6: input_direction = 2; break;
		case 7: input_direction = 4; break;
		case 5: input_direction = 8; break;
		case 14: interact_below = true; break;
		case 13: turn_done = true; break;
		default: break;
	}
}
void Player::input_controller_up()
{
	input_direction = 0;
}
void Player::input_mouse_button_down(Dungeon *scene, SDL_Event eve)
{
	if (pathfinder != nullptr)
	{
		const int16_t map_x = (eve.button.x + camera.get_cam_x()) / 32;
		const int16_t map_y = (eve.button.y + camera.get_cam_y()) / 32;

		if (eve.button.button == SDL_BUTTON_RIGHT)
		{
			scene->get_level()->spawn_object(map_x, map_y, "core/script/object/chest.lua", "chest");
			return;
		}
		if (map_x == grid_x && map_y == grid_y)
		{
			if (auto_move_path)
				auto_move_path = false;
			else turn_done = true;
			return;
		}
		if (!scene->get_level()->get_node_discovered((uint8_t)map_x, (uint8_t)map_y))
		{
			pathfinder->clear_path();
			auto_move_path = false;
			return;
		}
		if (pathfinder->get_path_found())
		{
			// If we don't click the end of the path, recalculate it (if we're not moving)
			if (map_x != pathfinder->get_last_x() || map_y != pathfinder->get_last_y())
			{
				pathfinder->clear_path();
				if (!auto_move_path)
					pathfinder->find_path(scene, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y);
				auto_move_path = false;
			}
			// If we click the end of a path, start moving there automatically
			else auto_move_path = true;
		}
		// Otherwise just calculate the new path
		else pathfinder->find_path(scene, grid_x, grid_y, (int8_t)map_x, (int8_t)map_y);
		anim_timer = 0;
	}
}
void Player::update_item_values(const std::vector<Item*> &equipped_items)
{
	// This function is called whenever an item gets equipped or un-equipped,
	turn_done = true; // that's why we end the turn right here

	weapon_values.clear();
	weapon_values = { 0, 0, 0 };

	total_armor = 0;
	visibility_range = 2;

	uint8_t weapons = 0;
	bool small_weapons = true;
	bool large_weapon = false;

	for (uint8_t i = 0; i < equipped_items.capacity(); i++)
	{
		const Item *item = equipped_items[i];
		if (item == nullptr) continue;

		// Calculate weapon values based on all equipped weapons
		if (item->get_equip_type() == ET_WEAPON)
		{
			if (item->get_item_size() == IS_LARGE)
			{
				large_weapon = true;
				small_weapons = false;
			}
			else if (item->get_item_size() != IS_SMALL)
				small_weapons = false;

			// Calculate die size in a weird way I'm not sure I agree with anymore
			if (weapon_values[1] == 0 || item->get_value(1) <= weapon_values[1])
			{
				if (weapon_values[1] != 0)
					weapon_values[2] += weapon_values[0];

				if (small_weapons) // Dual-wielding small weapons is more efficient
					weapon_values[1] += item->get_value(1);
				else weapon_values[1] = item->get_value(1);
			}
			else weapon_values[2] += item->get_value(0);

			weapon_values[0] += item->get_value(0); // Sum together dice amounts
			weapon_values[2] += item->get_value(2); // Sum together extra damage

			weapons += 1;
		}
		// Calculate armor values based on all equipped protective items
		else if (
			item->get_equip_type() == ET_ARMOR || item->get_equip_type() == ET_BOOTS ||
			item->get_equip_type() == ET_CLOAK || item->get_equip_type() == ET_GLOVES ||
			item->get_equip_type() == ET_HAT   || item->get_equip_type() == ET_SHIELD
		){
			if (item->get_value(0) > 0)
				total_armor += item->get_value(0);
		}
		// Calculate visibility range based on equipped light sources
		else if (item->get_equip_type() == ET_LIGHT)
		{
			const uint8_t light_strength = std::min(item->get_value(0), item->get_durability());
			if (light_strength > visibility_range)
				visibility_range = light_strength;
		}
	}
	if (weapons > 1 && large_weapon) // Dual-wielding a large weapon is really bad
	{
		weapon_values[1] /= 2;
		weapon_values[2] = 0;
	}
}
std::pair<uint8_t, bool> Player::get_melee_damage() const
{
	uint8_t damage = (skills[A_STR] > 1) ? skills[A_STR] : 1;
	for (uint8_t i = 0; i < weapon_values[0]; i++)
	{
		if (weapon_values[1] > 0) // Just in case we somehow arrive here before update_item_values()
			damage += (engine.get_rng() % weapon_values[1]) + 1;
	}
	bool crit = false;
	if (engine.get_rng() % 10 == 0)
	{
		crit = true;
		damage *= 2;
	}
	damage + weapon_values[2]; // Extra damage gets added after crit multiplication
	return std::make_pair(damage, crit);
}
uint8_t Player::get_armor_value() const
{
	if (total_armor < 90)
		return total_armor;
	else return 90;
}
