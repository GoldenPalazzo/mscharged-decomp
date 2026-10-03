namespace
{
static void* byteCode;
}

const char* NisPlayer::GetTargetFilter(NisTarget target, NisWinnerType winnerType) const
{
    if (target == NIS_TARGET_STADIUM)
    {
        int stadium = GameInfoManager::Instance()->GetStadium();
        const char* stadiumName = GetStadiumName(stadium);
        return stadiumName;
    }

    if (target == NIS_TARGET_HOME_CAPTAIN)
    {
        return GetTeamName((eTeamID)GameInfoManager::Instance()->GetTeam(0));
    }

    if (target == NIS_TARGET_AWAY_CAPTAIN)
    {
        return GetTeamName((eTeamID)GameInfoManager::Instance()->GetTeam(1));
    }

    if (target == NIS_TARGET_HOME_SIDEKICK)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(0, 0));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_5)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(0, 0));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_6)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(0, 1));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_7)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(0, 2));
    }

    if (target == NIS_TARGET_AWAY_SIDEKICK)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(1, 0));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_9)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(1, 0));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_10)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(1, 1));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_11)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(1, 2));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_14)
    {
        return g_pCharacters[mGoalScorerCharIndex]->mUnidentified11C->mName;
    }

    if (target == NIS_TARGET_WINNER_SIDEKICK)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick((short)fn_8027E284(winnerType), 0));
    }

    if (target == NIS_TARGET_LOSER_SIDEKICK)
    {
        int side = (fn_8027E284(winnerType) + 1) % 2;
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick((short)side, 0));
    }

    if (target == NIS_TARGET_WINNER_CAPTAIN)
    {
        return GetTeamName((eTeamID)GameInfoManager::Instance()->GetTeam((short)fn_8027E284(winnerType)));
    }

    if (target == NIS_TARGET_LOSER_CAPTAIN)
    {
        int side = (fn_8027E284(winnerType) + 1) % 2;
        return GetTeamName((eTeamID)GameInfoManager::Instance()->GetTeam((short)side));
    }

    if (target == NIS_TARGET_UNIDENTIFIED_21)
    {
        return GetTeamName((eTeamID)GameInfoManager::Instance()->GetTeam((short)mMegaStrikeSide));
    }

    if (target == NIS_TARGET_HOME_GOALIE || target == NIS_TARGET_AWAY_GOALIE || target == NIS_TARGET_WINNER_GOALIE || target == NIS_TARGET_LOSER_GOALIE || target == NIS_TARGET_UNIDENTIFIED_22)
    {
        return "goalie";
    }

    if (target == NIS_TARGET_AWAY_SIDEKICK)
    {
        return GetSidekickName((eSidekickID)GameInfoManager::Instance()->GetSidekick(1, 0));
    }

    return "";
}

static inline void FormatNisName(char* fullName, const char* filter, const char* nisType, NisUseFilter useFilter, const char* extraNameFilter)
{
    char prefix[64];
    if (nlStrCmp(filter, "") != 0)
    {
        nlSNPrintf(prefix, sizeof(prefix), "%s_", filter);
    }
    else
    {
        prefix[0] = '\0';
    }

    char extra[64];
    if (useFilter != NIS_NO_FILTER)
    {
        nlSNPrintf(extra, sizeof(extra), "_%s", extraNameFilter);
    }
    else
    {
        extra[0] = '\0';
    }

    nlSNPrintf(fullName, 64, "%s%s%s", prefix, nisType, extra);
}

static inline int RandomNisIndex(int count, unsigned int* seed)
{
    float value = (count - 1) * nlRandomf(1.0f, seed);
    value += value < 0.0f ? -0.5f : 0.5f;
    return (int)value;
}

static inline void PlayNisCue(NisPlayer* player, const char* nisName)
{
    char cueName[128];
    nlStrNCpy(cueName, nisName, sizeof(cueName));
    unsigned long length = nlStrLen(cueName);
    if (GetStadiumUnknown0x10(GameInfoManager::Instance()->GetStadium()))
    {
        cueName[length - 4] = '\0';
    }
    else
    {
        nlStrNCpy(cueName + length - 4, "_nocrowd", sizeof(cueName) - length - 4);
    }
    player->PrepareNisCue(nlStringLowerHash(cueName));
}

