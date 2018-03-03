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

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "actor.hpp"
#include <vector>

class AbilityManager;
class AStar;
class Dijkstra;
class Item;

class Player : public Actor
{
public:
	Player();
	~Player();

	virtual void update(Dungeon *scene);
	virtual void render() const;
	virtual void start_turn();
	virtual bool take_turn(Dungeon *scene);
	virtual void end_turn();

	bool init_class_script(const std::string &script);
	std::string get_class_texture(const std::string &race);

	void init_ability_manager(Dungeon *scene);
	void init_inventory();

	void init_pathfinder(Dungeon *scene);
	void step_pathfinder(Dungeon *scene);

	bool input_keyboard_down(SDL_Keycode key);
	void input_keyboard_up(SDL_Keycode key);
	void input_controller_down(uint8_t index, uint8_t value);
	void input_controller_up();
	void input_mouse_button_down(Dungeon *scene, SDL_Event eve);

	void update_item_values(const std::vector<Item*> &equipped_items);
	virtual std::pair<uint8_t, bool> get_melee_damage() const;
	virtual uint8_t get_armor_value() const;

	AbilityManager* get_ability_manager() const { return ability_manager; }
	AStar* get_pathfinder() const { return pathfinder; }
	Dijkstra* get_dijkstra() const { return dijkstra; }
	Ability* get_ability() const { return activated_ability; }

	lua_State* get_class_script() const { return class_script; }
	std::string get_class_script_name() const { return class_script_name; }

	void set_ability(Ability *ability) { activated_ability = ability; }

private:
	bool auto_move_path;
	bool interact_below;
	uint8_t input_direction;
	int16_t turn_timer;

	uint8_t total_armor;
	std::vector<uint8_t> weapon_values;

	AbilityManager *ability_manager;
	AStar *pathfinder;
	Dijkstra *dijkstra;
	Ability *activated_ability;

	std::string class_script_name;
	lua_State *class_script;
};
extern Player *player;

#endif // PLAYER_HPP
