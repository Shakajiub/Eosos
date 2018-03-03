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

#ifndef ABILITY_HPP
#define ABILITY_HPP

class Actor;
class Button;
class Dungeon;

enum AbilityTarget
{
	AT_NONE, // No target, or the ability user itself
	AT_DIRECTION, // Directly adjacent, or a beam
	AT_ENEMY, // Single enemy in line-of-sight
	AT_FRIENDLY, // Single friendly in line-of-sight
	AT_AREA, // Target a floor square (could be just 1x1)
	AT_PASSIVE // Passive abilities get activated via triggers defined below
};
enum DirectionFilter
{
	DF_NONE, // All non-wall directions are valid
	DF_FLOOR, // Has to target an empty (no wall, actor or object) tile
	DF_WALL, // Has to target an adjacent wall
	DF_ACTOR, // Has to target an adjacent actor
	DF_OBJECT // Has to target an adjacent object
};
enum PassiveTrigger
{
	PT_NONE, // No automatic trigger (should be the only choice for non-passive abilities)
	PT_MOVEMENT // Ability is applied every time the actor moves
};
class Ability
{
public:
	Ability();
	~Ability();

	void free();
	bool init(Dungeon *scene, const std::string &script, SDL_Keycode code);
	void render() const;

	void activate();
	void apply(Actor *source, uint8_t value = 0);
	bool validate_value(uint8_t value);
	void cancel();
	void reduce_cooldown(uint8_t amount = 1);

	bool get_overlap(int16_t xpos, int16_t ypos);
	bool get_click();

	bool get_hovered() const { return ability_rect.x != 0; }
	bool get_activated() const { return activated; }
	uint8_t get_target_type() const { return ability_target; }
	uint8_t get_trigger_type() const { return passive_trigger; }
	std::string get_name() const { return ability_name; }
	SDL_Keycode get_hotkey() const { return hotkey; }

	void set_ui_position(int16_t xpos, int16_t ypos) { x = xpos; y = ypos; }

private:
	bool init_texture(const std::string &icon, SDL_Color color, SDL_Keycode code);
	void init_direction_buttons();
	void update_direction_buttons();

	void set_target_type(uint8_t type);

	bool activated;
	int16_t x, y;

	std::string ability_name;
	std::string ability_desc;
	std::string hotkey_name;
	uint8_t ability_target;
	uint8_t direction_filter;
	uint8_t passive_trigger;

	std::pair<uint8_t, uint8_t> cooldown;

	SDL_Keycode hotkey;
	SDL_Rect ability_rect;
	SDL_Texture *ability_texture;

	Button* buttons[8];
	Dungeon *game_scene;
	std::string script_name;

	lua_State *ability_script;
	SDL_Texture *info_texture;
};

#endif // ABILITY_HPP
