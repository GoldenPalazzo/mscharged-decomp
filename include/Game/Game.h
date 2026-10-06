#ifndef GAME_GAME_H
#define GAME_GAME_H

#include "Game/GameEventQueue.h"
#include "Game/NetworkMessage.h"
#include "types.h"
#include "NL/nlMath.h"
#include "NL/CircularQueue.h"

class Clock;
class FuzzyTweaks;
class DebugWriteCache;
class WeatherManager;
class RunningChecksum;
class nlPolygonRegion;
class Terrain;
class CrowdRiot;
class AvoidablePolygon;
class AIContext;
struct DetermDataEvent;
struct CharacterImpactEvent;
struct GoalieSaveData;
struct PlayerAttackData;
struct CollisionPlayerWallData;
struct PeachPhotoData;
class cFielder;
class cPlayer;

void DestroyPowerups();
void DestroyGame();
void FreeCollisionPlayerWallData(CollisionPlayerWallData* node);
extern "C" void fn_8005B330(nlVector3*, float, float);

void SetFieldTilt(int relative, float xTilt, float yTilt);

class cGame;
// MegastrikeEnd: retail passes g_pGame in r3, so this is a cGame member in
// all but name.
extern "C" void fn_8005DB7C(cGame* pGame);

class cGame : public NetworkMessageReceiver
{
    friend void SetFieldTilt(int relative, float xTilt, float yTilt);
    friend void fn_8005DB7C(cGame* pGame);

public:
    void QueueCharacterElectrocuted(CollisionPlayerWallData* data);

    virtual int ProcessMessage(NetworkMessage* message);
    virtual ~cGame();

    cGame(void* param1, int param2, bool param3);
    void ResetMegaStrikeMeterQueues();
    void SendMegaStrikeMeter(bool param1);
    void SendRemainingMegaStrikeMeter();
    void InitMegaStrikeGameplay();
    void CleanupMegaStrikeGameplay();
    void ClearMegaStrikeCleanupPending();
    void ResetGameFields();
    void RegisterEventListeners();
    void BeginGame(bool bRematch, bool bStraightToKickoff);
    void CheckForGoal();
    void ReceiveCustomDetermData(DetermDataEvent* data);
    void OnSuddenDeath();
    void OnGameOver();
    void SendPauseGameEvent();
    void SendResumingGameEvent();
    void SetMegaStrikeGameplay(bool param1, int param2, int param3);
    void StartSlowDown(float timeScale, float transitionTime);
    float GetNormalizedGameTime();
    float GetGameTime();
    float GetGameDuration() const { return m_fGameDuration; }
    void fn_800586C0();
    void fn_80058704();
    void ResetForKickOff();
    void fn_80058A78(float seconds);
    void BlowUpPowerups(
        const nlPolygonRegion& region,
        float fExplosionRadius);
    void ResetPowerups(bool clearPowerUps);
    void ResetCachedPlayerDistances();
    void CopyPlayerDistanceSnapshot(void* param1);
    void SendNISLoadedCustomDeterm(u8 param1);
    void SendMegaStrike(int param1, int param2, float param3, float param4);
    void SendMegaStrikePlayerReady();
    void SendMegaStrikeKillCursor();
    void SendMegaStrikeGoalie(unsigned int param1, unsigned int param2, float param3);
    void SendSlowDownEnd();
    void PreUpdate(float deltaTime);
    void RandomizePlayerUpdateOrder();
    void ResetCharacters();
    void SendPlayerVisibility();
    void UpdateCachedGameData(float fDeltaT);
    float fn_8005B748(int param1, int param2);
    cPlayer* GetClosestPlayer(int param1, int param2, int param3);
    void SetPotentialScorer(cPlayer* pPlayer);
    void ChecksumState(RunningChecksum* runningChecksum);
    static void UpdatePowerUpObjects(float fDeltaT);
    void Update(float fDeltaT);
    void SyncLog(void* checksum, DebugWriteCache* cache);
    void ChangeGameState(int state);
    void InitGameState(int state);
    void LoadTerrain(int terrain);
    void SetDifficulty(int diff0, int diff1, int diff2, bool param4);
    void SetMegaStrikeSaveResult(int param1, bool param2);
    void ResumeAfterPresentation();
    void fn_8005E130(NISData* pData);

    inline bool IsGameplayOrOvertime()
    {
        return (m_eGameState == 5 || m_eGameState == 6);
    }

    inline int GetGameState() const { return m_eGameState; }
    inline bool IsLastTeamToScore(int side) const { return m_nLastTeamToScore == side; }
    inline bool IsCaptainShotToScoreOn() const { return mbCaptainShotToScoreOn; }
    inline u32 GetMegaStrikeSaveMask() const { return m_uMegastrikeResults; }
    float GetXAxisTilt() const { return mfXTilt; }
    float GetYAxisTilt() const { return mfYTilt; }
    const nlVector3& GetTiltDirection() const { return mTiltDirection; }

    /* 0x04 */ FuzzyTweaks* m_pFuzzyTweaks;
    /* 0x08 */ Clock* m_pGameClock;
    /* 0x0C */ Clock* m_pPostResetClock;
    /* 0x10 */ Clock* m_pPostGameDoneClock;

private:
    inline void RegisterDetermGameFields(DebugWriteCache* cache);
    static void PlayEndGamePresentation();

