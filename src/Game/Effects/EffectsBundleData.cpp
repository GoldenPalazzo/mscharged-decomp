#include "Game/Effects/EffectsBundleData.h"

#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EffectsTemplate.h"
#include "Game/UnidentifiedStaticStorage.h"
#include "NL/nlChunk.h"

EffectsBundleData* EffectsBundleData::Initialize(nlChunk* bundle)
{
    EffectsBundleData* data;
    nlChunk* chunk = bundle->GetFirstChunk();
    data = (EffectsBundleData*)chunk->GetData();

    chunk = chunk->GetNextChunk();
    data->mTemplates = (EffectsTemplate**)chunk->GetData();

    chunk = chunk->GetNextChunk();
    data->mGroups = (EffectsGroup**)chunk->GetData();

    unsigned long i;
    for (i = 0; i < data->mNumTemplates; ++i)
    {
        chunk = chunk->GetNextChunk();
        data->mTemplates[i] = EffectsTemplate::LoadFromChunk(chunk);
    }

    for (i = 0; i < data->mNumGroups; ++i)
    {
        chunk = chunk->GetNextChunk();
        data->mGroups[i] = EffectsGroup::LoadFromChunk(chunk);
    }

    for (unsigned long j = 0; j < data->mNumGroups; ++j)
    {
        data->mGroups[j]->ResolveTemplates(data->mTemplates);
    }
    return data;
}

void EffectsBundleData::Destroy()
{
    for (unsigned long i = 0; i < mNumTemplates; ++i)
    {
        mTemplates[i]->Cleanup();
    }
    for (unsigned long i = 0; i < mNumGroups; ++i)
    {
        mGroups[i]->DestroyUserSpecs();
    }
}
