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
#include "sound_manager.hpp"
#include "sound.hpp"

#include "logging.hpp"
#include "options.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <algorithm> // for std::remove

SoundManager::SoundManager() :
	next_song(-1), prev_song(-2), silence_timer(1000), current_song(nullptr),
	current_playlist(PT_MENU), previous_playlist(PT_MENU)
{
	set_music_volume(options.get_i("sound-music_volume"));
}
SoundManager::~SoundManager()
{
	free();
}
void SoundManager::free()
{
	playlists[PT_MENU].clear();
	playlists[PT_PEST].clear();
	playlists[PT_KOBOLD].clear();
	playlists[PT_DWARF].clear();
	playlists[PT_DEMON].clear();
	playlists[PT_DEFEAT].clear();
	playlists[PT_VICTORY].clear();
	playlists[PT_BOSS].clear();

	sound_map.clear();
	reference_count.clear();

	logging.cout("All sounds erased", LOG_SOUND);
}
void SoundManager::update()
{
	if (volume_music <= 0)
		return;

	if (silence_timer > 0)
		silence_timer -= engine.get_dt();

	else if (next_song >= 0 && playlists[current_playlist].size() > 0 &&
		Mix_PlayingMusic() == 0 && Mix_FadingMusic() == MIX_NO_FADING)
	{
		if (current_song != nullptr)
			free_sound(current_song->get_name());

		const std::string next = playlists[current_playlist][next_song];

		current_song = load_sound(next, true);
		if (current_song != nullptr)
		{
			Mix_VolumeMusic(volume_music);
			current_song->fade_in(500);

			/*MessageLog *ml = ui.get_message_log();
			if (ml != nullptr)
			{
				std::size_t split = next.rfind('/');
				if (split != std::string::npos)
					ml->add_message("Now playing '" + next.substr(split + 1) + "'", DAWN_CORNFLOWER);
			}*/
			prev_song = next_song;
			next_song = -1;
			/*next_song += 1;
			if (next_song >= playlists[current_playlist].size())
				next_song = 0;*/

			previous_playlist = current_playlist;
		}
		else remove_from_playlist(current_playlist, next);
	}
	else silence_timer = 10000;
}
void SoundManager::skip_song(uint16_t ms)
{
	if (Mix_PlayingMusic() != 0)
	{
		if (ms > 0)
			Mix_FadeOutMusic(ms);
		else Mix_HaltMusic();

		prev_song = next_song;

		next_song += 1;
		if (next_song >= (int8_t)playlists[current_playlist].size())
			next_song = 0;
	}
	silence_timer = ms + 500;
}
void SoundManager::pause_music(bool msg_log)
{
	Mix_PauseMusic();

	if (msg_log && ui.get_message_log() != nullptr)
		ui.get_message_log()->add_message("Music paused", DAWN_CORNFLOWER);
}
void SoundManager::resume_music(bool msg_log)
{
	Mix_ResumeMusic();

	if (msg_log && ui.get_message_log() != nullptr)
		ui.get_message_log()->add_message("Music resumed", DAWN_CORNFLOWER);
}
void SoundManager::stop_music()
{
	Mix_HaltMusic();
}
Sound* SoundManager::load_sound(const std::string &sound_name, bool music)
{
	if (volume_music <= 0)
		return nullptr;

	auto it = sound_map.find(sound_name);
	if (it == sound_map.end())
	{
		std::shared_ptr<Sound> temp_sound = std::make_shared<Sound>();
		if (!temp_sound->load_from_file(sound_name, music))
		{
			temp_sound.reset();
			return nullptr;
		}
		else sound_map[sound_name] = std::move(temp_sound);
		reference_count[sound_name] = 1;

		logging.cout(std::string("Sound loaded: ") + sound_name, LOG_SOUND);
	}
	else reference_count[sound_name] += 1;
	return sound_map[sound_name].get();
}
void SoundManager::free_sound(const std::string &sound_name)
{
	auto it = sound_map.find(sound_name);
	if (it != sound_map.end())
	{
		reference_count[sound_name] -= 1;
		if (reference_count[sound_name] == 0)
		{
			it->second.reset();
			sound_map.erase(it);

			logging.cout(std::string("Sound freed: ") + sound_name, LOG_SOUND);
		}
	}
}
void SoundManager::add_to_playlist(PlaylistType playlist, const std::string &sound_name)
{
	// The same sound can be added multiple times to the playlist
	playlists[playlist].push_back(sound_name);
}
void SoundManager::remove_from_playlist(PlaylistType playlist, const std::string &sound_name)
{
	playlists[playlist].erase(
		std::remove(playlists[playlist].begin(), playlists[playlist].end(), sound_name),
		playlists[playlist].end()
	);
}
void SoundManager::clear_playlist(PlaylistType playlist)
{
	for (std::string song : playlists[playlist])
		free_sound(song);
	playlists[playlist].clear();
}
void SoundManager::set_music_volume(uint8_t volume)
{
	if (volume > 100)
		volume_music = MIX_MAX_VOLUME;
	else if (volume != 0)
		volume_music = std::abs(MIX_MAX_VOLUME / (100.0f / volume));
	else volume_music = 0;

	if (volume_music == 0)
		stop_music();
	Mix_VolumeMusic(volume_music);

	logging.cout(std::string("Music volume set to: ") + std::to_string((int)volume_music) + "/" + std::to_string(MIX_MAX_VOLUME), LOG_SOUND);
}
void SoundManager::set_playlist(PlaylistType playlist, bool instant)
{
	//const bool skip = (instant && playlist != current_playlist);

	previous_playlist = current_playlist;
	current_playlist = playlist;

	if (next_song < 0)
	{
		/*uint8_t loops = 0;
		while (next_song != prev_song)
		{*/
			if (playlists[current_playlist].size() > 1) // Start with a random song if there's more than one
				next_song = engine.get_rng() % playlists[current_playlist].size();
			else next_song = 0;

			/*loops += 1;
			if (loops > 200)
			{
				next_song = 0;
				break;
			}
		}*/
	}
	//if (skip) // Stop playing whatever we're playing and move to the next playlist/song
		skip_song(100);
}
