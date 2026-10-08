#include "NL/nlDLListContainer.inl"
#include "Game/Sys/audio.h"
#include "Game/UnidentifiedStaticStorage.h"
#include "Game/Audio/RegistryPools.h"
#include "Game/CrowdRiot.h"
#include "Game/Goalie.h"

#include "Game/AI/AiUtil.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Powerups.h"
#include "Game/Ball.h"
#include "Game/DebugWriteCache.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Event.h"
#include "Game/EventDataTypes.h"
#include "Game/EventRegistry.h"
#include "Game/Field.h"
#include "Game/Game.h"
#include "Game/MathHelpers.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/PhysicsAIBall.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Physics/PhysicsCharacter.h"
#include "Game/Physics/PhysicsTriggerVolume.h"
#include "NL/nlBind.h"
#include "NL/nlMemory.h"
#include "NL/nlSlotPool.h"

#include <math.h>
#include "NL/nlFunction.inl"

struct Generators
{
    void RegisterDebugFields(u16* type, DebugWriteCache* cache);
    /* 0x00 */ nlVector2 v2Location;
    /* 0x08 */ bool bIsOn;
    /* 0x0C */ float fTimeToExplode;
}; // total size: 0x10

float lbl_806E0C40;
float lbl_806E0C44;

void fn_80029C80(PhysicsObject*, PhysicsObject*, const nlVector3&, void*);
void fn_800298D8(void*);
void fn_800299C4(void*);
void fn_80029AB0(void*);
void fn_80029B9C(void*);

static float sUnidentifiedFloat0 = 2.45f;
static float sUnidentifiedFloat1 = 4.0f;
static float sUnidentifiedFloat2 = 2.0f;
static float sUnidentifiedFloat3 = 1.0f;
static float sUnidentifiedFloat4 = 10.4f;
static float sUnidentifiedFloat5 = 3.0f;
static unsigned short sCrowdRiotType = 0xFFFF;
static unsigned short sGeneratorsType = 0xFFFF;

Generators lbl_8056B890[6];

CrowdRiot::CrowdRiot(bool param1)
    : mTriggerVolume(0)
{
    fn_8002921C();
    if (param1)
    {
        fn_80029460(false);
    }
    else
    {
        mfRiotTime = -1.0f;
        meState = 0;
        mfStateTime = -1.0f;
        maDesiredFacingDirection = 0;
        mv3Position.x = 0.0f;
        mv3Position.y = 0.0f;
        mv3Position.z = -10.0f;
        mv3Velocity.x = 0.0f;
        mv3Velocity.y = 0.0f;
        mv3Velocity.z = 0.0f;
        mv3Target.x = 0.0f;
        mv3Target.y = 0.0f;
        mv3Target.z = 0.0f;
    }

    UnidentifiedFindEvent<void>("CollisionCrowd", -1)->Add(Function<void*>(fn_80029B9C), 0, -1);
    UnidentifiedFindEvent<void>("GoalScored", -1)->Add(Function<void*>(fn_800298D8), 0, -1);
    UnidentifiedFindEvent<void>("MegastrikeEnd", -1)->Add(Function<void*>(fn_800299C4), 0, -1);
    UnidentifiedFindEvent<void>("GameOver", -1)->Add(Function<void*>(fn_80029AB0), 0, -1);
}

CrowdRiot::~CrowdRiot()
{
    if (mTriggerVolume != 0)
    {
        delete mTriggerVolume;
        mTriggerVolume = 0;
    }
}

inline void Generators::RegisterDebugFields(u16* type, DebugWriteCache* cache)
{
    *type = cache->BeginType("Generators");
    cache->AddField(21, gDebugFieldTypes[21].size, 0, "v2Location");
    cache->AddField(16, gDebugFieldTypes[16].size, (u8*)&bIsOn - (u8*)this, "bIsOn");
    cache->AddField(17, gDebugFieldTypes[17].size, (u8*)&fTimeToExplode - (u8*)this, "fTimeToExplode");
    cache->EndType();
}

