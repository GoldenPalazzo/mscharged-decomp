#ifndef _FONTMANAGER_H_
#define _FONTMANAGER_H_

#include "NL/nlDLListContainer.h"
#include "NL/nlSingleton.h"

class nlFont;
class GLResourcePool;

class FontManager : public nlSingleton<FontManager>
{
public:
    FontManager();
    virtual ~FontManager();

    nlFont* GetFontByHashID(unsigned long hashID);
    bool IsLoadingComplete() const;
    bool LoadFont(const char* bundlePath, const char* fontName, const char* fontFileName);
    void SetResourcePool(GLResourcePool* resourcePool);

    static void BundleOpenComplete(void*, unsigned long, unsigned long uParam);
    static void FontDescriptionLoadComplete(void* buffer, unsigned long, unsigned long uParam);
    static void TextureLoadComplete(void* buffer, unsigned long uReadSize, unsigned long uParam);

    /* 0x04 */ nlDLListSlotPool<nlFont*> m_fonts;
    /* 0x20 */ GLResourcePool* field_0x20;
};

#endif // _FONTMANAGER_H_
