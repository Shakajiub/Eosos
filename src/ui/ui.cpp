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
#include "ui.hpp"
#include "texture.hpp"
#include "level_up_box.hpp"
#include "widget.hpp"
#include "bitmap_font.hpp"
#include "message_box.hpp"
#include "message_log.hpp"

#include "actor_manager.hpp"
#include "camera.hpp"
#include "logging.hpp"
#include "options.hpp"
#include "texture_manager.hpp"

template Widget* UI::spawn_widget<LevelUpBox>(const std::string &widget_name);

UI ui;

UI::UI() :
	mb_lock(false), capture_input(false), ui_background(nullptr),
	main_font(nullptr), message_log(nullptr), message_box(nullptr)
{

}
UI::~UI()
{
	free();
}
void UI::free()
{
	if (ui_background != nullptr)
	{
		engine.get_texture_manager()->free_texture(ui_background->get_name());
		ui_background = nullptr;
	}
	if (main_font != nullptr)
	{
		delete main_font;
		main_font = nullptr;
	}
	if (message_log != nullptr)
	{
		delete message_log;
		message_log = nullptr;
	}
	if (message_box != nullptr)
	{
		delete message_box;
		message_box = nullptr;
	}
	while (!message_queue.empty())
	{
		MessageBox *mb = message_queue.front();
		message_queue.pop();
		delete mb;
	}
	widget_map.clear();
	logging.cout("All widgets erased", LOG_UI);
}
void UI::update()
{
	/*if (message_box == nullptr && !message_queue.empty())
	{
		message_box = message_queue.front();
		message_queue.pop();
	}*/
}
void UI::render() const
{
	if (message_log != nullptr)
		message_log->render();

	for (auto widget : widget_map)
		widget.second.get()->render();

	if (message_box != nullptr)
		message_box->render();
}
void UI::init_background()
{
	if (ui_background != nullptr)
	{
		engine.get_texture_manager()->free_texture(ui_background->get_name());
		ui_background = nullptr;
	}
	ui_background = engine.get_texture_manager()->load_texture(
		"ui/" + options.get_s("ui-image") + ".png"
	);
}
bool UI::init_bitmap_font()
{
	main_font = new BitmapFont;
	if (!main_font->build("ui/" + options.get_s("ui-font") + ".png"))
		return false;
	return true;
}
void UI::init_message_log()
{
	if (message_log != nullptr)
		delete message_log;

	message_log = new MessageLog;
	message_log->set_position(0, camera.get_cam_h() - 128);
	message_log->set_size(25, 4);
	message_log->init();
	//message_log->add_message("Welcome to HELL.", DAWN_BERRY);
}
template <class T>
Widget* UI::spawn_widget(const std::string &widget_name)
{
	auto it = widget_map.find(widget_name);
	if (it == widget_map.end())
	{
		widget_map[widget_name] = std::make_shared<T>();
		widget_map[widget_name]->set_name(widget_name);
		return widget_map[widget_name].get();
	}
	logging.cerr(std::string("Warning! Attempting to spawn a second '") + widget_name + "' widget!", LOG_UI);
	return nullptr;
}
Widget* UI::get_widget(const std::string &widget_name) const
{
	auto it = widget_map.find(widget_name);
	if (it != widget_map.end())
		return it->second.get();
	else return nullptr;
}
bool UI::remove_widget(const std::string &widget_name)
{
	auto it = widget_map.find(widget_name);
	if (it != widget_map.end())
	{
		it->second.reset();
		widget_map.erase(it);
		return true;
	}
	else return false;
}
bool UI::spawn_message_box(const std::string &title, const std::string &message, bool lock)
{
	if (mb_lock)
		return false;
	mb_lock = lock;

	if (message_box != nullptr)
	{
		delete message_box;
		message_box = nullptr;
	}
	MessageBox *mb = new MessageBox;
	if (mb->init(title, message, 0, 0))
	{
		message_box = mb;
		return true;
	}
	delete mb;
	return false;
}
void UI::clear_message_box(bool pass_lock)
{
	if (mb_lock && !pass_lock)
		return;

	while (!message_queue.empty())
	{
		MessageBox *mb = message_queue.front();
		message_queue.pop();
		delete mb;
	}
	delete message_box;

	message_box = nullptr;
	mb_lock = false;
}
void UI::draw_box(uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, bool highlight) const
{
	if (ui_background == nullptr)
		return;

	if (width == 1) // If the width is 1, just draw a vertical "bar"
	{
		for (uint8_t i = 0; i < height + 1; i++)
		{
			SDL_Rect temp_rect = { 48, 0, 16, 16 };
			if (i == width)
				temp_rect.y = 32;
			else if (i != 0)
				temp_rect.y = 16;

			if (highlight || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(0, i * 32, &temp_rect);
		}
		return;
	}
	if (height == 1) // Same for the height, but a horizontal "bar"
	{
		for (uint8_t i = 0; i < width + 1; i++)
		{
			SDL_Rect temp_rect = { 0, 48, 16, 16 };
			if (i == width)
				temp_rect.x = 32;
			else if (i != 0)
				temp_rect.x = 16;

			if (highlight || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(i * 32, 0, &temp_rect);
		}
		return;
	}
	for (uint8_t x = 0; x < width; x++) // Otherwise, just draw the box
	{
		for (uint8_t y = 0; y < height; y++)
		{
			SDL_Rect temp_rect = { 16, 16, 16, 16 };
			if (x == 0)
			{
				if (y == 0) temp_rect = { 0, 0, 16, 16};
				else if (y == height - 1) temp_rect = { 0, 32, 16, 16 };
				else temp_rect = { 0, 16, 16, 16 };
			}
			else if (x == width - 1)
			{
				if (y == 0) temp_rect = { 32, 0, 16, 16};
				else if (y == height - 1) temp_rect = { 32, 32, 16, 16 };
				else temp_rect = { 32, 16, 16, 16 };
			}
			else if (y == 0)
				temp_rect = { 16, 0, 16, 16 };
			else if (y == height - 1)
				temp_rect = { 16, 32, 16, 16 };

			if (highlight)// || options.get_b("ui-highlight"))
				temp_rect.x += 64;
			ui_background->render(xpos + (x * 32), ypos + (y * 32), &temp_rect);
		}
	}
}
bool UI::get_overlap(int16_t xpos, int16_t ypos)
{
	for (auto widget : widget_map)
	{
		if (widget.second.get()->get_overlap(xpos, ypos))
			return true;
	}
	if (engine.get_actor_manager()->get_overlap(xpos, ypos))
		return true;
	return false;
}
bool UI::get_click(int16_t xpos, int16_t ypos)
{
	for (auto widget : widget_map)
	{
		if (widget.second.get()->get_click(xpos, ypos))
			return true;
	}
	if (message_box != nullptr)
	{
		if (message_box->get_click(xpos, ypos))
		{
			delete message_box;
			message_box = nullptr;
		}
		//return true;
	}
	if (engine.get_actor_manager()->get_click(xpos, ypos))
		return true;
	return false;
}
