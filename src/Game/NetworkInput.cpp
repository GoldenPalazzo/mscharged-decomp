#include "Game/NetworkInput.h"

#include <string.h>

#include "Game/InputManager.h"
#include "Game/NetworkSession.h"
#include "Game/PackedDetInput.h"
#include "NL/globalpad.h"

NetworkPeerChannel* NetworkPeer::GetNetworkPeerChannel(int channel)
{
    return &mChannels[channel];
}

s8 NetworkPeer::GetNetworkPeerMachineId()
{
    return mMachineId;
}

NetworkPeer::NetworkPeer()
    : mMachineId(-1)
    , mPlayerCount(0)
{
}

void NetworkPeer::ResetNetworkPeerInputs()
{
    for (int channel = 0; channel < (int)mPlayerCount; ++channel)
    {
        NetworkPeerChannel* entry = &mChannels[channel];
        entry->mInput0.Reset();
        entry->mInput1.Reset();
        entry->mInput2.Reset();
        entry->mInput3.Reset();
        mChannels[channel].mInput0.m_pMyUser = entry;
        mChannels[channel].mInput1.m_pMyUser = entry;
        mChannels[channel].mInput2.m_pMyUser = entry;
        mChannels[channel].mInput3.m_pMyUser = entry;
        mChannels[channel].mInput1.m_pPrevInput = &entry->mInput0;
        mChannels[channel].mInput3.m_pPrevInput = &entry->mInput2;
    }
}

NetworkPeerChannel::NetworkPeerChannel()
    : mPeer(0)
    , mChannelIndex(-1)
    , mGlobalPadIndex(0)
{
    mInput0.Reset();
    mInput1.Reset();
    mInput2.Reset();
    mInput3.Reset();
    mInput0.m_pMyUser = this;
    mInput1.m_pMyUser = this;
    mInput2.m_pMyUser = this;
    mInput3.m_pMyUser = this;
    mInput1.m_pPrevInput = &mInput0;
    mInput3.m_pPrevInput = &mInput2;
}

void NetworkPeerChannel::Initialize(NetworkPeer* peer, s8 channelIndex, int globalPadIndex)
{
    mPeer = peer;
    mChannelIndex = channelIndex;
    mGlobalPadIndex = globalPadIndex;
    mUnidentified00C = false;
}

DetInput* NetworkPeerChannel::GetNetworkPeerChannelInput()
{
    return &mInput1;
}

s8 NetworkPeerChannel::GetNetworkPeerChannelId()
{
    return mPeer->mMachineId * 4 + mChannelIndex;
}

cGlobalPad* NetworkPeerChannel::GetLocalChannelPad()
{
    if (GetGlobalPadIndex() == -1)
    {
        return 0;
    }
    cGlobalPad* pad;
    if (mPeer == g_pNetworkSessionBase->GetLocalPeer())
        pad = g_pPadManager->GetPad(GetGlobalPadIndex());
    else
        pad = 0;
    return pad;
}

void NetworkPeerChannel::CaptureNetworkPeerChannelInput()
{
    mInput2.CopyState(mInput3);
    cGlobalPad* pad = this->GetLocalChannelPad();
    if (pad != 0 && !gInputManager->mFrameProvider->IsInPauseMenu())
    {
        mInput3.ReadFromPad(pad);
    }
    else
    {
        mInput3.Reset();
    }
    mInput3.m_aRemapAngle
        = gInputManager->mFrameProvider->GetInputRemapAngle();
}

void NetworkPeerChannel::ApplyNetworkPeerChannelInput(const PackedDetInput* record, u16 tick, u8 connected)
{
    mInput0.CopyState(mInput1);
    UnpackDetInput(record, &mInput1);
    mInput1.m_aRemapAngle = tick;
    mInput1.m_nConnected = connected;
    mInput1.UpdatePolarAnalog();
    mInput1.UpdateButtonStateTicks();
}

void NetworkPeerChannel::PackNetworkPeerChannelInput(PackedDetInput* record)
{
    PackDetInput(record, &mInput3);
}

u16 NetworkPeerChannel::GetNetworkPeerChannelRemapAngle()
{
    return mInput3.m_aRemapAngle;
}

u8 NetworkPeerChannel::GetNetworkPeerChannelConnectionStatus()
{
    return mInput3.GetConnectionStatus();
}