inline void CrowdRiot::RegisterDebugFields(u16* type, DebugWriteCache* cache)
{
    *type = cache->BeginType("CrowdRiot");
    cache->AddField(17, gDebugFieldTypes[17].size, 0, "mfStateTime");
    cache->AddField(17, gDebugFieldTypes[17].size, (u8*)&mfRiotTime - (u8*)this, "mfRiotTime");
    cache->AddField(22, gDebugFieldTypes[22].size, (u8*)&mv3Target - (u8*)this, "mv3Target");
    cache->AddField(22, gDebugFieldTypes[22].size, (u8*)&mv3Position - (u8*)this, "mv3Position");
    cache->AddField(22, gDebugFieldTypes[22].size, (u8*)&mv3Velocity - (u8*)this, "mv3Velocity");
    cache->AddField(19, gDebugFieldTypes[19].size, (u8*)&maDesiredFacingDirection - (u8*)this, "maDesiredFacingDirection");
    cache->AddField(14, gDebugFieldTypes[14].size, (u8*)&meState - (u8*)this, "meState");
    cache->EndType();
}

void CrowdRiot::SyncLog(void* context, DebugWriteCache* cache)
{
    if (sCrowdRiotType == 0xFFFF)
    {
        RegisterDebugFields(&sCrowdRiotType, cache);
    }

    cache->ChecksumData(sCrowdRiotType, this, context);
    cache->WriteData(sCrowdRiotType, this, sizeof(CrowdRiot));

    for (int i = 0; i < 6; i++)
    {
        Generators* generator = &lbl_8056B890[i];
        if (sGeneratorsType == 0xFFFF)
        {
            generator->RegisterDebugFields(&sGeneratorsType, cache);
        }

        cache->ChecksumData(sGeneratorsType, generator, context);
        cache->WriteData(sGeneratorsType, generator, sizeof(Generators));
    }
}

void CrowdRiot::fn_8002921C()
{
    for (int i = 0; i < 6; i++)
    {
        Generators* generator = &lbl_8056B890[i];
        generator->bIsOn = true;
        generator->fTimeToExplode = -1.0f;

        float goalLineX = lbl_806E0C40 + cField::GetGoalLineX(1U);
        float sidelineY = lbl_806E0C44 + cField::GetSidelineY(1U);

        if (i == 0 || i == 3)
        {
            generator->v2Location.x = goalLineX;
        }
        else if (i == 2 || i == 5)
        {
            generator->v2Location.x = -goalLineX;
        }
        else
        {
            generator->v2Location.x = 0.0f;
        }

        if (i < 3)
        {
            generator->v2Location.y = sidelineY;
        }
        else
        {
            generator->v2Location.y = -sidelineY;
        }
    }
}

void InterpolateRiotBallPosition(nlVector3& result, const nlVector3& riotPosition,
    const nlVector3& ballPosition, float time)
{
    nlVecLerp(result, riotPosition, ballPosition, time);
}

void CrowdRiot::fn_80029320()
{
    for (int i = 0; i < 6; i++)
    {
        Generators* generator = &lbl_8056B890[i];
        if (!generator->bIsOn)
        {
            nlVector3 position;
            position.x = sUnidentifiedFloat4
                       * AIsgn(generator->v2Location.x);
            position.y = sUnidentifiedFloat5
                       + fabsf(generator->v2Location.y);
            position.y *= AIsgn(generator->v2Location.y);
            position.z = 0.0f;
            mv3Position = position;

            nlVector3 velocity;
            velocity.x = 0.0f;
            velocity.y = -sUnidentifiedFloat2;
            velocity.y *= AIsgn(generator->v2Location.y);
            velocity.z = 0.0f;
            mv3Velocity = velocity;
            maDesiredFacingDirection = nlVector3ToAngle(velocity);
        }
    }
}

enum CrowdRiotEffect
{
    GENERATOR_BROKEN,
    GENERATOR_EXPLOSION,
    CROWD_RIOT,
    CROWD_RIOT_WITH_FADE
};

EffectsGroup* GetCrowdRiotEffectGroup(CrowdRiotEffect effect)
{
    switch (effect)
    {
    case CROWD_RIOT_WITH_FADE:
        return EmissionManager::Instance()->GetEffectsGroup("crowd_riot_with_fade");
    case CROWD_RIOT:
        return EmissionManager::Instance()->GetEffectsGroup("crowd_riot");
    case GENERATOR_EXPLOSION:
        return EmissionManager::Instance()->GetEffectsGroup("generator_explode");
    case GENERATOR_BROKEN:
        return EmissionManager::Instance()->GetEffectsGroup("generator_broken");
    }
    return 0;
}

