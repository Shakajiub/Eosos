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

#ifndef HERO_HPP
#define HERO_HPP

#include "actor.hpp"

class AStar;

enum HeroClass
{
	HC_PEON,
	HC_BARBARIAN,
	HC_NINJA,
	HC_MAGE,
	HC_JUGGERNAUT
};
class Hero : public Actor
{
public:
	Hero();
	~Hero();

	void free();

	virtual bool init(ActorType at, uint8_t xpos, uint8_t ypos, const std::string &texture_name);
	virtual void update(Level *level);
	virtual void render_ui(uint16_t xpos, uint16_t ypos) const;

	virtual void start_turn();
	virtual bool take_turn(Level *level, ActorManager *am);
	virtual void end_turn();

	virtual uint8_t get_damage() const;
	void clear_status();

	bool init_ui_texture();
	bool init_pathfinder();
	bool init_class(HeroClass hc);

	void reset_moves();
	void step_pathfinder(Level *level);
	void clear_pathfinder();
	void clear_ui_texture();

	void input_keyboard_down(SDL_Keycode key, Level *level);
	void input_mouse_button_down(SDL_Event eve, Level *level);

	HeroClass get_hero_class() const { return hero_class; }
	AStar* get_pathfinder() const { return pathfinder; }
	bool get_auto_move() const;
	bool get_ability_activated() const { return ability_activated; }

	void set_sleep_timer(uint8_t timer);
	void set_ability_activated(bool a) { ability_activated = a; }

private:
	bool auto_move_path;
	bool command_this_turn;
	bool ability_activated;
	bool random_move;

	HeroClass hero_class;

	uint8_t hp_shake;
	uint16_t hb_timer;
	int8_t prev_health;

	AStar *pathfinder;
	SDL_Texture *ui_texture;
	Texture *health_texture;

	uint8_t sleep_timer;
};

#endif // HERO_HPP
