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

#ifndef CAMERA_HPP
#define CAMERA_HPP

class Camera
{
public:
	Camera();
	~Camera();

	void init();
	void update();

	void update_position(int16_t desired_x, int16_t desired_y, bool jump = false);
	void move_camera(uint8_t direction, uint8_t map_width, uint8_t map_height);

	bool get_in_camera_grid(uint8_t xpos, uint8_t ypos) const;
	int16_t get_cam_x() const { return (int16_t)camera_x; }
	int16_t get_cam_y() const { return (int16_t)camera_y; }
	uint16_t get_cam_w() const { return camera_w; }
	uint16_t get_cam_h() const { return camera_h; }

	void set_locked(bool lock) { locked = lock; }
	void set_free_move(bool move) { free_move = move; }
	void set_scroll_speed(float speed) { scroll_speed = speed; }
	void set_follow_speed(float speed) { follow_speed = speed; }
	void set_window_size(uint16_t width, uint16_t height);
	void set_window_fullscreen(bool fullscreen);

private:
	bool locked;
	bool free_move;

	float scroll_speed;
	float follow_speed;
	float camera_x, camera_y;

	uint16_t camera_w, camera_h;
	int16_t center_x, center_y;
	int16_t offset_x, offset_y;
};
extern Camera camera;

#endif // CAMERA_HPP