void NisPlayer::Load(const char* nisType, NisTarget target, NisUseStadiumOffset useStadiumOffset, NisUseFilter useFilter, NisWinnerType winnerType, int param5, int param6)
{
    char fullName[64];
    mActive = true;

    const char* filter = GetTargetFilter(target, winnerType);
    FormatNisName(fullName, filter, nisType, useFilter, mExtraNameFilter);

    int numAvailableNis = 0;
    NisHeader* availableNis[10] = { 0 };
    int dictionaryIndex;
    for (dictionaryIndex = 0; dictionaryIndex < mDictSize && numAvailableNis < 10; dictionaryIndex++)
    {
        if (nlStrNICmp(mDict[dictionaryIndex].name, fullName, nlStrLen(fullName)) != 0)
        {
            continue;
        }
        NisHeader* candidate = &mDict[dictionaryIndex];
        if (strstr(candidate->name, "_same") != NULL)
        {
            continue;
        }
        if (strstr(candidate->name, "_other") != NULL)
        {
            continue;
        }
        availableNis[numAvailableNis++] = candidate;
    }

    if (numAvailableNis == 0)
    {
        return;
    }

    int index;
    if (param6 >= 0)
    {
        index = param6 % numAvailableNis;
    }
    else
    {
        index = RandomNisIndex(numAvailableNis, &GetPresentation()->mRandomSeed);
        if (DuringGoalCelebration(GetPresentation()) && param5 == 0)
        {
            if (numAvailableNis > 1 && mUnidentified343F4 == index && nlStrCmp(mUnidentified343F8, mExtraNameFilter) == 0)
            {
                while (index == mUnidentified343F4)
                {
                    index = RandomNisIndex(numAvailableNis, &GetPresentation()->mRandomSeed);
                }
            }
            mUnidentified343F4 = index;
            nlStrNCpy(mUnidentified343F8, mExtraNameFilter, sizeof(mUnidentified343F8));
        }
    }

    NisHeader& nisHeader = *availableNis[index];
    fn_802805B4(nisHeader, target, useStadiumOffset, winnerType, param5, false);

    NisTarget sameTarget = NIS_TARGET_NONE;
    NisTarget otherTarget = NIS_TARGET_NONE;
    switch (target)
    {
    case NIS_TARGET_HOME_CAPTAIN:
    case NIS_TARGET_HOME_GOALIE:
        sameTarget = NIS_TARGET_HOME_SIDEKICK;
        otherTarget = NIS_TARGET_AWAY_SIDEKICK;
        break;
    case NIS_TARGET_AWAY_CAPTAIN:
    case NIS_TARGET_AWAY_GOALIE:
        sameTarget = NIS_TARGET_AWAY_SIDEKICK;
        otherTarget = NIS_TARGET_HOME_SIDEKICK;
        break;
    case NIS_TARGET_LOSER_CAPTAIN:
    case NIS_TARGET_LOSER_GOALIE:
        sameTarget = NIS_TARGET_LOSER_SIDEKICK;
        otherTarget = NIS_TARGET_WINNER_SIDEKICK;
        break;
    case NIS_TARGET_WINNER_CAPTAIN:
    case NIS_TARGET_WINNER_GOALIE:
        sameTarget = NIS_TARGET_WINNER_SIDEKICK;
        otherTarget = NIS_TARGET_LOSER_SIDEKICK;
        break;
    case NIS_TARGET_UNIDENTIFIED_14:
    {
        cCharacter* character = g_pCharacters[mGoalScorerCharIndex];
        if (character != NULL && character->IsCaptain())
        {
            if (((cPlayer*)character)->m_pTeam->m_nSide == 0)
            {
                sameTarget = NIS_TARGET_HOME_SIDEKICK;
                otherTarget = NIS_TARGET_AWAY_SIDEKICK;
            }
            else
            {
                sameTarget = NIS_TARGET_AWAY_SIDEKICK;
                otherTarget = NIS_TARGET_HOME_SIDEKICK;
            }
        }
        break;
    }
    case NIS_TARGET_UNIDENTIFIED_22:
    {
        cCharacter* character = g_pCharacters[mGoalScorerCharIndex];
        if (character != NULL && character->IsCaptain())
        {
            if (((cPlayer*)character)->m_pTeam->m_nSide == 0)
            {
                sameTarget = NIS_TARGET_AWAY_SIDEKICK;
                otherTarget = NIS_TARGET_HOME_SIDEKICK;
            }
            else
            {
                sameTarget = NIS_TARGET_HOME_SIDEKICK;
                otherTarget = NIS_TARGET_AWAY_SIDEKICK;
            }
        }
        break;
    }
    case NIS_TARGET_UNIDENTIFIED_21:
    {
        cCharacter* character = g_pCharacters[mGoalScorerCharIndex];
        if (character != NULL && character->IsCaptain())
        {
            if (((cPlayer*)character)->m_pTeam->m_nSide == 0)
            {
                sameTarget = NIS_TARGET_HOME_SIDEKICK;
                otherTarget = NIS_TARGET_AWAY_SIDEKICK;
            }
            else
            {
                sameTarget = NIS_TARGET_AWAY_SIDEKICK;
                otherTarget = NIS_TARGET_HOME_SIDEKICK;
            }
        }
        break;
    }
    }

    if (sameTarget != NIS_TARGET_NONE)
    {
        fn_8028041C(nisHeader.name, "same", sameTarget, useStadiumOffset, winnerType, nisHeader.mirrored, param5);
    }
    if (otherTarget != NIS_TARGET_NONE)
    {
        fn_8028041C(nisHeader.name, "other", otherTarget, useStadiumOffset, winnerType, nisHeader.mirrored, param5);
    }

    if (param5 != 1 && mUnidentified34354 == 0)
    {
        PlayNisCue(this, nisHeader.name);
    }
}