    /* 0x14 */ AIContext* mpAIContext;

public:
    /* 0x18 */ int m_eGameState;
    /* 0x1C */ float m_fGameDuration;
    /* 0x20 */ bool m_bBallInNet;

private:
    /* 0x21 */ u8 mPad21[0x03];

public:
    /* 0x24 */ int m_nLastTeamToScore;

public:
    /* 0x28 */ u32 m_uMegastrikeNumShots;
    /* 0x2C */ u32 m_uMegastrikeCurShot;

public:
    /* 0x30 */ u32 m_uMegastrikeGoals;

private:
    /* 0x34 */ int m_nMegastrikeDefendingTeam;

public:
    /* 0x38 */ u32 m_uMegastrikeResults;

public:
    /* 0x3C */ cFielder* mpMegaStrikeShooter;

private:
public:
    /* 0x40 */ bool mbCaptainShotToScoreOn;
    /* 0x41 */ bool mbMegaStrikePositiveNet;
    /* 0x42 */ bool mbMegaStrikePlayerReady;

private:
    /* 0x43 */ u8 mPad43;

public:
    /* 0x44 */ cPlayer* m_pScorer;
    /* 0x48 */ cPlayer* m_pAssister;
    /* 0x4C */ cPlayer* m_pTeamTouch[2];
    /* 0x54 */ cPlayer* m_pRandomPlayersArray[10];

private:
    /* 0x7C */ float mfTiltLevelTimer;

public:
    /* 0x80 */ float mfXTilt;
    /* 0x84 */ float mfYTilt;

private:
    /* 0x88 */ float mfXTiltMin;
    /* 0x8C */ float mfXTiltMax;
    /* 0x90 */ float mfYTiltMin;
    /* 0x94 */ float mfYTiltMax;
    /* 0x98 */ float mfTiltSpeed;
    /* 0x9C */ float mfDesiredTiltSpeed;

public:
    /* 0xA0 */ float mUnidentified0A0;

private:
    /* 0xA4 */ u16 maTiltDir;
    /* 0xA6 */ u16 maDesiredTiltDir;
    /* 0xA8 */ u32 muTiltFrames;
    /* 0xAC */ nlVector3 mTiltDirection;
    /* 0xB8 */ u32 muCountdownBeepsRemaining;

public:
    /* 0xBC */ bool mbMegaStrikePlayerReadySent;
    /* 0xBD */ bool mbMegaStrikeCleanupPending;

private:
    /* 0xBE */ u8 mPadBE[0x02];

public:
    /* 0xC0 */ CircularQueueBase<bool> mReceivedMegaStrikeMeter;

private:
    /* 0xD0 */ u8 mReceivedMegaStrikeMeterStorage[0x64];

public:
    /* 0x134 */ CircularQueueBase<bool> mPendingMegaStrikeMeter;

private:
    /* 0x144 */ u8 mPendingMegaStrikeMeterStorage[0x10];

public:
    /* 0x154 */ cPlayer* m_nClosestPlayers[10][2][5];
    /* 0x2E4 */ float m_fCachedPlayerDistances[10][10];
    /* 0x474 */ float m_fCachedBallPlayerDistances[10];

public:
    void QueueChainNisEnd(ShotAtGoalData* data);
    void fn_80060BFC(CollisionBulletBillData& data);

    /* 0x49C */ UnidentifiedGameEventQueue mUnidentified49C;

public:
    /* 0x10D8 */ Terrain* mpTerrain;
    /* 0x10DC */ WeatherManager* mpWeatherManager;
    /* 0x10E0 */ CrowdRiot* mpCrowdRiot;
    /* 0x10E4 */ AvoidablePolygon* mpBoundaryAvoidables[4];
};

extern cGame* g_pGame;

extern "C" void fn_8006040C(cGame*, cFielder*);
extern "C" void fn_80060804(cGame*, cFielder*);

extern "C" void fn_8005D210(cGame*, LightningStrikeData*);
extern "C" void fn_8005D354(cGame* pGame, const GoalieSaveData* pData);
extern "C" void fn_8005D550(cGame* pGame, const GoalieSaveData* pData);
extern "C" void fn_8005D948(cGame* pGame, const GoalieSaveData* pData);
extern "C" void fn_8005E408(cGame* pGame, const PlayerAttackData* pData);
extern "C" void fn_8005E800(cGame* pGame, const PlayerAttackData* pData);
extern "C" void fn_8005E604(cGame* pGame, const PlayerAttackData* pData);
extern "C" void fn_8005E9FC(cGame* pGame, const PlayerAttackData* pData);

extern "C" void fn_8005D74C(cGame* game, const GoalieSaveData* pSaveData);

extern "C" void fn_8005FA2C(cGame* pGame);
extern "C" void fn_8005FC1C(cGame* pGame, PeachPhotoData* pEventData);
extern "C" void fn_8005FE18(cGame* pGame, PeachPhotoData* pEventData);
extern "C" void fn_80060014(cGame* pGame, PeachPhotoData* pEventData);
extern "C" void fn_80060210(cGame* pGame, PeachPhotoData* pEventData);
extern "C" void fn_80060A00(cGame* pGame, cFielder* pFielder);
extern "C" void fn_80060FF4(cGame* pGame, const CharacterImpactEvent* pEventData);

extern "C" void fn_800611F0(cGame* pGame, const void* pEventData);

#endif // GAME_GAME_H
