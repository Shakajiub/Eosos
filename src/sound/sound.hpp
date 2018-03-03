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

#ifndef SOUND_HPP
#define SOUND_HPP

class Sound
{
public:
	Sound();
	~Sound();

	void free();
	void play(int8_t channel = -1, int8_t repeat = 0);

	void fade_in(uint16_t ms = 1000) { if (music_file != nullptr) Mix_FadeInMusic(music_file, 1, ms); }
	//void fade_out(uint16_t ms) { if (music_file != nullptr) Mix_FadeOutMusic(ms); }

	bool load_from_file(const std::string &path, bool music = false);

	std::string get_name() const { return name; }
	//bool get_is_music() const { return music_file != nullptr; }

private:
	std::string name;

	Mix_Chunk *sound_file;
	Mix_Music *music_file;
};

#endif // SOUND_HPP