void NisPlayer::fn_8028041C(const char* param1, const char* param2, NisTarget target, NisUseStadiumOffset useStadiumOffset, NisWinnerType winnerType, bool param5, int param6)
{
    char baseName[64];
    int length = nlStrChr(param1, '.') - param1 + 1;
    nlStrNCpy(baseName, param1, nlMin((int)sizeof(baseName), length));

    const char* filter = GetTargetFilter(target, winnerType);
    char fullName[64];
    nlSNPrintf(fullName, sizeof(fullName), "%s_%s_%s.nis", baseName, filter, param2);

    NisHeader* nisHeader = NULL;
    for (int dictionaryIndex = 0; dictionaryIndex < mDictSize; dictionaryIndex++)
    {
        if (nlStrCmp(mDict[dictionaryIndex].name, fullName) == 0)
        {
            mDict[dictionaryIndex].mirrored = param5;
            nisHeader = &mDict[dictionaryIndex];
            break;
        }
    }

    if (nisHeader != NULL)
    {
        fn_802805B4(*nisHeader, target, useStadiumOffset, winnerType, param6, true);
    }
}

void NisPlayer::fn_802805B4(NisHeader& nisHeader, NisTarget target, NisUseStadiumOffset useStadiumOffset, NisWinnerType winnerType, int param5, bool param6)
{
    nisHeader.target = target;
    nisHeader.winnerType = winnerType;
    nisHeader.mTime = 0.0f;
    nisHeader.unknown_0x180 = param5;
    if (!param6)
    {
        nisHeader.mirrored = IsMirrored(target, nisHeader.name, winnerType);
    }
    if (useStadiumOffset == NIS_NO_STADIUM_OFFSET)
    {
        nisHeader.stadiumOffset.x = 0.0f;
        nisHeader.stadiumOffset.y = 0.0f;
        nisHeader.stadiumOffset.z = 0.0f;
    }
    else
    {
        float scale = (useStadiumOffset == NIS_AWAY_STADIUM_OFFSET) ? -1.0f : 1.0f;
        char offsetConfigName[64];
        nlSNPrintf(offsetConfigName, 64, "nisHeader/%s_offset", GetStadiumName(GameInfoManager::Instance()->GetStadium()));
        float offset = scale * GetConfigFloat(Config::Global(), offsetConfigName, 0.0f);
        nisHeader.stadiumOffset.x = 0.0f;
        nisHeader.stadiumOffset.y = offset;
        nisHeader.stadiumOffset.z = 0.0f;
    }
    for (int i = 0; i < nisHeader.numAnimations; i++)
    {
        mBeginPositions[i] = nisHeader.beginPositions[i];
        if (nisHeader.mirrored)
        {
            mBeginPositions[i].x *= -1.0f;
        }
    }
    if (nisHeader.buffer != NULL)
    {
        Load(nisHeader.buffer, nisHeader.bufferSize, nisHeader);
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            if (mLoadQueue[i] == NULL)
            {
                mLoadQueue[i] = &nisHeader;
                break;
            }
        }
    }
}
