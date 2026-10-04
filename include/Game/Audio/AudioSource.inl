#ifndef GAME_AUDIO_AUDIO_SOURCE_INL
#define GAME_AUDIO_AUDIO_SOURCE_INL

inline bool AudioSource::IsLooping()
{
    return m_Unknown14_00 == 0xFFFF;
}

inline void AudioReadState::SetInputVolume(float value)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoiceInputVolume(channel->m_Unknown04, value);
    }
    OSRestoreInterrupts(enabled);
}

inline void AudioReadState::SetMixVolume(float value)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoiceMixVolume(channel->m_Unknown04, value);
    }
    OSRestoreInterrupts(enabled);
}

inline void AudioReadState::SetPitch(float value)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoicePitch(channel->m_Unknown04, m_Unknown0C, value);
        OSRestoreInterrupts(enabled);
    }
}

inline void AudioReadState::SetSurroundPan(float value)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoiceSurroundPan(channel->m_Unknown04, value);
    }
    OSRestoreInterrupts(enabled);
}

inline void AudioReadState::SetLowPassFilter(bool on, unsigned int frequency, bool unchanged)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoiceLowPassFilter(channel->m_Unknown04, on, frequency, unchanged);
    }
    OSRestoreInterrupts(enabled);
}

inline void AudioReadState::SetAuxiliaryVolume(int auxiliary, int value)
{
    bool enabled = OSDisableInterrupts();
    if (!HasVoice())
    {
        OSRestoreInterrupts(enabled);
        return;
    }
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown04 != 0)
            SetVoiceAuxiliaryVolume(channel->m_Unknown04, auxiliary, value);
    }
    OSRestoreInterrupts(enabled);
}

template <unsigned int ChannelCount>
inline AXVPB* UnidentifiedAudioChannels<ChannelCount>::GetVoice()
{
    return m_Channels[0].m_Unknown04;
}

template <unsigned int ChannelCount>
inline bool UnidentifiedAudioChannels<ChannelCount>::WasVoiceDropped()
{
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
    {
        if (channel->m_Unknown10_1F)
            return true;
    }
    return false;
}

template <unsigned int ChannelCount>
inline void UnidentifiedAudioChannels<ChannelCount>::ReleaseVoice(bool release)
{
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
        channel->ReleaseVoice(release);
}

template <unsigned int ChannelCount>
inline unsigned int UnidentifiedAudioChannels<ChannelCount>::GetChannelCount()
{
    return ChannelCount;
}

template <unsigned int ChannelCount>
inline AudioStreamChannel* UnidentifiedAudioChannels<ChannelCount>::GetFirstChannel()
{
    return m_Channels;
}

template <unsigned int ChannelCount>
inline AudioStreamChannel* UnidentifiedAudioChannels<ChannelCount>::GetChannelIterator()
{
    return m_Channels - 1;
}

template <unsigned int ChannelCount>
inline AudioStreamChannel* UnidentifiedAudioChannels<ChannelCount>::GetNextChannel(AudioStreamChannel* channel)
{
    ++channel;
    if (channel >= m_Channels + ChannelCount)
        channel = 0;
    return channel;
}

template <unsigned int ChannelCount>
inline bool UnidentifiedAudioChannels<ChannelCount>::HasVoice()
{
    bool hasVoice = false;
    AudioStreamChannel* channel = GetChannelIterator();
    while ((channel = GetNextChannel(channel)) != 0)
        hasVoice |= channel->m_Unknown04 != 0;
    return hasVoice;
}

inline AudioReadState_8035D154::~AudioReadState_8035D154()
{
}

inline void AudioReadState_8035D154::SetPan(float value)
{
    SetVoicePan(m_Channels[0].m_Unknown04, value);
}

inline void AudioReadState_8035D154::SetInterauralDelay(int value)
{
    SetVoiceInterauralDelay(m_Channels[0].m_Unknown04, value);
}

inline unsigned int AudioReadState_8035D154::UnidentifiedVirtual70()
{
    return m_Unknown08->m_Unknown04;
}

inline AudioSource::~AudioSource()
{
}

#endif
