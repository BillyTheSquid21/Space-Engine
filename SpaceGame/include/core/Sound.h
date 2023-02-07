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
	/**
	* Channel group to use, currently only music or effects
	*/
	enum class ChannelGroup
	{
		NONE = -1, EFFECTS = 0, MUSIC = 1, CHANNEL_GROUP_COUNT = 2
	};

	/**
	* Is essentially the hashed form of the path, which identifies it in storage 
	*/
	typedef size_t sound_id;

	/** 
	* Simple non directional FMOD wrapper - TODO - make static
	*/
	class System
	{
	public:
		static bool init();
		static void set();

		/**
		* Loads sound from path and writes location to ptr reference, storing the sound pointer internally as well
		*/
		static sound_id loadSound(const char* path, FMOD::Sound*& sound);
		
		/**
		* Loads sound from path , storing the sound pointer internally as well
		* Returns the id for the sound
		*/
		static sound_id loadSound(const char* path);
		/**
		* Directly releases sound from pointer
		*/
		static void releaseSound(FMOD::Sound*& sound);
		/**
		* Releases sound from internal based on ID
		*/
		static void releaseSound(sound_id sound);
		/**
		* Plays sound from pointer directly
		*/
		static bool playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		/**
		* Queues sound to be played immediately, once is loaded
		*/
		static void playSound(sound_id sound, FMOD::Channel** channel, ChannelGroup group);
		/**
		* Queues the sound to be paused immediately
		*/
		static void pauseSound(FMOD::Channel*& channel, sound_id sound);
		/**
		* Queues the sound to be stopped immediately
		*/
		static void stopSound(FMOD::Channel*& channel, sound_id sound);
		/**
		* Queues the sound to be faded over a given period
		*/
		static void fadeSound(sound_id id, FMOD::Channel*& channel, float fadeTime, bool fadeIn);
		
		static void update();
		static void clean();

		/**
		* Sets volume for channel
		*/
		static void setChannelGroupVolume(ChannelGroup group, float volume);
		static FMOD::ChannelGroup* getGroup(ChannelGroup group) { return m_ChannelGroups[(int)group]; }
	private:

		static bool playSoundInternal(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		static void releaseSoundInternal(FMOD::Sound*& sound);
		static void fadeSoundInternal(FMOD::Channel*& channel, float fadeTime, bool fadeIn);

		static void releaseQueuedSound(int& index);
		static std::vector<FMOD::Sound*> m_ReleaseQueue; //Queue ensures that if a loading sound is cut off it is disposed of
		static FMOD::System* m_System;

		//Channels
		static std::vector<FMOD::ChannelGroup*> m_ChannelGroups;

		//Sounds storage
		struct ChannelInstance
		{
			FMOD::Channel** channel = NULL;
			//Playback for one sound at a time
			bool shouldPlay = false;
			bool isPlaying = false;
			char fade = -1; //0 is fade in, 1 is fade out
			float fadeTime = 0.0f;
		};

		struct SoundData
		{
			sound_id id; //Unique for a given path
			FMOD::Sound* sound = NULL;
			ChannelGroup group;
			std::vector<ChannelInstance> chl; //Can fire off same sound in multiple instances
		};
		//Takes the path stored under, and a reference to the sound to overwrite if found
		static int isSoundStored(std::string path, FMOD::Sound*& sound); //-1 if not

		static std::vector<SoundData> m_Sounds;
		static std::shared_mutex m_SoundAccessMutex;
		static bool s_Set;
	};
}

#endif