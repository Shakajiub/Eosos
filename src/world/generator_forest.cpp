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
#include "generator_forest.hpp"
#include "actor_manager.hpp"
#include "level.hpp"
#include "astar.hpp"

#include "hero.hpp"
#include "monster.hpp"
#include "mount.hpp"
#include "sound_manager.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <unordered_map>

GeneratorForest::GeneratorForest() :
	pathfinder(nullptr), wave_class(WAVE_NONE), wave_boss(MONSTER_NONE), boss_name("???"), boss_desc("???")
{

}
GeneratorForest::~GeneratorForest()
{
	free();
}
void GeneratorForest::free()
{
	if (pathfinder != nullptr)
	{
		delete pathfinder;
		pathfinder = nullptr;
	}
	spawn_positions.clear();
}
void GeneratorForest::init()
{
	pathfinder = new AStar;
	pathfinder->init();
}
void GeneratorForest::render_ui()
{
	ui.get_bitmap_font()->render_text(16, 16, "Level: " + std::to_string(current_depth));
	ui.get_bitmap_font()->render_text(16, 27, "Wave: " + std::to_string(current_wave));
}
const std::string GeneratorForest::generate(uint8_t depth)
{
	const int8_t offset_x[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
	const int8_t offset_y[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };

	width = 25; height = 15;
	uint8_t map_data[width * height];
	base_pos = std::make_pair(5, 7 + ((engine.get_rng() % 5) - 2));

	calm_timer = 3;
	spawned_mobs = 0;
	current_wave = 0;
	current_turn = 0;
	current_depth = depth;
	wave_monsters.clear();

	bool map_fine = false;
	while (!map_fine)
	{
		spawn_positions.clear();
		std::fill(map_data, map_data + (width * height), 1);

		uint8_t floor_num = 1;
		uint8_t xpos = base_pos.first;
		uint8_t ypos = base_pos.second;
		map_data[ypos * width + xpos] = 0;

		while (floor_num < 180)
		{
			if (xpos == width - 1)
			{
				spawn_positions.push_back(std::make_pair(xpos, ypos));
				xpos = base_pos.first;
				ypos = base_pos.second;
				map_fine = true;
			}
			else if (xpos == 0 || ypos == 0 || ypos == height - 1)
			{
				xpos = base_pos.first;
				ypos = base_pos.second;
			}
			const uint8_t dir = engine.get_rng() % 4;
			xpos += offset_x[dir];
			ypos += offset_y[dir];

			if (map_data[ypos * width + xpos] == 1)
			{
				map_data[ypos * width + xpos] = 0;
				floor_num += 1;
			}
		}
		if (!map_fine)
			std::cout << "map discarded, no access to right edge" << std::endl;
	}
	const std::string woods = (depth % 2 == 0) ? "spruce_dead" : "oak_dead";
	const std::string bases[4] = { "base_camp", "base_outpost", "base_garrison", "base_fort" };
	std::string level =
		"l-0-core/texture/level/floor/dark2_base.png\n"
		"l-1-core/texture/level/floor/dark2_grass.png\n"
		"l-2-core/texture/level/floor/dark2_hill.png\n"
		"l-3-core/texture/level/floor/dark2_field.png\n"
		"l-M-core/texture/level/hill/dark_blue.png\n"
		"l-#-core/texture/level/map/road_dark2.png\n";
	level += "l-T-core/texture/level/tree/" + woods +  ".png\n";
	level += "l-B-core/texture/level/map/" + ((depth < 5) ? bases[depth-1] : bases[3]) + ".png\n";

	const uint8_t field_probability = (depth < 5) ? 11 - depth : 7;
	for (uint8_t y = 0; y < height; y++)
	{
		for (uint8_t x = 0; x < width; x++)
		{
			const uint8_t node = map_data[y * width + x];
			bool field = false;

			if (node != 1 && x < 10 && x > 1 && y < 11 && y > 3)
			{
				level += (engine.get_rng() % field_probability == 0) ? '3' : '0';
				field = true;
			}
			else if (x < 15)
				level += (engine.get_rng() % 5 < 2) ? '0' : '1';
			else level += (engine.get_rng() % 4 == 0) ? '1' : '2';

			if (node == 1 && !field)
			{
				if (engine.get_rng() % ((x + 1) * 4) > 20)
					level += 'M';
				else level += 'T';
			}
			else level += '_';
		}
		level += '\n';
	}
	std::cout << level << std::endl;
	return level;
}
void GeneratorForest::post_process(ActorManager *am, Level *level)
{
	if (pathfinder == nullptr)
	{
		pathfinder = new AStar;
		pathfinder->init();
	}
	else pathfinder->clear_path();

	const std::pair<uint8_t, uint8_t> start = spawn_positions[engine.get_rng() % spawn_positions.size()];
	const uint8_t start_x = start.first;
	const uint8_t start_y = start.second;

	spawn_positions.clear();
	spawn_positions.push_back(start);

	auto sub_nodes = level->get_sub_nodes();
	MapNode new_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };
	MapNode base_node = { nullptr, nullptr, nullptr, 0, 0, NT_NONE, false, false };

	new_node.floor_texture = sub_nodes['0'].sub_texture;
	new_node.wall_texture = sub_nodes['#'].sub_texture;
	new_node.wall_animated = sub_nodes['#'].sub_animated;
	new_node.wall_type = sub_nodes['#'].sub_type;

	base_node.floor_texture = sub_nodes['0'].sub_texture;
	base_node.wall_texture = sub_nodes['B'].sub_texture;
	base_node.wall_animated = sub_nodes['B'].sub_animated;
	base_node.wall_type = sub_nodes['B'].sub_type;

	pathfinder->find_path(level, start_x, start_y, base_pos.first, base_pos.second, 0);
	while (pathfinder->get_path_found())
	{
		level->set_node(pathfinder->get_goto_x(), pathfinder->get_goto_y(), new_node);
		pathfinder->step();
	}
	level->set_node(start_x, start_y, new_node);
	level->set_node(base_pos.first, base_pos.second, base_node);

	const std::string crops[6] = { "1", "2", "3", "4", "5", "6" };
	MapNode temp_node;

	for (uint8_t y = 0; y < height; y++)
	{
		for (uint8_t x = 0; x < width; x++)
		{
			if (x == base_pos.first && y == base_pos.second)
				continue;

			temp_node = level->get_node(x, y);
			if (am != nullptr && temp_node.floor_texture->get_name().find("_field") != std::string::npos)
			{
				if (engine.get_rng() % 4 != 0)
				{
					const std::string crop_name = "core/texture/level/decor/crop_" + crops[engine.get_rng() % 6] + ".png";
					am->spawn_actor(level, ACTOR_PROP, x, y, crop_name);
				}
				else am->spawn_actor(level, ACTOR_MOUNT, x, y, "core/texture/actor/sheep_white.png");
			}
		}
	}
}
void GeneratorForest::next_turn(ActorManager *am, Level *level)
{
	if (current_wave > current_depth + 1)
		return;

	current_turn += 1;
	if (calm_timer > 0)
	{
		if (current_turn == 1)
		{
			if (current_depth < 4)
			{
				Actor *hero = am->spawn_actor(level, ACTOR_HERO, base_pos.first, base_pos.second, "core/texture/actor/orc_peon.png");
				if (current_depth > 1)
					ui.spawn_message_box("", "A new Peon has joined the party");
			}
		}
		else if (current_turn == 2)
			ui.clear_message_box();

		calm_timer -= 1;
		if (calm_timer == 0)
			init_wave();
	}
	else if (am != nullptr && current_turn % 2 != 0)
	{
		auto pos = get_spawn_pos();
		Actor *monster = am->spawn_actor(level, ACTOR_MONSTER, pos.first, pos.second);

		if (monster != nullptr && wave_monsters.size() > 1)
		{
			const uint8_t i = (engine.get_rng() % wave_monsters.size());
			MonsterClass mc = (MonsterClass)wave_monsters[i];

			if (current_wave == current_depth + 1 && spawned_mobs == (current_wave * 5 + 4))
			{
				mc = (MonsterClass)wave_boss;
				ui.spawn_message_box("BOSS", boss_name);
				if (ui.get_message_log() != nullptr)
					ui.get_message_log()->add_message(boss_desc, COLOR_MAIZE);
				engine.get_sound_manager()->set_playlist(PT_BOSS);
			}
			else if (mount_name.length() > 0 && engine.get_rng() % 20 == 0)
			{
				Actor *mount = am->spawn_actor(level, ACTOR_MOUNT, pos.first, pos.second, mount_name);
				if (mount != nullptr)
				{
					level->set_actor(mount->get_grid_x(), mount->get_grid_y(), nullptr);
					monster->set_mount(dynamic_cast<Mount*>(mount));
				}
			}
			dynamic_cast<Monster*>(monster)->init_class(mc);
			spawned_mobs += 1;
		}
		if (spawned_mobs >= (current_wave * 5 + 5))
		{
			calm_timer = 10;
		}
	}
	else if (current_turn == 2)
		ui.clear_message_box();
}
std::pair<uint8_t, uint8_t> GeneratorForest::get_spawn_pos() const
{
	if (spawn_positions.size() == 0)
		return std::make_pair(0, 0);
	return spawn_positions[engine.get_rng() % spawn_positions.size()];
}
void GeneratorForest::init_wave()
{
	spawned_mobs = 0;
	current_turn = 0;
	current_wave += 1;

	if (current_wave > current_depth + 1)
		return;

	if (current_wave == 1)
	{
		if (current_depth == 1) // Tier 1 waves
		{
			wave_class = WAVE_PEST;
			wave_monsters = {
				MONSTER_PEST_ANT, MONSTER_PEST_ANT, MONSTER_PEST_ANT,
				MONSTER_PEST_BEE
			};
			wave_boss = MONSTER_PEST_SCORPION;
			boss_name = "You must defeat it before it reaches your base!";
			boss_desc = "The Humongous Scorpion has arrived!";
			mount_name = "";
		}
		else if (current_depth == 2) // Tier 2 waves
		{
			wave_class = WAVE_KOBOLD;
			wave_boss = MONSTER_KOBOLD_DEMONIAC;
			boss_name = "Kobold Demoniac";
			boss_desc = "The Kobold Demoniac has arrived!";
			mount_name = "core/texture/actor/ape.png";
		}
		else if (current_depth == 3) // Tier 3 waves
		{
			wave_class = WAVE_DWARF;
			wave_monsters = {
				MONSTER_DWARF_WARRIOR, MONSTER_DWARF_WARRIOR, MONSTER_DWARF_WARRIOR,
				MONSTER_DWARF_WARRIOR, MONSTER_DWARF_WARRIOR, MONSTER_DWARF_WARRIOR,
				MONSTER_DWARF_BEASTMASTER, MONSTER_DWARF_BEASTMASTER,
				MONSTER_DWARF_NECROMANCER
			};
			wave_boss = MONSTER_DWARF_KING;
			boss_name = "The Dwarven King";
			boss_desc = "The Dwarven King has arrived!";
			mount_name = "core/texture/actor/griffin.png";
		}
		else // Final waves
		{
			wave_class = WAVE_DEMON;
			wave_boss = MONSTER_PLATINO;
			boss_name = "The ancient dragon has had enough of you.";
			boss_desc = "The ancient dragon, Platino, has arrived!";
			mount_name = "core/texture/actor/toad.png";
		}
	}
	if (wave_class == WAVE_PEST)
	{
		//engine.get_sound_manager()->set_playlist(PT_PEST);
	}
	else if (wave_class == WAVE_KOBOLD)
	{
		if (current_wave == 1)
			wave_monsters = {
				MONSTER_KOBOLD_WARRIOR, MONSTER_KOBOLD_WARRIOR, MONSTER_KOBOLD_WARRIOR, MONSTER_KOBOLD_WARRIOR,
				MONSTER_KOBOLD_ARCHER, MONSTER_KOBOLD_ARCHER,
				MONSTER_KOBOLD_MAGE
			};
		else wave_monsters = {
				MONSTER_KOBOLD_WARRIOR, MONSTER_KOBOLD_WARRIOR, MONSTER_KOBOLD_WARRIOR,
				MONSTER_KOBOLD_ARCHER, MONSTER_KOBOLD_ARCHER, MONSTER_KOBOLD_ARCHER,
				MONSTER_KOBOLD_MAGE
			};
		engine.get_sound_manager()->set_playlist(PT_KOBOLD);
	}
	else if (wave_class == WAVE_DWARF)
	{
		engine.get_sound_manager()->set_playlist(PT_DWARF);
	}
	else if (wave_class == WAVE_DEMON)
	{
		if (current_wave == 1)
			wave_monsters = {
				MONSTER_DEMON_RED, MONSTER_DEMON_RED, MONSTER_DEMON_RED,
				MONSTER_DEMON_HORNED,
				MONSTER_DEMON_FLYING
			};
		else if (current_wave == 2)
			wave_monsters = {
				MONSTER_DEMON_RED, MONSTER_DEMON_RED,
				MONSTER_DEMON_HORNED, MONSTER_DEMON_HORNED,
				MONSTER_DEMON_FLYING,
				MONSTER_DEMON_FIRE
			};
		else if (current_wave == 3)
			wave_monsters = {
				MONSTER_DEMON_RED, MONSTER_DEMON_RED,
				MONSTER_DEMON_HORNED, MONSTER_DEMON_HORNED,
				MONSTER_DEMON_FLYING,
				MONSTER_DEMON_FIRE, MONSTER_DEMON_FIRE,
				MONSTER_DEMON_PLATINUM
			};
		else wave_monsters = {
				MONSTER_DEMON_RED,
				MONSTER_DEMON_HORNED, MONSTER_DEMON_HORNED,
				MONSTER_DEMON_FLYING, MONSTER_DEMON_FLYING,
				MONSTER_DEMON_FIRE, MONSTER_DEMON_FIRE,
				MONSTER_DEMON_PLATINUM
			};
		engine.get_sound_manager()->set_playlist(PT_DEMON);
	}
	ui.spawn_message_box("Wave #" + std::to_string(current_wave), "placeholder text");
}