void PlayCrowdRiotSound(CrowdRiot* crowdRiot)
{
    PlaySound(13, 0x198B7ED3, "CrowdRiot", crowdRiot);
}

void CrowdRiot::fn_80029460(bool param1)
{
    if (meState == 0)
    {
        return;
    }

    EffectsGroup* group;
    EmissionController* controller;
    bool resumeRiot = false;
    if (mfRiotTime > 0.0f && param1)
    {
        resumeRiot = true;
    }
    else
    {
        mfRiotTime = -1.0f;
        fn_8002921C();
    }

    if (mTriggerVolume != 0)
    {
        delete mTriggerVolume;
        mTriggerVolume = 0;
    }

    meState = 1;
    mfStateTime = -1.0f;
    maDesiredFacingDirection = 0;
    mv3Position.x = 0.0f;
    mv3Position.y = 0.0f;
    mv3Position.z = -10.0f;
    mv3Velocity.x = 0.0f;
    mv3Velocity.y = 0.0f;
    mv3Velocity.z = 0.0f;
    mv3Target.x = 0.0f;
    mv3Target.y = 0.0f;
    mv3Target.z = 0.0f;

    group = GetCrowdRiotEffectGroup(GENERATOR_BROKEN);
    EmissionManager::Instance()->Kill((unsigned long)this, group);
    group = GetCrowdRiotEffectGroup(GENERATOR_EXPLOSION);
    EmissionManager::Instance()->Kill((unsigned long)this, group);
    group = GetCrowdRiotEffectGroup(CROWD_RIOT);
    EmissionManager::Instance()->Kill((unsigned long)this, group);
    group = GetCrowdRiotEffectGroup(CROWD_RIOT_WITH_FADE);
    EmissionManager::Instance()->Kill((unsigned long)this, group);
    StopSound(0x198B7ED3, this);

    if (resumeRiot && meState != 0 && meState == 1)
    {
        fn_80029320();
        meState = 2;
        mfStateTime = sUnidentifiedFloat3;

        if (mTriggerVolume == 0)
        {
            PhysicsTriggerVolume* physicsObject
                = new (8, false) PhysicsTriggerVolume(
                    sUnidentifiedFloat0);
            mTriggerVolume = physicsObject;
            physicsObject->m_pTriggerCallbackFunc
                = fn_80029C80;
            physicsObject->m_pCallbackParam = this;
            mTriggerVolume->SetPosition(
                mv3Position, PhysicsObject::WORLD_COORDINATES);
            mTriggerVolume->EnableCollisions();
        }

        group = GetCrowdRiotEffectGroup(CROWD_RIOT_WITH_FADE);
        controller = EmissionManager::Instance()->Create(group, 3, true, 0);
        controller->SetPosition(mv3Position);
        controller->SetVelocity(mv3Velocity);
        {
            Function<EmissionController&> callback(
                BindExp2<void,
                    Detail::MemFunImpl<void,
                        void (CrowdRiot::*)(EmissionController&)>,
                    CrowdRiot*,
                    Placeholder<0> >(
                    Detail::MemFunImpl<void,
                        void (CrowdRiot::*)(EmissionController&)>(
                        &CrowdRiot::fn_80029D78),
                    this,
                    placeholder0));
            controller->SetUpdateCallback(callback);
        }
        controller->m_uUserData = (u32)this;
        PlayCrowdRiotSound(this);
    }
}

static inline void UnidentifiedInline_800298D8()
{
    CrowdRiot* crowdRiot
        = (CrowdRiot*)g_pGame->mpCrowdRiot;
    if (crowdRiot->meState == 2)
    {
        nlVector3 velocity;
        velocity.x = sUnidentifiedFloat1 * crowdRiot->mv3Velocity.x;
        velocity.y = sUnidentifiedFloat1 * crowdRiot->mv3Velocity.y;
        velocity.z = 0.0f;
        crowdRiot->mv3Velocity = velocity;

        nlVector3 target;
        target.x = sUnidentifiedFloat4
                 * AIsgn(crowdRiot->mv3Position.x);
        float sideline = sUnidentifiedFloat5
                       + cField::GetSidelineY(1U);
        float sign = AIsgn(crowdRiot->mv3Position.y);
        target.y = sign * sideline;
        target.y = -target.y;
        target.z = 0.0f;
        crowdRiot->mv3Target = target;
        crowdRiot->meState = 4;
        crowdRiot->mfStateTime = -1.0f;
    }
}

