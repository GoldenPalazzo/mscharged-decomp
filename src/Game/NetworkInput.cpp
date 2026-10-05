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
    mUnidentified00C = 0;
    mUnidentified010 = 0;
    mUnidentified014 = 0;
    mUnidentified00A = 0;
    memset(mUnidentified018, 0, sizeof(mUnidentified018));
    mUnidentified05C = 0;
    mUnidentified008 = 0;
    mUnidentified009 = 0;
    memset(mUnidentified01C, 0, sizeof(mUnidentified01C));
}

void NetMessageInput::Reset(bool clearInputs, bool clearHeader)
{
    mUnidentified00C = 0;
    mUnidentified010 = 0;
    mUnidentified014 = 0;
    mUnidentified00A = 0;
    if (clearHeader)
    {
        memset(mUnidentified018, 0,
            sizeof(mUnidentified018));
        mUnidentified05C = 0;
        mUnidentified008 = 0;
    }
    else
    {
        mUnidentified008 &= 1;
    }
    if (clearInputs)
    {
        mUnidentified009 = 0;
        memset(mUnidentified01C, 0,
            sizeof(mUnidentified01C));
    }
}

void NetMessageInput::CopyFrom(const NetMessageInput* source)
{
    mUnidentified008 = source->mUnidentified008;
    mUnidentified009 = source->mUnidentified009;
    mUnidentified00A = source->mUnidentified00A;
    mUnidentified00C = source->mUnidentified00C;
    mUnidentified010 = source->mUnidentified010;
    mUnidentified014 = source->mUnidentified014;
    mUnidentified05C = source->mUnidentified05C;
    memcpy(mUnidentified018, source->mUnidentified018, 4);
    memcpy(mUnidentified01C, source->mUnidentified01C, 0x40);
    memcpy(mDetermData, source->mDetermData, 0x90);
}

void NetMessageInput::SetNetworkInputMessagePlayerState(s8 player, u8 state)
{
    u8 shift = (player & 0x7F) * 2;
    u32 mask = 3 << shift;
    u8 inverse = ~mask;
    u8 value = mUnidentified00A & inverse;
    mUnidentified00A = value | (state << shift);
}

u8 NetMessageInput::GetNetworkInputMessagePlayerState(s8 player)
{
    return (mUnidentified00A >> ((player & 0x7F) * 2)) & 3;
}

void NetMessageInput::SetNetworkInputMessageRecord(s8 player, const PackedDetInput* record)
{
    const u8* source = (const u8*)record;

    if (*(const u16*)(source + 0) != *(u16*)(mUnidentified01C[player].mData + 0))
        mUnidentified018[player] |= 2;
    if ((s8)source[12] != (s8)mUnidentified01C[player].mData[12]
        || (s8)source[13] != (s8)mUnidentified01C[player].mData[13])
        mUnidentified018[player] |= 4;
    if ((s8)source[14] != (s8)mUnidentified01C[player].mData[14]
        || (s8)source[15] != (s8)mUnidentified01C[player].mData[15])
        mUnidentified018[player] |= 8;
    if ((s8)source[3] != (s8)mUnidentified01C[player].mData[3]
        || (s8)source[4] != (s8)mUnidentified01C[player].mData[4]
        || (s8)source[5] != (s8)mUnidentified01C[player].mData[5])
        mUnidentified018[player] |= 0x10;
    if ((s8)source[6] != (s8)mUnidentified01C[player].mData[6]
        || (s8)source[7] != (s8)mUnidentified01C[player].mData[7]
        || (s8)source[8] != (s8)mUnidentified01C[player].mData[8])
        mUnidentified018[player] |= 0x20;

    if (mUnidentified018[player] != 0)
        mUnidentified008 |= 0x10 << player;
    memcpy(mUnidentified01C[player].mData, source, 0x10);
}

