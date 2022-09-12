#pragma once
#ifndef SG_SOUND_H
#define SG_SOUND_H

#include "utility/SGUtil.h"
#include "fmod.hpp"
#include "vector"
#include "string"
#include "mutex"
#include "unordered_map"

#define SOUND_CHANNELS 512

namespace SGSound
{
	enum class ChannelGroup
	{
		NONE = -1, EFFECTS = 0, MUSIC = 1, CHANNEL_GROUP_COUNT = 2
	};

	typedef size_t sound_id;

	/** 
	* Simple non directional FMOD wrapper
	*/
	class System
	{
	public:
		bool init();
		//Returns if result is FMOD_OK
		sound_id loadSound(const char* path, FMOD::Sound*& sound);
		sound_id loadSound(const char* path);
		//Loads sound as new regardless of if it already is loaded
		void releaseSound(FMOD::Sound*& sound);
		void releaseSound(sound_id sound);
		//Returns whether the playback has properly started
		bool playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		//Queues to be played so is handled by the audio update
		void playSound(sound_id sound, FMOD::Channel** channel, ChannelGroup group);
		void pauseSound(FMOD::Channel*& channel, sound_id sound);
		void stopSound(FMOD::Channel*& channel, sound_id sound);
		//Returns play position when paused
		void update();
		void clean();

		//Adjust volume for channel
		void setChannelGroupVolume(ChannelGroup group, float volume);
		FMOD::ChannelGroup* getGroup(ChannelGroup group) { return m_ChannelGroups[(int)group]; }
	private:
		bool playSoundInternal(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		void releaseSoundInternal(FMOD::Sound*& sound);
		void releaseQueuedSound(int& index);
		std::vector<FMOD::Sound*> m_ReleaseQueue; //Queue ensures that if a loading sound is cut off it is disposed of
		FMOD::System* m_System = NULL;

		//Channels
		std::vector<FMOD::ChannelGroup*> m_ChannelGroups;

		//Sounds storage
		struct ChannelInstance
		{
			FMOD::Channel** channel = NULL;
			//Playback for one sound at a time
			bool shouldPlay = false;
			bool isPlaying = false;
		};

		struct SoundData
		{
			sound_id id; //Unique for a given path
			FMOD::Sound* sound = NULL;
			ChannelGroup group;
			std::vector<ChannelInstance> chl; //Can fire off same sound in multiple instances
		};
		//Takes the path stored under, and a reference to the sound to overwrite if found
		int isSoundStored(std::string path, FMOD::Sound*& sound); //-1 if not

		std::vector<SoundData> m_Sounds;
		std::shared_mutex m_SoundAccessMutex;
	};
}

#endif