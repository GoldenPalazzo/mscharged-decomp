#ifndef GAME_AI_TEAM_PLAY_MACHINE_H
#define GAME_AI_TEAM_PLAY_MACHINE_H

#include "Game/AI/Desire.h"
#include "Game/AI/TransitionFunc.h"
#include "Game/AI/Variant.h"

class cTeam;
class ScriptQuestionCache;
class UnidentifiedVariantCollection;

class ScriptMachine
{
public:
    ScriptMachine(
        int, bool, AIContext*, const char*);
    virtual ~ScriptMachine();

    virtual bool IsIdle() const
    {
        return mActiveState == 0;
    }
    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void Reset(bool);
    virtual shdStateMachine* ActivateState(
        int, UnidentifiedVariantCollection*, bool);
    virtual void DeactivateState();
    virtual void SelectState();
    virtual void OnBudgetCheckFailed()
    {
    }

    void SetTransition(const char* name)
    {
        ScriptTransitionFunc transition(name);
        mTransition = transition.mValue;
    }

    void AddState(int, shdStateMachine*, bool);

    shdStateMachine* GetState(int state) const
    {
        if (state >= 0 && state < mStateCount)
        {
            return mStates[state];
        }
        return 0;
    }

    shdStateMachine* GetActiveState() const { return mActiveState; }
    FuzzyRuntimeBase* GetFuzzyRuntime();

    shdStateMachine* mActiveState;
    shdStateMachine* mPreviousState;
    UnsetTransitionFunc mTransition;
    int mPendingState;
    UnidentifiedVariantCollection mPendingParameters;
    AIContext* mAIContext;
    bool mOwnsStates;
    u8 mPadding069[3];
    shdStateMachine** mStates;
    shdStateMachine** mConcurrentStates;
    int mStateCount;
    char mName[0x40];
};

class TeamPlayMachine : public ScriptMachine
{
public:
    TeamPlayMachine();
    virtual ~TeamPlayMachine();

    virtual void Initialize();
    virtual void Update(float deltaTime);
    virtual void SelectState();
};

class TeamDesire : public shdStateMachine
{
public:
    TeamDesire(
        int state, TransitionFunc& transition);
    virtual ~TeamDesire()
    {
    }

    virtual bool Initialize(void*) = 0;
    virtual bool Reinitialize(void*);
    virtual void Cleanup();
    virtual void Update(DesireUpdate*, float);
    virtual void SetContext(ScriptMachine*);

protected:
    cTeam* m_pTeam;
};

class TutorialMegastrikeDesire : public TeamDesire
{
public:
    TutorialMegastrikeDesire(
        int state, TransitionFunc transition)
        : TeamDesire(state, transition)
    {
    }

    virtual ~TutorialMegastrikeDesire();

    virtual bool Initialize(void*);
    virtual void Cleanup();
    virtual void Update(DesireUpdate*, float);
};

class cFielder;
ScriptMachine* fn_8002E1A4(cFielder* pFielder);


void DeactivateScriptMachineState(ScriptMachine* machine, shdStateMachine* state);
AIContext* GetScriptMachineAIContext(ScriptMachine* machine);
bool HasStateMachineTimedOut(const shdStateMachine* machine);
UnidentifiedVariant_80054AB8 ExecuteScriptStateFunction(FuzzyRuntimeBase* runtime, const u32& hash, void* argument);
UnidentifiedVariant_80054AB8 ExecuteScriptStateFunction(FuzzyRuntimeBase* runtime, const u32& hash, void* argument, float value);
void QueueScriptMachineState(ScriptMachine* machine, int state, const UnidentifiedVariantCollection* parameters);

extern const float gStateMachineZeroDuration;
extern const float gStateMachineUnsetDuration;
extern const float gStateMachineNeverActiveTime;
extern const float gStateMachineAgeThreshold;

bool IsTransitionFuncSet(const TransitionFunc* transition);
bool HasTransitionFunc(const TransitionFunc* transition);
void AddScriptState( ScriptMachine* machine, int state, const char* name, bool secondary);
void DeactivateConcurrentStates(ScriptMachine* machine);
void DeactivateScriptMachine(ScriptMachine* machine);
void DeactivateConcurrentState(ScriptMachine* machine, int state);
shdStateMachine* ActivateConcurrentState(ScriptMachine* machine, int state, UnidentifiedVariantCollection* parameters, bool reinitialize);
shdStateMachine* GetScriptMachineState( ScriptMachine* machine, int state);
shdStateMachine* GetConcurrentState(ScriptMachine* machine, int state);
bool IsConcurrentStateActive(ScriptMachine* machine, int state);
extern "C" void fn_8031A02C(ScriptQuestionCache*);
bool CheckScriptTimeBudget();
float AccumulateScriptExecutionTime(float start, float end);
extern "C" void fn_8031A0FC(float value);

#endif // GAME_AI_TEAM_PLAY_MACHINE_H
