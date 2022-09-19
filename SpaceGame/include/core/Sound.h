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
	* Simple non directional FMOD wrapper
	*/
	class System
	{
	public:
		bool init();
		/**
		* Loads sound from path and writes location to ptr reference, storing the sound pointer internally as well
		*/
		sound_id loadSound(const char* path, FMOD::Sound*& sound);
		/**
		* Loads sound from path , storing the sound pointer internally as well
		* Returns the id for the sound
		*/
		sound_id loadSound(const char* path);

		/**
		* Directly releases sound from pointer
		*/
		void releaseSound(FMOD::Sound*& sound);
		/**
		* Releases sound from internal based on ID
		*/
		void releaseSound(sound_id sound);
		/**
		* Plays sound from pointer directly
		*/
		bool playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		/**
		* Queues sound to be played immediately, once is loaded
		*/
		void playSound(sound_id sound, FMOD::Channel** channel, ChannelGroup group);
		/**
		* Queues the sound to be paused immediately
		*/
		void pauseSound(FMOD::Channel*& channel, sound_id sound);
		/**
		* Queues the sound to be stopped immediately
		*/
		void stopSound(FMOD::Channel*& channel, sound_id sound);
		/**
		* Queues the sound to be faded over a given period
		*/
		void fadeSound(sound_id id, FMOD::Channel*& channel, float fadeTime, bool fadeIn);
		
		void update();
		void clean();

		/**
		* Sets volume for channel
		*/
		void setChannelGroupVolume(ChannelGroup group, float volume);
		FMOD::ChannelGroup* getGroup(ChannelGroup group) { return m_ChannelGroups[(int)group]; }
	private:
		bool playSoundInternal(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		void releaseSoundInternal(FMOD::Sound*& sound);
		void fadeSoundInternal(FMOD::Channel*& channel, float fadeTime, bool fadeIn);

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
		int isSoundStored(std::string path, FMOD::Sound*& sound); //-1 if not

		std::vector<SoundData> m_Sounds;
		std::shared_mutex m_SoundAccessMutex;
	};
}

#endif