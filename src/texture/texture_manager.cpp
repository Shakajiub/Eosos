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
#include "texture_manager.hpp"
#include "texture.hpp"

#include "logging.hpp"

TextureManager::TextureManager()
{

}
TextureManager::~TextureManager()
{
	free();
}
void TextureManager::free()
{
	texture_map.clear();
	reference_count.clear();

	logging.cout("All textures erased", LOG_TEXTURE);
}
Texture* TextureManager::load_texture(const std::string &texture_name, bool grayscale, bool outline)
{
	auto it = texture_map.find(texture_name);
	if (it == texture_map.end())
	{
		std::shared_ptr<Texture> temp_texture = std::make_shared<Texture>();
		if (!temp_texture->load_from_file(texture_name, grayscale, outline))
		{
			temp_texture.reset();
			return nullptr;
		}
		else texture_map[texture_name] = std::move(temp_texture);
		reference_count[texture_name] = 1;

		logging.cout(std::string("Texture loaded: ") + texture_name, LOG_TEXTURE);
	}
	else reference_count[texture_name] += 1;
	return texture_map[texture_name].get();
}
void TextureManager::free_texture(const std::string &texture_name)
{
	auto it = texture_map.find(texture_name);
	if (it != texture_map.end())
	{
		reference_count[texture_name] -= 1;
		if (reference_count[texture_name] == 0)
		{
			it->second.reset();
			texture_map.erase(it);

			logging.cout(std::string("Texture freed: ") + texture_name, LOG_TEXTURE);
		}
	}
}
