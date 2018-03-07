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

#ifndef UI_HPP
#define UI_HPP

#include <queue>

class ActorManager;
class Texture;
class BitmapFont;
class MessageBox;
class MessageLog;

class UI
{
public:
	UI();
	~UI();

	void free();

	void init_background();
	bool init_bitmap_font();
	void init_message_log();

	bool spawn_message_box(const std::string &title, const std::string &message, uint16_t xpos = 0, uint16_t ypos = 0);

	void update();
	void render() const;

	void draw_box(uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, bool highlight = false) const;

	bool get_overlap(ActorManager *at, int16_t xpos, int16_t ypos) const;
	bool get_click(ActorManager *at, int16_t xpos, int16_t ypos);

	Texture* get_background() const { return ui_background; }
	BitmapFont* get_bitmap_font() const { return main_font; }
	MessageLog* get_message_log() const { return message_log; }

private:
	Texture *ui_background;
	BitmapFont *main_font;
	MessageLog *message_log;

	MessageBox *message_box;
	std::queue<MessageBox*> message_queue;
};
extern UI ui;

#endif // UI_HPP
