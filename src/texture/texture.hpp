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

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

class Texture
{
public:
	Texture();
	~Texture();

	void free();
	void render(int16_t x, int16_t y, const SDL_Rect *clip = nullptr, uint8_t scale = 2, SDL_RendererFlip flip = SDL_FLIP_NONE, double angle = 0.0) const;

	bool load_from_file(const std::string &path, bool greyscale = false, bool outline = true);

	bool lock_texture();
	bool unlock_texture();

	std::string get_name() const { return name; }
	uint16_t get_width() const { return texture_width; }
	uint16_t get_height() const { return texture_height; }
	uint8_t get_tiles_horizontal() const { return texture_width > 16 ? texture_width / 16 : 1; }
	uint8_t get_tiles_vertical() const { return texture_height > 16 ? texture_height / 16 : 1; }

	void set_color(SDL_Color color) { SDL_SetTextureColorMod(texture, color.r, color.g, color.b); }

private:
	uint16_t texture_width, texture_height;
	int texture_pitch;
	std::string name;

	SDL_Texture *texture;
	void* texture_pixels;
};

#endif // TEXTURE_HPP
