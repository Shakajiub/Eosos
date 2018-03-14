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
#include "camera.hpp"

#include "options.hpp"

Camera camera;

Camera::Camera() :
	locked(false), free_move(false), scroll_speed(0.0f), follow_speed(0.0f), camera_x(0.0f),
	camera_y(0.0f), camera_w(0), camera_h(0), center_x(0), center_y(0), offset_x(0), offset_y(0)
{

}
Camera::~Camera()
{

}
void Camera::init()
{
	int width, height;
	SDL_GetWindowSize(engine.get_window(), &width, &height);

	camera_w = width; offset_x = camera_w / 2 - 32;
	camera_h = height; offset_y = camera_h / 2 - 32;

	scroll_speed = options.get_i("camera-scroll_speed") * 0.01f;
	follow_speed = options.get_i("camera-follow_speed") * 0.01f;
}
void Camera::update()
{
	if (locked || free_move)
		return;

	float t = engine.get_dt() / 100.0f * follow_speed;
	if (t < 0.01f)      t = 0.01f;
	else if (t > 0.99f) t = 0.99f;

	const float desired_x = (float)(center_x - offset_x);
	const float desired_y = (float)(center_y - offset_y);

	// Basic lerp towards desired location
	camera_x = ((1.0f-t) * camera_x) + (t * (center_x - offset_x));
	camera_y = ((1.0f-t) * camera_y) + (t * (center_y - offset_y));

	if (std::abs(camera_x - desired_x) < 2.0f && std::abs(camera_y - desired_y) < 2.0f)
		free_move = true; // Once we're close enough, just stop updating
}
void Camera::update_position(int16_t desired_x, int16_t desired_y, bool jump)
{
	free_move = false;
	center_x = desired_x;
	center_y = desired_y;

	if (jump) // Pass true here to jump immediately to the new position
	{
		camera_x = (float)(center_x - offset_x);
		camera_y = (float)(center_y - offset_y);
	}
}
void Camera::move_camera(uint8_t direction, uint8_t map_width, uint8_t map_height)
{
	const float movement = engine.get_dt() * scroll_speed;
	switch (direction)
	{
		case 1: if (camera_y - movement > -camera_h) camera_y -= movement; break;
		case 2: if (camera_y + movement < map_height * 32) camera_y += movement; break;
		case 4: if (camera_x - movement > -camera_w) camera_x -= movement; break;
		case 8: if (camera_x + movement < map_width * 32) camera_x += movement; break;
		default: break;
	}
	free_move = true;
}
bool Camera::get_in_camera_grid(uint8_t xpos, uint8_t ypos) const
{
	if (xpos >= camera_x / 32 - 1 && xpos < (camera_x + camera_w) / 32 + 1 &&
		ypos >= camera_y / 32 - 1 && ypos < (camera_y + camera_h) / 32 + 1)
		return true;
	return false;
}
void Camera::set_window_size(uint16_t width, uint16_t height)
{
	SDL_SetWindowSize(engine.get_window(), width, height);
	SDL_SetWindowPosition(engine.get_window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	camera_w = width; offset_x = camera_w / 2 - 32;
	camera_h = height; offset_y = camera_h / 2 - 32;
}
void Camera::set_window_fullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		SDL_DisplayMode dm;
		if (SDL_GetDisplayMode(0, 0, &dm) == 0) // Get fullscreen resolution
			set_window_size(dm.w, dm.h);
		SDL_SetWindowFullscreen(engine.get_window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		SDL_SetWindowFullscreen(engine.get_window(), 0);
		SDL_SetWindowPosition(engine.get_window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
}