void NetMessageInput::ApplyNetworkInputMessageRecord(s8 player, PackedDetInput* record)
{
    if (mUnidentified018[player] & 2)
        record->mButtonBitfield = *(u16*)mUnidentified01C[player].mData;
    if (mUnidentified018[player] & 4)
    {
        const u8* source = mUnidentified01C[player].mData + 12;
        s8 leftX = *source++;
        s8 leftY = *source;
        record->mAnalogAxes[0] = leftX;
        record->mAnalogAxes[1] = leftY;
    }
    if (mUnidentified018[player] & 8)
    {
        const u8* source = mUnidentified01C[player].mData + 14;
        s8 rightX = *source++;
        s8 rightY = *source;
        record->mAnalogAxes[2] = rightX;
        record->mAnalogAxes[3] = rightY;
    }
    if (mUnidentified018[player] & 0x10)
    {
        const u8* source = mUnidentified01C[player].mData + 3;
        s8 x = *source++;
        s8 y = *source++;
        s8 z = *source;
        record->mRemoteAccel[0] = x;
        record->mRemoteAccel[1] = y;
        record->mRemoteAccel[2] = z;
    }
    if (mUnidentified018[player] & 0x20)
    {
        const u8* source = mUnidentified01C[player].mData + 6;
        s8 x = *source++;
        s8 y = *source++;
        s8 z = *source;
        record->mFreeStyleAccel[0] = x;
        record->mFreeStyleAccel[1] = y;
        record->mFreeStyleAccel[2] = z;
    }
}

void NetMessageInput::SetNetworkInputMessageRemapAngle(u16 tick)
{
    u8 value = tick >> 8;
    if (mUnidentified009 != value)
    {
        mUnidentified009 = value;
        mUnidentified008 |= 1;
    }
}

void NetMessageInput::GetNetworkInputMessageRemapAngle(u16* tick)
{
    if (mUnidentified008 & 1)
        *tick = mUnidentified009 << 8;
}

void NetMessageInput::SetNetworkInputMessageSyncData(u32 checksum, u32 frame, u32 randomSeed)
{
    mUnidentified00C = checksum;
    mUnidentified010 = frame;
    mUnidentified014 = randomSeed;
    mUnidentified008 |= 4;
}

void NetMessageInput::SetNetworkInputMessageCongested(bool congested)
{
    if (congested)
        mUnidentified008 |= 8;
    else
        mUnidentified008 &= ~8;
}

void NetMessageInput::AddNetworkInputMessageEvent(DetermDataEvent* event)
{
    if (mUnidentified05C < 4)
    {
        mDetermData[mUnidentified05C] = *event;
        ++mUnidentified05C;
    }
}

DetermDataEvent* NetMessageInput::GetNetworkInputMessageEvent(int index)
{
    return &mDetermData[index];
}

void NetMessageInput::Serialize(
    NetworkMessageSerializer* serializer)
{
    serializer->Transfer(&mUnidentified008, 1);
    serializer->Transfer(&mUnidentified00A, 1);
    if (mUnidentified008 & 1)
        serializer->Transfer(&mUnidentified009, 1);
    if (mUnidentified008 & 4)
    {
        serializer->Transfer(&mUnidentified00C, 4);
        serializer->Transfer(&mUnidentified010, 4);
        serializer->Transfer(&mUnidentified014, 4);
    }
    else if (mUnidentified008 & 2)
    {
        serializer->Transfer(&mUnidentified010, 4);
    }
    serializer->Transfer(&mUnidentified05C, 1);
    for (int i = 0; i < mUnidentified05C; ++i)
        mDetermData[i].Serialize(serializer);
    if (serializer->mDirection == 0)
    {
        for (s8 i = 0; i < 4; ++i)
        {
            if (mUnidentified008 & (0x10 << i))
                serializer->Transfer(&mUnidentified018[i], 1);
        }
    }
    else
    {
        for (s8 i = 0; i < 4; ++i)
        {
            if (mUnidentified008 & (0x10 << i))
                serializer->Transfer(&mUnidentified018[i], 1);
        }
    }
    for (s8 i = 0; i < 4; ++i)
    {
        if (mUnidentified018[i] & 2)
            serializer->Transfer(mUnidentified01C[i].mData + 0, 2);
        if (mUnidentified018[i] & 4)
        {
            serializer->Transfer(mUnidentified01C[i].mData + 12, 1);
            serializer->Transfer(mUnidentified01C[i].mData + 13, 1);
        }
        if (mUnidentified018[i] & 8)
        {
            serializer->Transfer(mUnidentified01C[i].mData + 14, 1);
            serializer->Transfer(mUnidentified01C[i].mData + 15, 1);
        }
        if (mUnidentified018[i] & 0x10)
            serializer->Transfer(mUnidentified01C[i].mData + 3, 3);
        if (mUnidentified018[i] & 0x20)
            serializer->Transfer(mUnidentified01C[i].mData + 6, 3);
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
