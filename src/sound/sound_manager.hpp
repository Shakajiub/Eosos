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

#ifndef SOUNDMANAGER_HPP
#define SOUNDMANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>

class Sound;

enum PlaylistType
{
	PT_MENU,
	PT_WORLD,
	PT_BOSS
};
class SoundManager
{
public:
	SoundManager();
	~SoundManager();

	void free();
	void update();

	void skip_song(uint16_t ms = 500);
	void pause_music(bool msg_log = false);
	void resume_music(bool msg_log = false);
	void stop_music();

	Sound* load_sound(const std::string &sound_name, bool music = false);
	void free_sound(const std::string &sound_name);

	void add_to_playlist(PlaylistType playlist, const std::string &sound_name);
	void remove_from_playlist(PlaylistType playlist, const std::string &sound_name);
	void clear_playlist(PlaylistType playlist);

	bool get_paused() const { return Mix_PausedMusic(); }
	PlaylistType get_playlist() const { return current_playlist; }

	void set_music_volume(uint8_t volume);
	void set_playlist(PlaylistType playlist, bool instant = true);

private:
	uint8_t next_song;
	uint8_t volume_music;
	int16_t silence_timer;

	Sound *current_song;
	PlaylistType current_playlist;
	PlaylistType previous_playlist;

	std::vector<std::string> playlists[3];
	std::unordered_map<std::string, std::shared_ptr<Sound> > sound_map;
	std::unordered_map<std::string, uint16_t> reference_count;
};

#endif // SOUNDMANAGER_HPP