void fn_800297B8(cBall* ball, CrowdRiot* crowdRiot)
{
    if (ball->mbStuckInRiotDone)
    {
        nlVector3 velocity;
        MakeRandomDirection2D(velocity, 10.0f);
        velocity.z = 10.0f + nlRandomf(5.0f);
        ball->SetVelocity(velocity, SPINTYPE_NONE, 0);
        ball->mtStuckInRiotTimer.UnidentifiedClear();
        ball->mbStuckInRiotDone = false;
    }
    else if (ball->mtStuckInRiotTimer.m_uPackedTime == 0)
    {
        if (ball->m_tNoPickupTimer.m_uPackedTime == 0)
        {
            ball->mtStuckInRiotTimer.SetSeconds(1.0f);
            ball->mbStuckInRiotDone = false;
            ball->m_tNoPickupTimer.SetSeconds(1.25f);
        }
    }
    else
    {
        float time = ball->mtStuckInRiotTimer.GetSeconds();
        nlVector3 position;
        InterpolateRiotBallPosition(position, crowdRiot->mv3Position, ball->m_v3Position, time);
        position.z = 0.18f;
        ball->SetPosition(position);
    }
}

void fn_800298D8(void*)
{
    UnidentifiedInline_800298D8();
}

void fn_800299C4(void*)
{
    UnidentifiedInline_800298D8();
}

void fn_80029AB0(void*)
{
    UnidentifiedInline_800298D8();
}

void fn_80029B9C(void* param)
{
    CollisionCrowdData* event
        = (CollisionCrowdData*)param;
    CrowdRiot* crowdRiot = event->pCrowdRiot;
    PhysicsObject* object = event->pObject;

    switch (object->GetObjectType())
    {
    case 4:
    {
        PhysicsObject* parent = object->m_parentObject;
        cFielder* fielder
            = (cFielder*)((PhysicsCharacter*)parent)->m_pAICharacter;
        if (fielder->m_eClassType == FIELDER
            && fielder->m_eActionState != ACTION_SHOOT_TO_SCORE
            && fielder->m_eActionState != ACTION_SHOT)
        {
            fielder->fn_80043ADC();
        }
        break;
    }
    case 16:
    {
        cBall* ball = ((PhysicsAIBall*)object)->m_pAIBall;
        cPlayer* player = ball->m_pOwner;
        if (player != 0)
        {
            if (player->m_eClassType == FIELDER)
            {
                ((cFielder*)player)->fn_80043ADC();
            }
            else
            {
                static_cast<Goalie*>(player)->FumbleBall();
            }
        }
        else
        {
            fn_800297B8(ball, crowdRiot);
        }
        break;
    }
    case 20:
        break;
    case 21:
        ((PhysicsBanana*)object)->m_pPowerupObject->m_bShouldDestroy = true;
        break;
    case 24:
    case 28:
        break;
    }
}

void fn_80029C80(PhysicsObject*, PhysicsObject* other,
    const nlVector3& position, void* context)
{
    switch (other->GetObjectType())
    {
    case 4:
    case 16:
    case 20:
    case 21:
    case 24:
    case 28:
    {
        CrowdRiot* crowdRiot = (CrowdRiot*)context;
        if (crowdRiot->meState != 1)
        {
            CollisionCrowdData* event = 0;
            g_CollisionCrowdDataPool.Allocate(event);
            event->pObject = other;
            event->v3Position = position;
            event->pCrowdRiot = crowdRiot;
            QueueCollisionCrowdEvent(g_pGame, event);
        }
        break;
    }
    }
}

void CrowdRiot::fn_80029D78(EmissionController& controller)
{
    controller.SetPosition(mv3Position);
}
