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
#include "message_box.hpp"

#include "camera.hpp"
#include "bitmap_font.hpp"
#include "ui.hpp"

MessageBox::MessageBox() : box_title("???"), box_message("..."), box_background(nullptr)
{

}
MessageBox::~MessageBox()
{
	free();
}
void MessageBox::free()
{
	if (box_background != nullptr)
	{
		SDL_DestroyTexture(box_background);
		box_background = nullptr;
	}
}
bool MessageBox::init(const std::string &title, const std::string &message, uint16_t xpos, uint16_t ypos)
{
	free();

	box_title = title;
	box_message = message;

	/*x = xpos; y = ypos; width = 9; height = 5;
	box_title = title; box_message = message;

	if (x == 0) x = (camera.get_cam_w() / 2) - ((width * 32) / 2);
	if (y == 0) y = 16;// (camera.get_cam_h() / 2) - ((height * 32) / 2);

	box_background = SDL_CreateTexture(engine.get_renderer(),
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width * 32, height * 32
	);
	if (box_background == NULL)
	{
		box_background = nullptr;
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_SetRenderTarget(engine.get_renderer(), box_background);

	SDL_SetTextureBlendMode(box_background, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(engine.get_renderer(), 0, 0, 0, 0);
	SDL_RenderClear(engine.get_renderer());
	SDL_SetRenderDrawColor(engine.get_renderer(), 20, 12, 28, 255);

	//ui.draw_box(0, 0, width, height);

	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
	ui.get_bitmap_font()->set_scale(3);
	ui.get_bitmap_font()->render_text(((width * 32) / 2) - ((box_title.length() * 16) / 2), 16, box_title);
	ui.get_bitmap_font()->set_scale(2);
	ui.get_bitmap_font()->render_text(((width * 32) / 2) - ((box_message.length() * 16) / 2), 32, box_message);
	ui.get_bitmap_font()->set_scale(1);

	SDL_SetRenderTarget(engine.get_renderer(), NULL);*/
	return true;
}
void MessageBox::render() const
{
	/*if (box_background != nullptr)
	{
		const SDL_Rect clip = { 0, 0, width * 32, height * 32 };
		const SDL_Rect quad = { x, y, width * 32, height * 32 };

		SDL_RenderCopyEx(engine.get_renderer(), box_background, &clip, &quad, 0.0, nullptr, SDL_FLIP_NONE);
	}*/
	ui.get_bitmap_font()->set_color(COLOR_PEPPERMINT);
	ui.get_bitmap_font()->set_scale(3);
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - ((box_title.length() * 24) / 2), 64, box_title);
	ui.get_bitmap_font()->set_scale(2);
	uint8_t message_length = box_message.length();
	for (char c : box_message)
	{
		if (c == '%')
			message_length -= 2;
	}
	ui.get_bitmap_font()->render_text((camera.get_cam_w() / 2) - ((message_length * 16) / 2), 112, box_message);
	ui.get_bitmap_font()->set_scale(1);
}
bool MessageBox::get_overlap(int16_t mouse_x, int16_t mouse_y) const
{
	return true;// (mouse_x > x && mouse_x < x + (width * 32) && mouse_y > y && mouse_y < y + (height * 32));
}
bool MessageBox::get_click(int16_t mouse_x, int16_t mouse_y) const
{
	return (mouse_x > 160 && mouse_x < camera.get_cam_w() - 160 && mouse_y < 80);
}
