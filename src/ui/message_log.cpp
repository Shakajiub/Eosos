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
#include "message_log.hpp"

#include "logging.hpp"
#include "options.hpp"
#include "texture.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

#include <algorithm> // for std::min

MessageLog::MessageLog() : log_texture(nullptr)
{

}
MessageLog::~MessageLog()
{
	free();
}
void MessageLog::free()
{
	if (log_texture != nullptr)
	{
		SDL_DestroyTexture(log_texture);
		log_texture = nullptr;
	}
}
void MessageLog::init()
{
	free();

	log_texture = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width * 32, height * 32
	);
	if (log_texture == NULL)
	{
		log_texture = nullptr;
		logging.cerr(std::string("Unable to create blank texture! SDL Error: ") + SDL_GetError(), LOG_TEXTURE);
		return;
	}
	refresh_texture();
}
void MessageLog::render() const
{
	if (log_texture != nullptr)
	{
		const SDL_Rect clip = { 0, 0, width * 32, height * 32 };
		const SDL_Rect quad = { x, y, width * 32, height * 32 };

		SDL_RenderCopyEx(engine.get_renderer(), log_texture, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}
}
void MessageLog::add_message(const std::string &message, SDL_Color color)
{
	std::string final_message = "> " + message + " ";
	const uint8_t max_width = ((width * 32) / ui.get_bitmap_font()->get_width()) - 2;

	if (final_message.length() > max_width) // If the message is too long for the log, split it into multiple
	{
		bool text_split = false;
		uint8_t space_pos = 0;
		uint8_t prev_pos = 0;

		while (!text_split)
		{
			prev_pos = space_pos;
			space_pos = (uint8_t)final_message.find(' ', prev_pos + 1); // Search for the next space

			if (final_message.length() <= max_width || space_pos == 255) // The message is now short enough
			{
				Message new_msg = { final_message, color };
				message_log.push_back(new_msg);
				text_split = true;
			}
			else if (space_pos > max_width) // If a space is found after the max length, split from the PREVIOUS space
			{
				Message new_msg = { final_message.substr(0, prev_pos), color };
				message_log.push_back(new_msg);

				final_message = final_message.substr(prev_pos + 1, final_message.length() - (prev_pos + 1));
				space_pos = 0;
			}
		}
	}
	else // The message is short enough to fit in the log
	{
		Message new_msg = { final_message, color };
		message_log.push_back(new_msg);
	}
	if (message_log.size() > (size_t)(max_messages * 5)) // Make sure the log doesn't get too long
	{
		std::vector<Message> cleaned_up(message_log.end() - max_messages, message_log.end());
		message_log.clear(); message_log = cleaned_up;
	}
	if (log_texture != nullptr)
		refresh_texture();
}
void MessageLog::clear_log()
{
	message_log.clear();
	if (log_texture != nullptr)
		refresh_texture();
}
void MessageLog::set_size(uint8_t new_width, uint8_t new_height)
{
	width = new_width;
	height = new_height;

	max_messages = ((height * 32) - 16) / ui.get_bitmap_font()->get_height();
}
void MessageLog::refresh_texture()
{
	SDL_SetRenderTarget(engine.get_renderer(), log_texture);

	//ui.draw_box(0, 0, width, height);

	// Fill the texture with "nothing" (full transparency)
	SDL_SetTextureBlendMode(log_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(engine.get_renderer(), 0, 0, 0, 0);
	SDL_RenderClear(engine.get_renderer());
	SDL_SetRenderDrawColor(engine.get_renderer(), 20, 12, 28, 255);

	const uint8_t real_width = (width * 32) - 32;
	const uint8_t real_height = (height * 32) - 32;

	if (message_log.size() != 0)
	{
		int16_t render_y = (real_height + 24) - ui.get_bitmap_font()->get_height();
		for (uint8_t i = (uint8_t)message_log.size(); i > message_log.size() - (std::min(message_log.size(), (size_t)max_messages)); i--)
		{
			ui.get_bitmap_font()->set_color(message_log[i-1].color);
			ui.get_bitmap_font()->render_text(12, render_y, message_log[i-1].text);
			render_y -= ui.get_bitmap_font()->get_height();
		}
	}
	SDL_SetRenderTarget(engine.get_renderer(), NULL);
}
