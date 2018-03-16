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
#include "sound.hpp"

#include "logging.hpp"

Sound::Sound() : name("???"), sound_file(nullptr), music_file(nullptr)
{

}
Sound::~Sound()
{
	free();
}
void Sound::free()
{
	if (sound_file != nullptr)
	{
		Mix_FreeChunk(sound_file);
		sound_file = nullptr;
	}
	if (music_file != nullptr)
	{
		Mix_FreeMusic(music_file);
		music_file = nullptr;
	}
}
void Sound::play(int8_t channel, int8_t repeat)
{
	if (sound_file != nullptr)
		Mix_PlayChannel(channel, sound_file, repeat);

	if (music_file != nullptr)
	{
		if (Mix_PlayMusic(music_file, repeat) < 0)
			logging.cerr(std::string("Could not play music! SDL_mixer Error: ") + Mix_GetError(), LOG_SOUND);
	}
}
bool Sound::load_from_file(const std::string &path, bool music)
{
	free();
	const std::string full_path = engine.get_base_path() + "sound/" + path;

	if (!music) // This same class is used to load and play either music files or sound effects
	{
		sound_file = Mix_LoadWAV(full_path.c_str());
		if (sound_file == NULL)
		{
			logging.cerr(std::string("Could not load sound: '") + path + "'! SDL_mixer Error: " + Mix_GetError(), LOG_SOUND);
			sound_file = nullptr;
			return false;
		}
	}
	else // The default for this function is a sound file, down here is where we load music files
	{
		music_file = Mix_LoadMUS(full_path.c_str());
		if (music_file == NULL)
		{
			logging.cerr(std::string("Could not load music: '") + path + "'! SDL_mixer Error: " + Mix_GetError(), LOG_SOUND);
			music_file = nullptr;
			return false;
		}
	}
	name = path;
	return true;
}
void Sound::fade_in(uint16_t ms)
{
	if (music_file != nullptr)
		Mix_FadeInMusic(music_file, 1, ms);
}
