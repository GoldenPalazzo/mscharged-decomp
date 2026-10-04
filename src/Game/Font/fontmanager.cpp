#include "Game/Font/fontmanager.h"
#include "NL/nlBundleFile.h"
#include "NL/nlFont.h"
#include "NL/nlMemory.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/gl/gl.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glTexture.h"

template <>
FontManager* nlSingleton<FontManager>::s_pInstance = 0;

struct Unidentified_80307564
{
    Unidentified_80307564()
    {
        field_0x0 = true;
        field_0x4 = 0;
        field_0x388 = 0;
        field_0x38C = 0;
        field_0x8[0] = field_0x107[0] = field_0x206[0] = '\0';
        for (int i = 0; i < 16; i++)
        {
            field_0x308[i].field_0x0 = 0;
            field_0x308[i].field_0x4 = -1;
        }
    }

    bool field_0x0;
    BundleFile* field_0x4;
    char field_0x8[255];
    char field_0x107[255];
    char field_0x206[255];
    struct Entry
    {
        void* field_0x0;
        unsigned long field_0x4;
    } field_0x308[16];
    unsigned long field_0x388;
    unsigned long field_0x38C;
};

Unidentified_80307564 lbl_80580748[16];

static inline void LoadFontTexture(Unidentified_80307564* state, unsigned long fileHashID)
{
    BundleFileDirectoryEntry entry;
    if (state->field_0x4->GetFileInfo(fileHashID, &entry, true))
    {
        void* textureData = nlMalloc(entry.m_length, 0x20, true);
        state->field_0x4->ReadFileAsync(fileHashID, textureData, entry.m_length, FontManager::TextureLoadComplete, (unsigned long)state);
        state->field_0x308[state->field_0x388].field_0x0 = textureData;
        state->field_0x308[state->field_0x388].field_0x4 = fileHashID;
        state->field_0x388++;
    }
}

void FontManager::BundleOpenComplete(void*, unsigned long, unsigned long uParam)
{
    Unidentified_80307564* state = (Unidentified_80307564*)uParam;
    unsigned long hashID = nlStringHash(state->field_0x107);
    BundleFileDirectoryEntry entry;

    if (state->field_0x4->GetFileInfo(hashID, &entry, true))
    {
        void* fileData = nlMalloc(entry.m_length, 0x20, true);
        state->field_0x4->ReadFileAsync(hashID, fileData, entry.m_length, FontDescriptionLoadComplete, (unsigned long)state);
    }
}

void FontManager::FontDescriptionLoadComplete(void* buffer, unsigned long, unsigned long uParam)
{
    Unidentified_80307564* state = (Unidentified_80307564*)uParam;
    BundleFileDirectoryEntry entry;

    nlFont* newFont = new (8, false) nlFont();
    newFont->Load(state->field_0x107, (char*)buffer, nlStringHash(state->field_0x206));
    FontManager::Instance()->m_fonts.AddEnd(newFont);
    delete[] (char*)buffer;

    for (unsigned long i = 0; i < newFont->m_PageCount; i++)
    {
        unsigned long hashID = newFont->m_TextureHandles[i];
        state->field_0x4->GetFileInfo(hashID, &entry, true);
        LoadFontTexture(state, hashID);

        if (newFont->m_TextureType == SplitFX)
        {
            hashID = newFont->m_EffectTextureHandles[i];
            state->field_0x4->GetFileInfo(hashID, &entry, true);
            LoadFontTexture(state, hashID);
        }
    }

    state->field_0x38C = 0;
}

void FontManager::TextureLoadComplete(void* buffer, unsigned long uReadSize, unsigned long uParam)
{
    Unidentified_80307564* state = (Unidentified_80307564*)uParam;
    char* textureData = (char*)buffer;

    for (int i = 0; i < 16; i++)
    {
        if (textureData == state->field_0x308[i].field_0x0)
        {
            glBeginResource(state->field_0x308[i].field_0x4);
            glTextureAdd(state->field_0x308[i].field_0x4, textureData, uReadSize, FontManager::Instance()->field_0x20);
            glEndResource();
            state->field_0x38C++;
            break;
        }
    }

    delete[] textureData;

    if (state->field_0x38C == state->field_0x388)
    {
        state->field_0x4->Close();
        delete state->field_0x4;
        state->field_0x0 = true;
    }
}

bool FontManager::IsLoadingComplete() const
{
    for (int i = 0; i < 16; i++)
    {
        if (!lbl_80580748[i].field_0x0)
        {
            return false;
        }
    }
    return true;
}

FontManager::FontManager()
    : m_fonts(8)
{
    field_0x20 = glGetCurrentResourcePool();
}

FontManager::~FontManager()
{
    nlDLListIterator<nlFont*> it = m_fonts.Begin();
    DLListEntry<nlFont*>* head = it.m_Head;
    DLListEntry<nlFont*>* current = it.m_Curr;

    while (current != 0)
    {
        delete current->entry;

        if (nlDLRingIsEnd(head, current) || current == 0)
        {
            current = 0;
        }
        else
        {
            current = current->m_next;
        }
    }

    m_fonts.Clear();
}

nlFont* FontManager::GetFontByHashID(unsigned long hashID)
{
    nlDLListIterator<nlFont*> it = m_fonts.Begin();
    DLListEntry<nlFont*>* head = it.m_Head;
    DLListEntry<nlFont*>* entry = it.m_Curr;

    while (entry != 0)
    {
        nlFont* font = entry->entry;
        if (hashID == font->m_Metrics.FontName)
        {
            return font;
        }

        if (nlDLRingIsEnd(head, entry) || entry == 0)
        {
            entry = 0;
        }
        else
        {
            entry = entry->m_next;
        }
    }

    nlPrintf("FontManager: Warning, failed to find font 0x%08x\n", hashID);

    nlDLListIterator<nlFont*> start = m_fonts.Begin();
    if (start.hasNext())
    {
        return *start;
    }
    return 0;
}

bool FontManager::LoadFont(const char* bundlePath, const char* fontName, const char* fontFileName)
{
    Unidentified_80307564* state = 0;
    for (int i = 0; i < 16; i++)
    {
        if (lbl_80580748[i].field_0x0)
        {
            state = &lbl_80580748[i];
            break;
        }
    }

    state->field_0x0 = false;
    state->field_0x4 = new (0x20, true) BundleFile();
    nlStrNCpy(state->field_0x8, bundlePath, 0xFF);
    nlStrNCpy(state->field_0x107, fontName, 0xFF);
    nlStrNCpy(state->field_0x206, fontFileName, 0xFF);
    nlToLower(state->field_0x206);
    state->field_0x388 = 0;
    state->field_0x38C = 0;
    state->field_0x4->OpenAsync(bundlePath, BundleOpenComplete, (unsigned long)state, false);
    return true;
}

void FontManager::SetResourcePool(GLResourcePool* resourcePool)
{
    field_0x20 = resourcePool;
}
