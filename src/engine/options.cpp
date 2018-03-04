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
#include "options.hpp"

#include "camera.hpp"
#include "scene_manager.hpp"
#include "sound_manager.hpp"
#include "bitmap_font.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <fstream> // for std::ifstream
#include <algorithm> // for std::remove

Options options;

Options::Options()
{

}
Options::~Options()
{
	options_b.clear();
	options_i.clear();
	options_s.clear();
}
void Options::init()
{
	options_b.clear();
	options_i.clear();
	options_s.clear();

	options_b["display-fullscreen"] = false;
	options_b["display-borderless"] = false;
	options_i["display-width"] = 1024;
	options_i["display-height"] = 640;
	options_i["display-fps_cap"] = 60;
	options_b["display-vsync"] = true;

	options_i["camera-scroll_speed"] = 40;
	options_i["camera-follow_speed"] = 20;
	options_b["camera-apply_shake"] = true;

	options_i["sound-music_volume"] = 80;

	options_s["ui-image"] = "bg_red";
	options_s["ui-font"] = "font";
}
void Options::load()
{
	init(); // Setup defaults

	std::string line, category;
	std::ifstream options_file(engine.get_base_path() + "../options.ini");

	if (options_file.is_open())
	{
		while (std::getline(options_file, line))
		{
			// Lines starting with a square bracket declare the category
			if (line[0] == '[')
			{
				category = line.substr(1, line.length() - 2);
			}
			else if (line[0] != '\n' && line[0] != ';') // Other lines define the options
			{
				// Remove all spaces and find the delimiter
				const std::string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
				line.erase(end_pos, line.end());
				std::size_t split = line.find('=');

				if (split != std::string::npos)
				{
					// Split the line into key and value
					const char value_type = line[0];
					const std::string key = line.substr(2, split - 2);
					std::string value = line.substr(split + 1);

					// Remove possible comment at the end of the line
					split = value.find(';');
					if (split != std::string::npos)
						value = value.substr(0, split);

					std::cout << "setting option '" << category << "-" << key << "': " << value << std::endl;
					if (value_type == 'b')
						set_b(category + "-" + key, std::stoi(value));
					else if (value_type == 'i')
						set_i(category + "-" + key, std::stoi(value));
					else if (value_type == 's')
						set_s(category + "-" + key, value);
					else std::cout << "invalid option identifier '" << value_type << "_'!" << std::endl;
				}
			}
		}
		options_file.close();
		apply(); // Apply any major changes immediately
	}
	else std::cout << "could not find 'options.ini'!" << std::endl;
}
void Options::apply()
{
	// Make sure the options are reasonable

	if (options_i["display-width"] < 640) options_i["display-width"] = 640;
	if (options_i["display-height"] < 360) options_i["display-height"] = 360;
	if (options_i["display-fps_cap"] < 1) options_i["display-fps_cap"] = 1;

	if (options_i["camera-scroll_speed"] < 1)
		options_i["camera-scroll_speed"] = 1;
	else if (options_i["camera-scroll_speed"] > 99)
		options_i["camera-scroll_speed"] = 99;

	if (options_i["camera-follow_speed"] < 1)
		options_i["camera-follow_speed"] = 1;
	else if (options_i["camera-follow_speed"] > 99)
		options_i["camera-follow_speed"] = 99;

	if (options_i["sound-music_volume"] > 100)
		options_i["sound-music_volume"] = 100;

	if (options_b["display-borderless"])
		SDL_SetWindowBordered(engine.get_window(), SDL_FALSE);
	else SDL_SetWindowBordered(engine.get_window(), SDL_TRUE);

	camera.set_scroll_speed(options_i["camera-scroll_speed"] * 0.01f);
	camera.set_follow_speed(options_i["camera-follow_speed"] * 0.01f);
	camera.set_window_size(options_i["display-width"], options_i["display-height"]);
	camera.set_window_fullscreen(options_b["display-fullscreen"]);

	if (engine.get_sound_manager() != nullptr)
		engine.get_sound_manager()->set_music_volume(options_i["sound-music_volume"]);
}
const bool& Options::get_b(const std::string &option)
{
	if (options_b.find(option) != options_b.end())
		return options_b[option];

	std::cout << "could not get option '" << option << "'!" << std::endl;
	return false;
}
const int16_t& Options::get_i(const std::string &option)
{
	if (options_i.find(option) != options_i.end())
		return options_i[option];

	std::cout << "could not get option '" << option << "'!" << std::endl;
	return 0;
}
const std::string& Options::get_s(const std::string &option)
{
	if (options_s.find(option) != options_s.end())
		return options_s[option];

	std::cout << "could not get option '" << option << "'!" << std::endl;
	return "err";
}
void Options::set_b(const std::string &option, const bool &value)
{
	options_b[option] = value;
}
void Options::set_i(const std::string &option, const int16_t &value)
{
	options_i[option] = value;
}
void Options::set_s(const std::string &option, const std::string &value)
{
	options_s[option] = value;
}
