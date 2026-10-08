#ifndef GAME_DB_PLAYERSTATS_H
#define GAME_DB_PLAYERSTATS_H

#include "types.h"

enum eTeamID
{
    TEAM_INVALID = -1,
};

enum eSidekickID
{
    SK_INVALID = -1,
};

enum eType
{
    TYPE_INVALID = -1,
    TYPE_CHARACTER = 0,
    TYPE_TEAM = 1,
    TYPE_USER = 2,
};

enum ePlayerStats
{
    STATS_INVALID = -1,
    STATS_00 = 0x00,
    STATS_01 = 0x01,
    STATS_02 = 0x02,
    STATS_SHOTS_ON_GOAL = 0x03,
    STATS_04 = 0x04,
    STATS_05 = 0x05,
    STATS_06 = 0x06,
    STATS_07 = 0x07,
    STATS_08 = 0x08,
    STATS_09 = 0x09,
    STATS_0A = 0x0A,
    STATS_GOALS_FOR = 0x0B,
    STATS_GOALS_AGAINST = 0x0C,
    STATS_PASSES_MADE = 0x0D,
    STATS_0E = 0x0E,
    STATS_0F = 0x0F,
    STATS_PASSES_RECEIVED = 0x10,
    STATS_FOULS = 0x11,
    STATS_12 = 0x12,
    STATS_ATTACK_ATTEMPTS = 0x13,
    STATS_ATTACK_SUCCESSES = 0x14,
    STATS_15 = 0x15,
    STATS_16 = 0x16,
    STATS_17 = 0x17,
    STATS_18 = 0x18,
    STATS_19 = 0x19,
    STATS_1A = 0x1A,
    STATS_1B = 0x1B,
    STATS_1C = 0x1C,
    STATS_1D = 0x1D,
    STATS_POWERUPS_USED = 0x1E,
    STATS_WIN = 0x1F,
    STATS_OT_WIN = 0x20,
    STATS_LOSS = 0x21,
    STATS_OT_LOSS = 0x22,
    STATS_PERFECT_PASSES = 0x23,
    STATS_PASSES_INTERCEPTED = 0x24,
    STATS_25 = 0x25,
    STATS_26 = 0x26,
    NUM_STATS = 0x27,
};

enum eSortOrder
{
    SORT_ASCENDING = 0,
    SORT_DESCENDING = 1,
};

union RECORDTYPE
{
    /* 0x0 */ int mCharacterClass;
    /* 0x0 */ eTeamID mTeamID;
    /* 0x0 */ int mControllerID;
};

struct PlayerStats
{
    /* 0x00 */ u16 unknown_0x00;
    /* 0x02 */ u16 unknown_0x02;
    /* 0x04 */ u16 unknown_0x04;
    /* 0x06 */ u16 mNumShotsOnGoal;
    /* 0x08 */ u16 unknown_0x08;
    /* 0x0A */ u16 unknown_0x0A;
    /* 0x0C */ u16 unknown_0x0C;
    /* 0x0E */ u16 unknown_0x0E;
    /* 0x10 */ u16 mNumGoalsFor;
    /* 0x12 */ u16 mNumGoalsAgainst;
    /* 0x14 */ u16 unknown_0x14;
    /* 0x16 */ u16 unknown_0x16;
    /* 0x18 */ u16 unknown_0x18;
    /* 0x1A */ u16 mNumFouls;
    /* 0x1C */ u16 unknown_0x1C;
    /* 0x1E */ u16 mNumPowerupsUsed;
    /* 0x20 */ u16 unknown_0x20;
    /* 0x22 */ u16 unknown_0x22;
    /* 0x24 */ u16 unknown_0x24;
    /* 0x26 */ u16 unknown_0x26;
    /* 0x28 */ u16 unknown_0x28;
    /* 0x2A */ u16 mNumPassesMade;
    /* 0x2C */ u16 unknown_0x2C;
    /* 0x2E */ u16 unknown_0x2E;
    /* 0x30 */ u16 mNumPassesReceived;
    /* 0x32 */ u16 mNumHitsMade;
    /* 0x34 */ u16 unknown_0x34;
    /* 0x36 */ u16 mNumSteals;
    /* 0x38 */ u16 unknown_0x38;
    /* 0x3C */ u32 unknown_0x3C;
    /* 0x40 */ u32 mNumButtonPresses;
    /* 0x44 */ u16 mNumPerfectPasses;
    /* 0x46 */ u16 unknown_0x46;
    /* 0x48 */ u16 unknown_0x48;
    /* 0x4A */ u16 mNumPassesIntercepted;
    /* 0x4C */ RECORDTYPE mRecordType;
    /* 0x50 */ eType mType;
};

inline int GetStatValue(const PlayerStats& stats, ePlayerStats stat)
{
    int value = -1;
    switch (stat)
    {
    case STATS_00: value = stats.unknown_0x00; break;
    case STATS_01: value = stats.unknown_0x02; break;
    case STATS_02: value = stats.unknown_0x04; break;
    case STATS_SHOTS_ON_GOAL: value = stats.mNumShotsOnGoal; break;
    case STATS_05: value = stats.unknown_0x08; break;
    case STATS_06: value = stats.unknown_0x0A; break;
    case STATS_07: value = stats.unknown_0x0C; break;
    case STATS_08: value = stats.unknown_0x0E; break;
    case STATS_GOALS_FOR: value = stats.mNumGoalsFor; break;
    case STATS_GOALS_AGAINST: value = stats.mNumGoalsAgainst; break;
    case STATS_04: value = stats.unknown_0x14; break;
    case STATS_09: value = stats.unknown_0x16; break;
    case STATS_0A: value = stats.unknown_0x18; break;
    case STATS_FOULS: value = stats.mNumFouls; break;
    case STATS_18: value = stats.unknown_0x1C; break;
    case STATS_19: value = stats.mNumPowerupsUsed; break;
    case STATS_1A: value = stats.unknown_0x20; break;
    case STATS_1B: value = stats.unknown_0x22; break;
    case STATS_1C: value = stats.unknown_0x24; break;
    case STATS_1D: value = stats.unknown_0x26; break;
    case STATS_PASSES_MADE: value = stats.mNumPassesMade; break;
    case STATS_0E: value = stats.unknown_0x2C; break;
    case STATS_0F: value = stats.unknown_0x2E; break;
    case STATS_PASSES_RECEIVED: value = stats.mNumPassesReceived; break;
    case STATS_12: value = stats.mNumHitsMade; break;
    case STATS_ATTACK_ATTEMPTS: value = stats.unknown_0x34; break;
    case STATS_ATTACK_SUCCESSES: value = stats.mNumSteals; break;
    case STATS_15: value = stats.unknown_0x38; break;
    case STATS_16: value = stats.unknown_0x3C; break;
    case STATS_17: value = stats.mNumButtonPresses; break;
    case STATS_PERFECT_PASSES: value = stats.mNumPerfectPasses; break;
    case STATS_25: value = stats.unknown_0x46; break;
    case STATS_26: value = stats.unknown_0x48; break;
    case STATS_POWERUPS_USED: value = stats.unknown_0x28; break;
    case STATS_PASSES_INTERCEPTED: value = stats.mNumPassesIntercepted; break;
    }
    return value;
}


#endif // GAME_DB_PLAYERSTATS_H
