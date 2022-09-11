#pragma once
#ifndef SG_SOUND_H
#define SG_SOUND_H

#include "fmod.hpp"
#include "utility/SGUtil.h"
#include "vector"

#define SOUND_CHANNELS 512

namespace SGSound
{
	enum class ChannelGroup
	{
		EFFECTS = 0, MUSIC = 1, CHANNEL_GROUP_COUNT = 2
	};

	/** 
	* Simple non directional FMOD wrapper
	*/
	class System
	{
	public:
		bool init();
		//Returns if result is FMOD_OK
		bool loadSound(const char* path, FMOD::Sound*& sound);
		void releaseSound(FMOD::Sound*& sound);
		//Returns whether the playback has properly started
		bool playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group);
		//Returns play position when paused
		bool setPauseSound(FMOD::Sound*& sound, FMOD::Channel*& channel, bool pause);
		void update();
		void clean();

		//Adjust volume for channel
		void setChannelGroupVolume(ChannelGroup group, float volume);
		FMOD::ChannelGroup* getGroup(ChannelGroup group) { return m_ChannelGroups[(int)group]; }
	private:
		void releaseQueuedSound(int index);
		std::vector<FMOD::Sound*> m_ReleaseQueue; //Queue ensures that if a loading sound is cut off it is disposed of
		FMOD::System* m_System = NULL;

		//Channels
		std::vector<FMOD::ChannelGroup*> m_ChannelGroups;
	};
}

#endif