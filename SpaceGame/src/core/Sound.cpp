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

size_t SGSound::System::loadSound(const char* path, FMOD::Sound*& sound)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    if (isSoundStored(std::string(path), sound))
    {
        return 0;
    }
    FMOD_RESULT result = m_System->createSound(path, FMOD_NONBLOCKING, NULL, &sound);
    std::hash<std::string> hash;
    size_t hashedPath = hash(path);
    m_Sounds.push_back({ hashedPath, sound, ChannelGroup::NONE, NULL, false, false });
    return hashedPath;
}

size_t SGSound::System::loadSound(const char* path)
{
    FMOD::Sound* sound;
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    if (isSoundStored(std::string(path), sound))
    {
        return 0;
    }
    FMOD_RESULT result = m_System->createSound(path, FMOD_NONBLOCKING, NULL, &sound);
    if (result != FMOD_OK)
    {
        EngineLog("Error loading sound: ", path);
    }
    //Create id and push
    std::hash<std::string> hash;
    size_t hashedPath = hash(path);
    m_Sounds.push_back({ hashedPath, sound, ChannelGroup::NONE, NULL, false, false });
    return hashedPath;
}

bool SGSound::System::isSoundStored(std::string path, FMOD::Sound*& sound)
{
    std::hash<std::string> hash;
    size_t hashedPath = hash(path);
    for (int i = 0; i < m_Sounds.size(); i++)
    {
        if (m_Sounds[i].id == hashedPath)
        {
            sound = m_Sounds[i].sound;
            return true;
        }
    }
    return false;
}

bool SGSound::System::playSound(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    return playSoundInternal(sound, channel, group);
}

bool SGSound::System::playSoundInternal(FMOD::Sound*& sound, FMOD::Channel*& channel, ChannelGroup group)
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

void SGSound::System::playSound(sound_id sound, FMOD::Channel*& channel, ChannelGroup group)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    //Find sound with id, set to shouldPlay
    for (int i = 0; i < m_Sounds.size(); i++)
    {
        if (m_Sounds[i].id == sound)
        {
           m_Sounds[i].shouldPlay = true;
           m_Sounds[i].channel = channel;
           m_Sounds[i].group = group;
        }
    }
}

void SGSound::System::releaseSound(FMOD::Sound*& sound)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    for (int i = 0; i < m_ReleaseQueue.size(); i++)
    {
        //Ensure no duplicates
        if (m_ReleaseQueue[i] == sound)
        {
            return;
        }
    }
    m_ReleaseQueue.push_back(sound);

    //Remove from storage if present
    for (int i = 0; i < m_Sounds.size(); i++)
    {
        if (m_Sounds[i].sound == sound)
        {
            m_Sounds.erase(m_Sounds.begin() + i);
            return;
        }
    }
}

void SGSound::System::releaseSound(sound_id sound)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    //Get sound pointer
    FMOD::Sound* soundPtr = NULL;
    for (int i = 0; i < m_Sounds.size(); i++)
    {
        if (m_Sounds[i].id == sound)
        {
            soundPtr = m_Sounds[i].sound;
        }
    }

    //If found, release
    if (soundPtr == NULL)
    {
        return;
    }
    releaseSound(soundPtr);
}

void SGSound::System::releaseQueuedSound(int& index)
{
    FMOD_OPENSTATE state;
    m_ReleaseQueue[index]->getOpenState(&state, NULL, NULL, NULL);
    if (state != FMOD_OPENSTATE_READY)
    {
        return;
    }
    m_ReleaseQueue[index]->release();
    m_ReleaseQueue.erase(m_ReleaseQueue.begin() + index);
    index--;
}

void SGSound::System::update()
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    m_System->update();
    for (int i = 0; i < m_ReleaseQueue.size(); i++)
    {
        releaseQueuedSound(i);
    }

    //Play sounds that should be played
    for (int i = 0; i < m_Sounds.size(); i++)
    {
        if (m_Sounds[i].shouldPlay && !m_Sounds[i].isPlaying)
        {
            SoundData& data = m_Sounds[i];
            if (playSoundInternal(data.sound, data.channel, data.group))
            {
                data.isPlaying = true;
            }
        }
    }
}

void SGSound::System::clean()
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    m_Sounds.clear();
    m_ReleaseQueue.clear();
    m_System->release();
    m_System->close();
}

void SGSound::System::setChannelGroupVolume(ChannelGroup group, float volume)
{
    std::lock_guard<std::shared_mutex> lock(m_SoundAccessMutex);
    m_ChannelGroups[(int)group]->setVolume(volume);
}