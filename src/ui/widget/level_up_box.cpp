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
#include "level_up_box.hpp"
#include "hero.hpp"
#include "texture.hpp"

#include "camera.hpp"
#include "logging.hpp"
#include "texture_manager.hpp"
#include "ui.hpp"

LevelUpBox::LevelUpBox() : mouse_input(true), temp_hero(nullptr), selection_box(nullptr), selected_option(-1)
{

}
LevelUpBox::~LevelUpBox()
{
	free();
}
void LevelUpBox::free()
{
	if (selection_box != nullptr)
	{
		SDL_DestroyTexture(selection_box);
		selection_box = nullptr;
	}
	for (auto option : level_options)
	{
		if (option.texture != nullptr)
			engine.get_texture_manager()->free_texture(option.texture->get_name());
	}
	level_options.clear();
	temp_hero = nullptr;
}
bool LevelUpBox::init(Hero *hero)
{
	free();

	if (ui.get_background() == nullptr || hero == nullptr)
		return false;

	selection_box = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 96, 48
	);
	if (selection_box == NULL)
	{
		selection_box = nullptr;
		logging.cerr(std::string("Unable to create blank texture! SDL Error: ") + SDL_GetError(), LOG_TEXTURE);
		return false;
	}
	temp_hero = hero;

	if (hero->get_hero_class() == HC_PEON)
	{
		const std::string classes[4] = {
			"actor/orc_barbarian.png",
			"actor/orc_ninja.png",
			"actor/orc_mage.png",
			"actor/orc_juggernaut.png"
		};
		const std::string titles[4] = {
			"Barbarian", "Ninja", "Mage", "Juggernaut"
		};
		const std::string messages[4] = {
			"%7[Damage]%F +1 for every 2 points of health lost",
			"%7[Ranged]%F Attacks from a distance with shuriken",
			"%7[Support]%F Can cast various spells",
			"%7[Tank]%F +1 damage for every 2 unused moves"
		};
		for (uint8_t i = 0; i < 4; i++)
		{
			Texture *temp = engine.get_texture_manager()->load_texture(classes[i]);
			if (temp != nullptr)
			{
				LevelOption option;
				option.overlap = false;
				option.texture = temp;
				option.title = titles[i];
				option.message = messages[i];
				level_options.push_back(option);
			}
		}
		ui.spawn_message_box("Level up!", "Choose class specialization");
	}
	else // Other levels after class specialization just give generic upgrades
	{
		const std::string bonuses[2] = {
			"ui/icon/heart.png",
			"ui/icon/defense.png"
		};
		const std::string titles[2] = {
			"Health",
			"Armor"
		};
		const std::string messages[2] = {
			"Receive +1 Heart",
			"Receive the %AArmored%F buff"
		};
		const SDL_Color colors[2] = {
			DAWN_BERRY,
			DAWN_SKY
		};
		uint8_t it = 0;
		if (hero->get_health().second > 3)
			it = 1;

		for (uint8_t i = it; i < 2; i++)
		{
			Texture *temp = engine.get_texture_manager()->load_texture(bonuses[i], true);
			if (temp != nullptr)
			{
				temp->set_color(colors[i]);
				LevelOption option;
				option.overlap = false;
				option.texture = temp;
				option.title = titles[i];
				option.message = messages[i];
				level_options.push_back(option);
			}
		}
		ui.spawn_message_box("Level up!", "Choose upgrade (you will also heal to max health)");
	}
	SDL_SetRenderTarget(engine.get_renderer(), selection_box);

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
	}
	SDL_SetRenderTarget(engine.get_renderer(), NULL);

	ui.set_capture_input(true);
	widget_activated = true;
	return true;
}
void LevelUpBox::render() const
{
	if (selection_box != nullptr)
	{
		uint16_t render_x = (camera.get_cam_w() / 2) - (level_options.size() * 32) + 8;
		const uint16_t render_y = 144;//camera.get_cam_h() / 2;

		for (uint8_t i = 0; i < level_options.size(); i++)
		{
			if (level_options[i].texture != nullptr)
			{
				const SDL_Rect hero = { 0, 0, 16, 16 };
				const SDL_Rect rect = { level_options[i].overlap ? 48 : 0, 0, 48, 48 };
				const SDL_Rect quad = { render_x, render_y, 48, 48 };

				SDL_RenderCopyEx(engine.get_renderer(), selection_box, &rect, &quad, 0.0, nullptr, SDL_FLIP_NONE);
				level_options[i].texture->render(render_x + 8, render_y + 8, &hero);
				render_x += 64;
			}
		}
	}
}
bool LevelUpBox::input_keyboard_down(SDL_Keycode key)
{
	int8_t prev_option = selected_option;
	bool input = true;
	switch (key)
	{
		case SDLK_LEFT: case SDLK_KP_4: case SDLK_h:
			if (selected_option > 0)
				selected_option -= 1;
			else selected_option = level_options.size() - 1;
			mouse_input = false;
			break;
		case SDLK_RIGHT: case SDLK_KP_6: case SDLK_l:
			if (selected_option < (int8_t)level_options.size() - 1)
				selected_option += 1;
			else selected_option = 0;
			mouse_input = false;
			break;
		case SDLK_RETURN: case SDLK_RETURN2: case SDLK_KP_ENTER:
			if (!mouse_input)
			{
				// TODO - Actual "apply" function, and maybe "move_selection_left" & "move_selection_right"
				return get_click(0, 0);
			}
			break;
		default: input = false; break;
	}
	if (!mouse_input && prev_option >= 0 && prev_option < (int8_t)level_options.size())
		level_options[prev_option].overlap = false;

	if (!mouse_input && selected_option >= 0 && selected_option < (int8_t)level_options.size())
		level_options[selected_option].overlap = true;

	return input;
}
bool LevelUpBox::get_overlap(int16_t mouse_x, int16_t mouse_y)
{
	uint16_t render_x = (camera.get_cam_w() / 2) - (level_options.size() * 32) + 8;
	const uint16_t render_y = 144;//camera.get_cam_h() / 2;

	for (uint8_t i = 0; i < level_options.size(); i++)
	{
		if (mouse_x > render_x && mouse_x < render_x + 48 &&
			mouse_y > render_y && mouse_y < render_y + 48)
		{
			if (!level_options[i].overlap)
			{
				ui.clear_message_box();
				ui.spawn_message_box(level_options[i].title, level_options[i].message);
			}
			if (i > 0) for (uint8_t j = 0; j < i; j++)
				level_options[j].overlap = false;

			level_options[i].overlap = true;
			mouse_input = true;
		}
		else if (mouse_input)
		{
			level_options[i].overlap = false;
		}
		render_x += 64;
	}
	return false;
}
bool LevelUpBox::get_click(int16_t mouse_x, int16_t mouse_y)
{
	for (uint8_t i = 0; i < level_options.size(); i++) if (level_options[i].overlap)
	{
		if (temp_hero != nullptr)
		{
			temp_hero->level_up();
			temp_hero->set_status(STATUS_NONE);
			temp_hero->remove_ability("level-up");

			auto health = temp_hero->get_health();
			if (temp_hero->get_hero_class() == HC_PEON)
			{
				const HeroClass classes[4] = {
					HC_BARBARIAN, HC_NINJA, HC_MAGE, HC_JUGGERNAUT
				};
				temp_hero->init_class(classes[i]);
				health = temp_hero->get_health();
			}
			else // Generic level-up bonuses
			{
				uint8_t it = 0;
				if (health.second > 3)
					it = 1;

				if (i + it == 0) // Health
					health.second += 3;
				else if (i + it == 1) // Armored
					temp_hero->set_status(STATUS_ARMORED);
			}
			temp_hero->set_health_max(health.second);
			temp_hero->set_health(health.second);

			temp_hero->add_action(ACTION_INTERACT, temp_hero->get_grid_x(), temp_hero->get_grid_y());
			//temp_hero->set_turn_done(true);
			temp_hero->set_moves(0);
		}
		ui.clear_message_box();
		ui.remove_widget(widget_name);
		ui.set_capture_input(false);
		return true;
	}
	return false;
}
