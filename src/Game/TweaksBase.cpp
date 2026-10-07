#include "Game/TweaksBase.h"

#include "NL/nlString.h"
#include "NL/nlstring_tmpl.h"

TweaksBase::TweaksBase(const char* fileName)
{
    mszFileName[0] = 0;
    nlStrNCpy<char>(mszFileName, fileName, 0x3F);
}

TweaksBase::~TweaksBase()
{
}