s8 GetNetworkPlayerId(s8 player, s8 machine)
{
    return machine * 4 + player;
}

NetMessageInput::NetMessageInput()
{
    mChecksum = 0;
    mFrame = 0;
    mRandomSeed = 0;
    mPlayerStates = 0;
    memset(mRecordChanges, 0, sizeof(mRecordChanges));
    mEventCount = 0;
    mFlags = 0;
    mRemapAngleHigh = 0;
    memset(mRecords, 0, sizeof(mRecords));
}

void NetMessageInput::Reset(bool clearInputs, bool clearHeader)
{
    mChecksum = 0;
    mFrame = 0;
    mRandomSeed = 0;
    mPlayerStates = 0;
    if (clearHeader)
    {
        memset(mRecordChanges, 0,
            sizeof(mRecordChanges));
        mEventCount = 0;
        mFlags = 0;
    }
    else
    {
        mFlags &= 1;
    }
    if (clearInputs)
    {
        mRemapAngleHigh = 0;
        memset(mRecords, 0,
            sizeof(mRecords));
    }
}

void NetMessageInput::CopyFrom(const NetMessageInput* source)
{
    mFlags = source->mFlags;
    mRemapAngleHigh = source->mRemapAngleHigh;
    mPlayerStates = source->mPlayerStates;
    mChecksum = source->mChecksum;
    mFrame = source->mFrame;
    mRandomSeed = source->mRandomSeed;
    mEventCount = source->mEventCount;
    memcpy(mRecordChanges, source->mRecordChanges, 4);
    memcpy(mRecords, source->mRecords, 0x40);
    memcpy(mEvents, source->mEvents, 0x90);
}

void NetMessageInput::SetNetworkInputMessagePlayerState(s8 player, u8 state)
{
    u8 shift = (player & 0x7F) * 2;
    u32 mask = 3 << shift;
    u8 inverse = ~mask;
    u8 value = mPlayerStates & inverse;
    mPlayerStates = value | (state << shift);
}

u8 NetMessageInput::GetNetworkInputMessagePlayerState(s8 player)
{
    return (mPlayerStates >> ((player & 0x7F) * 2)) & 3;
}

void NetMessageInput::SetNetworkInputMessageRecord(s8 player, const PackedDetInput* record)
{
    const u8* source = (const u8*)record;

    if (*(const u16*)(source + 0) != *(u16*)(mRecords[player].mData + 0))
        mRecordChanges[player] |= 2;
    if ((s8)source[12] != (s8)mRecords[player].mData[12]
        || (s8)source[13] != (s8)mRecords[player].mData[13])
        mRecordChanges[player] |= 4;
    if ((s8)source[14] != (s8)mRecords[player].mData[14]
        || (s8)source[15] != (s8)mRecords[player].mData[15])
        mRecordChanges[player] |= 8;
    if ((s8)source[3] != (s8)mRecords[player].mData[3]
        || (s8)source[4] != (s8)mRecords[player].mData[4]
        || (s8)source[5] != (s8)mRecords[player].mData[5])
        mRecordChanges[player] |= 0x10;
    if ((s8)source[6] != (s8)mRecords[player].mData[6]
        || (s8)source[7] != (s8)mRecords[player].mData[7]
        || (s8)source[8] != (s8)mRecords[player].mData[8])
        mRecordChanges[player] |= 0x20;

    if (mRecordChanges[player] != 0)
        mFlags |= 0x10 << player;
    memcpy(mRecords[player].mData, source, 0x10);
}

void NetMessageInput::ApplyNetworkInputMessageRecord(s8 player, PackedDetInput* record)
{
    if (mRecordChanges[player] & 2)
        record->mButtonBitfield = *(u16*)mRecords[player].mData;
    if (mRecordChanges[player] & 4)
    {
        const u8* source = mRecords[player].mData + 12;
        s8 leftX = *source++;
        s8 leftY = *source;
        record->mAnalogAxes[0] = leftX;
        record->mAnalogAxes[1] = leftY;
    }
    if (mRecordChanges[player] & 8)
    {
        const u8* source = mRecords[player].mData + 14;
        s8 rightX = *source++;
        s8 rightY = *source;
        record->mAnalogAxes[2] = rightX;
        record->mAnalogAxes[3] = rightY;
    }
    if (mRecordChanges[player] & 0x10)
    {
        const u8* source = mRecords[player].mData + 3;
        s8 x = *source++;
        s8 y = *source++;
        s8 z = *source;
        record->mRemoteAccel[0] = x;
        record->mRemoteAccel[1] = y;
        record->mRemoteAccel[2] = z;
    }
    if (mRecordChanges[player] & 0x20)
    {
        const u8* source = mRecords[player].mData + 6;
        s8 x = *source++;
        s8 y = *source++;
        s8 z = *source;
        record->mFreeStyleAccel[0] = x;
        record->mFreeStyleAccel[1] = y;
        record->mFreeStyleAccel[2] = z;
    }
}

