#ifndef GAME_AUDIO_AUDIO_RESOURCE_RUNTIME_H
#define GAME_AUDIO_AUDIO_RESOURCE_RUNTIME_H

#include "types.h"
#include "Game/Audio/AudioRegistryOwner.h"

class AudioConfigNode;
class AudioBundleManager;
class AudioEffectFactory;
class AudioScriptRuntime;

// The audio bundle's packed configuration tree and script runtime.
class AudioResourceRuntime
{
public:
    AudioResourceRuntime();

    /* 0x00 */ AudioRegistryOwner m_Registry;

    void LoadScriptData(void* data, unsigned int size);
    void ApplyEffectSet(u32 bindingKey, u32 effectSetKey,
        bool inverted, void* owner);
    void ApplyEffectSet(u32 bindingKey, u32 effectSetKey,
        bool inverted, float blendTime);
    AudioConfigNode* GetConfigRoot() const { return m_ConfigRoot; }

    /* 0x20 */ AudioConfigNode* m_ConfigRoot;
    /* 0x24 */ AudioScriptRuntime* m_Script;
    /* 0x28 */ AudioEffectFactory* m_EffectFactory;
}; // size: 0x2C

extern AudioResourceRuntime* g_pAudioResourceRuntime;

void UpdateAudioResourceRuntime(AudioResourceRuntime* runtime, float deltaTime);
void ShutdownAudioResourceRuntime(AudioResourceRuntime* runtime);
void NotifyAudioSoundStarted(AudioResourceRuntime* runtime, u32 hash, u32 instance);
void NotifyAudioSoundStopped(AudioResourceRuntime* runtime, u32 instance);
// C linkage: AsyncLoading.cpp sees the SDK u32 (unsigned long), which would
// mangle a C++ declaration differently from this unit's u32.
extern "C" bool StartAudioEffect(const u32* bindingKey, const u32* definitionKey,
    void* parameterData, bool invert, float blendTime);
bool ApplyAudioTransition(const u32* transitionHash, bool invert, void* owner);

#endif // GAME_AUDIO_AUDIO_RESOURCE_RUNTIME_H
