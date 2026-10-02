#include "Game/AI/TeamPlayMachine.h"
#include "Game/AI/AIContext.h"
#include "Game/UnidentifiedStaticStorage.h"
#include "Game/Audio/RegistryPools.h"

TeamDesire::TeamDesire(
    int state, TransitionFunc& transition)
    : shdStateMachine(state, transition)
{
    mUnidentified080 = 0.33f;
    mUnidentified084 = 1.0f;
}

void TeamDesire::UnidentifiedSetContext(
    UnidentifiedScriptMachine* context)
{
    shdStateMachine::UnidentifiedSetContext(context);
    if (context != 0)
    {
        m_pTeam = (cTeam*)context->mAIContext->mData.pointer;
    }
    else
    {
        m_pTeam = 0;
    }
}

bool TeamDesire::UnidentifiedReinitialize(void*)
{
    return true;
}

void TeamDesire::UnidentifiedCleanup()
{
}

void TeamDesire::Update(DesireUpdate*, float)
{
}
