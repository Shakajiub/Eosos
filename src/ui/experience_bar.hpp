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

#ifndef EXPERIENCE_BAR_HPP
#define EXPERIENCE_BAR_HPP

class ExperienceBar
{
public:
	ExperienceBar();
	~ExperienceBar();

	void free();
	void init();
	void render() const;

	bool get_overlap(int16_t xpos, int16_t ypos) const;
	void set_position(int16_t xpos, int16_t ypos) { x = xpos; y = ypos; }

private:
	void refresh_texture();

	int16_t x, y;
	SDL_Texture *bar_texture;
};

#endif // EXPERIENCE_BAR_HPP