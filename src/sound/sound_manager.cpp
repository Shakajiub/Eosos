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

#include "options.hpp"
#include "message_log.hpp"
#include "ui.hpp"

#include <algorithm> // for std::remove

SoundManager::SoundManager() :
	next_song(0), silence_timer(1000), current_song(nullptr),
	current_playlist(PT_AMBIANCE), previous_playlist(PT_AMBIANCE)
{
	set_music_volume(options.get_i("sound-music_volume"));
}
SoundManager::~SoundManager()
{
	free();
}
void SoundManager::free()
{
	playlists[PT_AMBIANCE].clear();
	playlists[PT_COMBAT].clear();

	sound_map.clear();
	reference_count.clear();

	std::cout << "all sounds erased" << std::endl;
}
void SoundManager::update()
{
	return; // Disable music for now

	if (silence_timer > 0)
		silence_timer -= engine.get_dt();

	else if (volume_music > 0 && playlists[current_playlist].size() > 0 &&
		Mix_PlayingMusic() == 0 && Mix_FadingMusic() == MIX_NO_FADING)
	{
		if (current_song != nullptr)
			free_sound(current_song->get_name());

		// PT_COMBAT gets automatically reset back to PT_AMBIANCE
		if (previous_playlist == PT_COMBAT)
			set_playlist(PT_AMBIANCE, false);

		const std::string next = playlists[current_playlist][next_song];

		current_song = load_sound(next, true);
		if (current_song != nullptr)
		{
			Mix_VolumeMusic(volume_music);
			current_song->fade_in(500);
			//current_song->play();

			MessageLog *ml = ui.get_message_log();
			if (ml != nullptr)
			{
				std::size_t split = next.rfind('/');
				if (split != std::string::npos)
					ml->add_message("Now playing '" + next.substr(split + 1) + "'", COLOR_CORNFLOWER);
			}
			next_song += 1;
			if (next_song >= playlists[current_playlist].size())
				next_song = 0;

			previous_playlist = current_playlist;
		}
		// If we can't play the song, remove it from the playlist
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
	}
	silence_timer = ms + 500;
}
void SoundManager::pause_music()
{
	Mix_PauseMusic();
}
void SoundManager::resume_music()
{
	Mix_ResumeMusic();
}
void SoundManager::stop_music()
{
	Mix_HaltMusic();
}
Sound* SoundManager::load_sound(const std::string &sound_name, bool music)
{
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

		std::cout << "sound loaded: " << sound_name << std::endl;
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

			std::cout << "sound freed: " << sound_name << std::endl;
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
	// This removes ALL instances of the given sound from the playlist
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

	std::cout << "music volume set to: " << (int)volume_music << std::endl;
}
void SoundManager::set_playlist(PlaylistType playlist, bool instant)
{
	const bool skip = (instant && playlist != current_playlist);

	previous_playlist = current_playlist;
	current_playlist = playlist;

	if (playlists[current_playlist].size() > 1) // Start with a random song if there's more than one
		next_song = engine.get_rng() % playlists[current_playlist].size();
	else next_song = 0;

	if (skip) // Stop playing whatever we're playing and move to the next playlist
		skip_song(100);
}
