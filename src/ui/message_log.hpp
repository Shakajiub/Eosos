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

#ifndef MESSAGE_LOG_HPP
#define MESSAGE_LOG_HPP

#include <vector>

typedef struct
{
	std::string text;
	SDL_Color color;
}
Message;

class MessageLog
{
public:
	MessageLog();
	~MessageLog();

	void free();
	void init();
	void render() const;

	void add_message(const std::string &message, SDL_Color color = COLOR_PEPPERMINT);
	void clear_log();

	void set_position(int16_t xpos, int16_t ypos) { x = xpos; y = ypos; }
	void set_size(uint8_t new_width, uint8_t new_height);

private:
	void refresh_texture();

	int16_t x, y;
	uint8_t width, height;
	uint8_t max_messages;

	std::vector<Message> message_log;
	SDL_Texture *log_texture;
};

#endif // MESSAGE_LOG_HPP
