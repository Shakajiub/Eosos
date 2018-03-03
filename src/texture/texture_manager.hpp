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

#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <memory>
#include <unordered_map>

class Texture;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	void free();

	Texture* load_texture(const std::string &texture_name, bool grayscale = false, bool outline = true);
	void free_texture(const std::string &texture_name);

private:
	std::unordered_map<std::string, std::shared_ptr<Texture> > texture_map;
	std::unordered_map<std::string, uint16_t> reference_count;
};

#endif // TEXTUREMANAGER_HPP
