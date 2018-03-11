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
#include "texture.hpp"

#include <cstring>

Texture::Texture() : texture_pitch(0), name(""), texture(nullptr), texture_pixels(nullptr)
{

}
Texture::~Texture()
{
	free();
}
void Texture::free()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}
void Texture::render(int16_t x, int16_t y, const SDL_Rect *clip, uint8_t scale, SDL_RendererFlip flip, double angle) const
{
	SDL_Rect quad = { x, y, texture_width, texture_height };
	if (clip != nullptr)
	{
		quad.w = clip->w;
		quad.h = clip->h;
	}
	if (scale != 1)
	{
		quad.w *= scale;
		quad.h *= scale;
	}
	SDL_RenderCopyEx(engine.get_renderer(), texture, clip, &quad, angle, nullptr, flip);
}
bool Texture::load_from_file(const std::string &path, bool greyscale, bool outline)
{
	free();
	const std::string fullpath = engine.get_base_path() + path;

	SDL_Texture *new_texture = nullptr;
	SDL_Surface *loaded_surface = IMG_Load(fullpath.c_str());

	if (loaded_surface == NULL)
	{
		std::cout << "unable to load image '" << path << "'! SDL_image Error: " << IMG_GetError() << std::endl;
		return false;
	}
	SDL_Surface *formatted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA8888, 0);
	if (formatted_surface == NULL)
	{
		SDL_FreeSurface(loaded_surface);
		std::cout << "unable to convert loaded surface to display format! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	new_texture = SDL_CreateTexture(engine.get_renderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formatted_surface->w, formatted_surface->h);
	if (new_texture == NULL)
	{
		SDL_FreeSurface(formatted_surface);
		SDL_FreeSurface(loaded_surface);
		std::cout << "unable to create blank texture! SDL Error: " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_SetTextureBlendMode(new_texture, SDL_BLENDMODE_BLEND);
	SDL_LockTexture(new_texture, &formatted_surface->clip_rect, &texture_pixels, &texture_pitch);
	std::memcpy(texture_pixels, formatted_surface->pixels, formatted_surface->pitch * formatted_surface->h);

	texture_width = formatted_surface->w;
	texture_height = formatted_surface->h;

	if (greyscale) // Apply manual transparency to greyscale textures
	{
		uint32_t *pixels = (uint32_t*)texture_pixels;
		const uint16_t pixel_count = (texture_pitch / 4) * texture_height;
		const uint32_t color_key = SDL_MapRGB(formatted_surface->format, 25, 25, 25);
		const uint32_t outline_key = SDL_MapRGB(formatted_surface->format, 0, 0, 0);
		const uint32_t transparent = SDL_MapRGBA(formatted_surface->format, 0, 0, 0, 0);

		for (uint16_t i = 0; i < pixel_count; ++i)
		{
			if (pixels[i] == color_key || (!outline && pixels[i] == outline_key))
				pixels[i] = transparent;
		}
	}
	SDL_UnlockTexture(new_texture);
	SDL_FreeSurface(formatted_surface);
	SDL_FreeSurface(loaded_surface);

	texture = new_texture;
	name = path;

	return true;
}
bool Texture::lock_texture()
{
	if (texture_pixels == nullptr)
		return false;
	else if (SDL_LockTexture(texture, nullptr, &texture_pixels, &texture_pitch) != 0)
		return false;
	return true;
}
bool Texture::unlock_texture()
{
	if (texture_pixels != nullptr)
	{
		SDL_UnlockTexture(texture);
		texture_pixels = nullptr;
		texture_pitch = 0;
		return true;
	}
	else return false;
}