void NetMessageInput::SetNetworkInputMessageRemapAngle(u16 remapAngle)
{
    u8 value = remapAngle >> 8;
    if (mRemapAngleHigh != value)
    {
        mRemapAngleHigh = value;
        mFlags |= 1;
    }
}

void NetMessageInput::GetNetworkInputMessageRemapAngle(u16* remapAngle)
{
    if (mFlags & 1)
        *remapAngle = mRemapAngleHigh << 8;
}

void NetMessageInput::SetNetworkInputMessageSyncData(u32 checksum, u32 frame, u32 randomSeed)
{
    mChecksum = checksum;
    mFrame = frame;
    mRandomSeed = randomSeed;
    mFlags |= 4;
}

void NetMessageInput::SetNetworkInputMessageCongested(bool congested)
{
    if (congested)
        mFlags |= 8;
    else
        mFlags &= ~8;
}

void NetMessageInput::AddNetworkInputMessageEvent(DetermDataEvent* event)
{
    if (mEventCount < 4)
    {
        mEvents[mEventCount] = *event;
        ++mEventCount;
    }
}

DetermDataEvent* NetMessageInput::GetNetworkInputMessageEvent(int index)
{
    return &mEvents[index];
}

void NetMessageInput::Serialize(
    NetworkMessageSerializer* serializer)
{
    serializer->Transfer(&mFlags, 1);
    serializer->Transfer(&mPlayerStates, 1);
    if (mFlags & 1)
        serializer->Transfer(&mRemapAngleHigh, 1);
    if (mFlags & 4)
    {
        serializer->Transfer(&mChecksum, 4);
        serializer->Transfer(&mFrame, 4);
        serializer->Transfer(&mRandomSeed, 4);
    }
    else if (mFlags & 2)
    {
        serializer->Transfer(&mFrame, 4);
    }
    serializer->Transfer(&mEventCount, 1);
    for (int i = 0; i < mEventCount; ++i)
        mEvents[i].Serialize(serializer);
    if (serializer->mDirection == 0)
    {
        for (s8 i = 0; i < 4; ++i)
        {
            if (mFlags & (0x10 << i))
                serializer->Transfer(&mRecordChanges[i], 1);
        }
    }
    else
    {
        for (s8 i = 0; i < 4; ++i)
        {
            if (mFlags & (0x10 << i))
                serializer->Transfer(&mRecordChanges[i], 1);
        }
    }
    for (s8 i = 0; i < 4; ++i)
    {
        if (mRecordChanges[i] & 2)
            serializer->Transfer(mRecords[i].mData + 0, 2);
        if (mRecordChanges[i] & 4)
        {
            serializer->Transfer(mRecords[i].mData + 12, 1);
            serializer->Transfer(mRecords[i].mData + 13, 1);
        }
        if (mRecordChanges[i] & 8)
        {
            serializer->Transfer(mRecords[i].mData + 14, 1);
            serializer->Transfer(mRecords[i].mData + 15, 1);
        }
        if (mRecordChanges[i] & 0x10)
            serializer->Transfer(mRecords[i].mData + 3, 3);
        if (mRecordChanges[i] & 0x20)
            serializer->Transfer(mRecords[i].mData + 6, 3);
    }
}

void NetMessageInputBundle::Serialize(
    NetworkMessageSerializer* serializer)
{
    mMessage0.Serialize(serializer);
    mMessage1.Serialize(serializer);
}

int NetMessageInputBundle::GetType()
{
    return 1;
}

int NetMessageInput::GetType()
{
    return 0;
}

typedef char VerifyNetworkPeerChannelSize[
    sizeof(NetworkPeerChannel) == 0x240 ? 1 : -1];
typedef char VerifyNetworkPeerSize[
    sizeof(NetworkPeer) == 0x908 ? 1 : -1];
