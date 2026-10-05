#include "Game/NetworkInputMessages.h"

#include <string.h>

NetMessageAllInputs::NetMessageAllInputs()
{
    mUnidentified008[0] = 0;
    memset(mPadding3CC, 0, sizeof(mPadding3CC));
}

void NetMessageAllInputs::Serialize(
    NetworkMessageSerializer* serializer)
{
    serializer->Transfer(&mUnidentified008[0], 1);
    for (int i = 0; i < 4; ++i)
    {
        if (mUnidentified008[0] & (1 << i))
            mMessages[i].Serialize(serializer);
    }
    if (mUnidentified008[0] & 0x80)
    {
        serializer->Transfer(mPadding3CC + 0, 4);
        serializer->Transfer(mPadding3CC + 4, 4);
        serializer->Transfer(mPadding3CC + 8, 4);
    }
}

void NetMessageAllInputsBundle::Serialize(
    NetworkMessageSerializer* serializer)
{
    mMessage0.Serialize(serializer);
    mMessage1.Serialize(serializer);
}

int NetMessageAllInputsBundle::GetType()
{
    return 9;
}

int NetMessageAllInputs::GetType()
{
    return 8;
}
