#include "core/Sound.h"

bool SGSound::System::init()
{
	bool success = true;
    //Init sound engine
    FMOD_RESULT result;
    result = FMOD::System_Create(&m_System);
    if (result != FMOD_OK)
    {
        EngineLog("FMOD Failed to create!");
        success = false;
    }

    result = m_System->init(SOUND_CHANNELS, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        EngineLog("FMOD Failed to initalize!");
        success = false;
    }

    //Init channel groups
    m_ChannelGroups.resize((int)ChannelGroup::CHANNEL_GROUP_COUNT);
    result = m_System->createChannelGroup("effects", &m_ChannelGroups[(int)ChannelGroup::EFFECTS]);
    if (result != FMOD_OK)
    {
        EngineLog("Effect channel failed to initialize!");
        success = false;
    }

    result = m_System->createChannelGroup("music", &m_ChannelGroups[(int)ChannelGroup::MUSIC]);
    if (result != FMOD_OK)
    {
        EngineLog("Music channel failed to initialize!");
        success = false;
    }

    return success;
}

bool SGSound::System::loadSound(const char* path, FMOD::Sound*& sound)
{
    FMOD_RESULT result = m_System->createSound(path, FMOD_NONBLOCKING, NULL, &sound);
    return result == FMOD_OK;
}

bool SGSound::System::playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group)
{
    //If isn't ready, don't start playing yet
    FMOD_OPENSTATE state;
    sound->getOpenState(&state, NULL, NULL, NULL);
    if (state != FMOD_OPENSTATE_READY)
    {
        return false;
    }
    m_System->playSound(sound, NULL, false, &channel);

    //Place in correct channel
    channel->setChannelGroup(m_ChannelGroups[(int)group]);
    return true;
}

bool SGSound::System::setPauseSound(FMOD::Sound*& sound, FMOD::Channel*& channel, bool pause)
{
    //If isn't ready, don't start playing yet
    FMOD_OPENSTATE state;
    sound->getOpenState(&state, NULL, NULL, NULL);
    if (state != FMOD_OPENSTATE_READY)
    {
        return false;
    }
    channel->setPaused(pause);
    return true;
}

void SGSound::System::releaseSound(FMOD::Sound*& sound)
{
    for (int i = 0; i < m_ReleaseQueue.size(); i++)
    {
        //Ensure no duplicates
        if (m_ReleaseQueue[i] == sound)
        {
            return;
        }
    }
    m_ReleaseQueue.push_back(sound);
}

void SGSound::System::releaseQueuedSound(int index)
{
    FMOD_OPENSTATE state;
    m_ReleaseQueue[index]->getOpenState(&state, NULL, NULL, NULL);
    if (state != FMOD_OPENSTATE_READY)
    {
        return;
    }
    m_ReleaseQueue[index]->release();
    m_ReleaseQueue.erase(m_ReleaseQueue.begin() + index);
}

void SGSound::System::update()
{
    m_System->update();
    for (int i = 0; i < m_ReleaseQueue.size(); i++)
    {
        releaseQueuedSound(i);
    }
}

void SGSound::System::clean()
{
    m_System->release();
    for (int i = 0; i < m_ReleaseQueue.size(); i++)
    {
        releaseQueuedSound(i);
    }
    for (int i = 0; i < m_ChannelGroups.size(); i++)
    {
        m_ChannelGroups[i]->release();
    }
}

void SGSound::System::setChannelGroupVolume(ChannelGroup group, float volume)
{
    m_ChannelGroups[(int)group]->setVolume(volume);
}