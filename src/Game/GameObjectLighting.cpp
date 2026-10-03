#include <revolution/gx/GXLight.h>
#include <revolution/gx/GXTev.h>
#include <revolution/gx/GXTransform.h>
#include <revolution/mtx/mtx.h>

#include "Game/GameObjectLighting.h"
#include "Game/Render/StadiumWorldObjects.h"
#include "Game/Render/ImpostorModel.h"

#include "Game/BasicStadium.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/MathHelpers.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glMaterialParameters.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glState.h"
#include "NL/gl/glTexture.h"
#include "NL/gl/glView.h"
#include "Game/TweakValue.h"
#include "NL/glx/GXMaterialProgram.h"
#include "NL/glx/glxGX.h"
#include "NL/glx/glxMatrix.h"
#include "NL/glx/glxTexture.h"
#include "NL/nlMath.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "NL/platvmath.h"
#include "Game/Render/LightingLookup.h"
#include "Game/TweakValueFloat.h"
#include "Game/TweakValueInt.h"
#include "Game/UnidentifiedStaticStorage.h"
#include "Game/UnidentifiedTweakAction.h"

// GameRenderTask defines this flag as u8; this unit only matches closer when
// it reads the byte as a bool.
extern bool g_bRenderWorldEffects;

struct StadiumLightingParams
{
    /* 0x00 */ u32 lightRamp;
    /* 0x04 */ s32 rampStartR;
    /* 0x08 */ s32 rampStartG;
    /* 0x0C */ s32 rampStartB;
    /* 0x10 */ s32 rampEndR;
    /* 0x14 */ s32 rampEndG;
    /* 0x18 */ s32 rampEndB;
    /* 0x1C */ f32 keyLightIntensity;
    /* 0x20 */ f32 fillLightIntensity;
    /* 0x24 */ f32 inGameKeyIntensity;
    /* 0x28 */ f32 inGameFillIntensity;
    /* 0x2C */ f32 inGameKeyRotYDeg;
    /* 0x30 */ f32 inGameKeyRotZDeg;
    /* 0x34 */ f32 inGameFillRotYDeg;
    /* 0x38 */ f32 inGameFillRotZDeg;
    /* 0x3C */ u32 unknown3C;
}; // total size: 0x40

struct GameObjectLight
{
    GameObjectLight();

    /* 0x00 */ bool enabled;
    /* 0x01 */ u8 unknown01;
    /* 0x02 */ u8 unknown02;
    /* 0x03 */ u8 _pad03;
    /* 0x04 */ f32 intensity;
    /* 0x08 */ f32 unknown08;
    /* 0x0C */ f32 unknown0C;
    /* 0x10 */ nlVector3 worldPosition;
    /* 0x1C */ nlColour colour;
    /* 0x20 */ f32 unknown20;
}; // total size: 0x24

struct GameObjectLightArray
{
    GameObjectLight lights[2];
}; // total size: 0x48

static inline const char* GetLastTweakCategory()
{
    return gLastTweakCategory;
}

TweakValueFloat gShadowLookupScaleX(
    "Scale X", "/Rendering/Lighting/Shadow Lookup", 0.042f, false);
TweakValueFloat gShadowLookupScaleY(
    "Scale Y", GetLastTweakCategory(), 0.073f, false);
TweakValueFloat gShadowLookupTransX(
    "Trans X", GetLastTweakCategory(), 0.0f, false);
TweakValueFloat gShadowLookupTransY(
    "Trans Y", GetLastTweakCategory(), 0.0f, false);

bool lbl_806DCC40 = true;
f32 lbl_806DCC44 = 1.0f;
bool lbl_806DCC48 = true;
u32 lbl_806DCC4C = 0x50;
u32 lbl_806DCC50 = 0x50;
u32 lbl_806DCC54 = 0x50;
bool lbl_806DCC58 = true;
bool lbl_806DCC59 = true;
bool lbl_806DCC5A = true;
s32 lbl_806DCC5C = 0x100;
u32 lbl_806DCC60 = (u32)-1;
s32 lbl_806DCC64 = 2;
u8 lbl_806DCC68 = 1;
u32 lbl_806DCC6C = (u32)-1;
u32 lbl_806DCC70 = (u32)-1;
s32 lbl_806DCC74 = -1;

bool lbl_806E1410;
bool lbl_806E1411;
bool lbl_806E1412;
bool lbl_806E1413;

u32 lbl_806E1414 = glGetTexture("global/lightramp");
u32 lbl_806E1418 = glGetTexture("global/black");
u32 lbl_806E141C = glGetTexture("global/white");

StadiumLightingParams gStadiumGameObjectLightingParams = {
    glGetTexture("ThePalaceStadiumPlayerLightRamp"),
    0x28, 0x30, 0x28,
    0xAF, 0xAF, 0xAF,
    0.85f, 0.3f, 0.9f, 0.25f,
    55.0f, 60.0f, 55.0f, -120.0f,
    0,
};

Mtx lbl_80511490 = {
    { 0.5f, 0.0f, 0.0f, 0.5f },
    { 0.0f, -0.5f, 0.0f, 0.5f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
};

LightingLookup* gpShadowLightingLookup;
PlatTexture* g_pGameObjectLightRamp;
s32 lbl_806E1428;
bool gAlwaysUseCameraRelativeCharacterLighting;

GameObjectLight lbl_805709D8[8];
GameObjectLight lbl_80570AF8[2];
GameObjectLight lbl_80570B40;
TweakValueInt lbl_80570B70(
    "numCharacterInGameLights", "/Rendering/Lighting/Character", 2);
GameObjectLight lbl_80570B80;
TweakValueInt lbl_80570BB0(
    "siCharacterLightRed", "/Rendering/Lighting/Character", 0);
TweakValueInt lbl_80570BD0("siCharacterLightGreen", gLastTweakCategory, 0);
TweakValueInt lbl_80570BF0("siCharacterLightBlue", gLastTweakCategory, 0);

void UpdateCharacterLightColour();

UnidentifiedTweakAction lbl_806E1430(
    "Character Light Red", gLastTweakCategory, Function0<void>(UpdateCharacterLightColour));
UnidentifiedTweakAction lbl_806E1434(
    "Character Light Green", gLastTweakCategory, Function0<void>(UpdateCharacterLightColour));
UnidentifiedTweakAction lbl_806E1438(
    "Character Light Blue", gLastTweakCategory, Function0<void>(UpdateCharacterLightColour));

static const nlVector3 sInitialDirection = { 1.0f, 0.0f, 0.0f };

void FillInGameObjectLightRamp();
void SetCameraRelativeLightData(void* pLightData);
void LoadGameObjectLight(int lightId, const GameObjectLight* pLight, const nlMatrix4& mview);

int IsGameObjectLightingEnabled()
{
    return lbl_806DCC40;
}

int ShouldUseGameObjectLightTexture(int character)
{
    return character ? lbl_806E1411 : lbl_806E1410;
}

int ShouldDoubleGameObjectLighting()
{
    return lbl_806E1412;
}

bool AlwaysUseCameraRelativeCharacterLighting()
{
    return gAlwaysUseCameraRelativeCharacterLighting;
}

void UpdateCharacterLightColour()
{
    lbl_80570B80.colour.c[0] = lbl_80570BB0.value;
    lbl_80570B80.colour.c[1] = lbl_80570BD0.value;
    lbl_80570B80.colour.c[2] = lbl_80570BF0.value;
}

void fn_80182164()
{
}

void PrepareStadiumLight(StadiumLight* pLight)
{
    // This retained path prepares a light locally but does not publish it.
    GameObjectLight light;
    light.unknown01 = true;
    light.enabled = true;
    const nlMatrix4& matrix = *pLight->GetWorldMatrix();
    light.colour.c[0] = nlFloatColourToByte(pLight->m_colour.c[0]);
    light.colour.c[1] = nlFloatColourToByte(pLight->m_colour.c[1]);
    light.colour.c[2] = nlFloatColourToByte(pLight->m_colour.c[2]);
    light.colour.c[3] = nlFloatColourToByte(pLight->m_colour.c[3]);
    light.worldPosition = matrix.GetTranslation();
    light.intensity = pLight->GetIntensity();
}

int GetGameObjectLightCount(bool arg0, bool arg1)
{
    bool var0 = arg1 && lbl_806DCC48;
    int var1 = var0 ? GetEmissionManager()->GetNumLights() : 0;

    switch (lbl_806E1428)
    {
    case 0:
        if (arg0)
            return var1 + lbl_80570B70.value;
        return lbl_806DCC64 + var1;
    case 1:
        return lbl_806DCC64 + var1;
    case 2:
        return 1;
    default:
        lbl_806E1428 = 0;
        return 2;
    }
}

GameObjectLight* GetGameObjectLight(int arg0, bool arg1)
{
    s32 var0 = arg1 ? lbl_80570B70.value : lbl_806DCC64;
    if (!lbl_806DCC68 && lbl_806E1428 == 1)
    {
        lbl_806E1428 = 0;
    }

    switch (lbl_806E1428)
    {
    case 0:
        if (arg0 >= var0)
        {
            EffectsLight* pLight = GetEmissionManager()->GetLight(arg0 - lbl_806DCC64);
            GameObjectLight* var1 = &lbl_805709D8[arg0];
            var1->enabled = true;
            var1->unknown01 = true;
            var1->unknown02 = true;
            var1->intensity = 1.0f;
            var1->worldPosition = pLight->m_v3Position;
            var1->colour.c[0] = pLight->m_Colour.c[0];
            var1->colour.c[1] = pLight->m_Colour.c[1];
            var1->colour.c[2] = pLight->m_Colour.c[2];
            var1->colour.c[3] = pLight->m_Colour.c[3];
            var1->unknown20 = pLight->m_fRadius;
            return var1;
        }
        if (arg1)
        {
            if (arg0 == 0)
                return &lbl_80570B80;
            s32 characterLightCount = lbl_80570B70.value;
            s32 lightIndex = arg0;
            if (lightIndex < characterLightCount)
                return &lbl_805709D8[lightIndex];
            lightIndex -= characterLightCount;
            return &lbl_805709D8[lightIndex + lbl_806DCC64];
        }
        return &lbl_805709D8[arg0];

    case 1:
        if (arg0 >= var0)
        {
            EffectsLight* pLight = GetEmissionManager()->GetLight(arg0 - lbl_806DCC64);
            GameObjectLight* var1 = &lbl_805709D8[arg0];
            var1->enabled = true;
            var1->unknown01 = true;
            var1->unknown02 = true;
            var1->intensity = 1.0f;
            var1->worldPosition = pLight->m_v3Position;
            var1->colour.c[0] = pLight->m_Colour.c[0];
            var1->colour.c[1] = pLight->m_Colour.c[1];
            var1->colour.c[2] = pLight->m_Colour.c[2];
            var1->colour.c[3] = pLight->m_Colour.c[3];
            var1->unknown20 = pLight->m_fRadius;
            return var1;
        }
        return &lbl_80570AF8[arg0];

    case 2:
        return &lbl_80570B40;

    default:
        return lbl_805709D8;
    }
}

void InitializeGameObjectLighting()
{
    StadiumLightingParams* pParams = &gStadiumGameObjectLightingParams;

    lbl_806DCC64 = 2;
    lbl_805709D8[0].intensity = pParams->inGameKeyIntensity;
    lbl_805709D8[0].unknown08 = pParams->inGameKeyRotYDeg;
    lbl_805709D8[0].unknown0C = pParams->inGameKeyRotZDeg;
    lbl_805709D8[1].intensity = pParams->inGameFillIntensity;
    lbl_805709D8[1].unknown08 = pParams->inGameFillRotYDeg;
    lbl_805709D8[1].unknown0C = pParams->inGameFillRotZDeg;

    lbl_80570B80.unknown01 = true;
    lbl_80570B80.unknown02 = false;
    lbl_80570B80.enabled = true;
    lbl_80570B80.intensity = 1.0f;
    nlColourSet(lbl_80570B80.colour, 0, 0, 0, 255);
    if (BasicStadium::GetCurrentStadium() != 0)
    {
        lbl_80570B80.worldPosition = BasicStadium::GetCurrentStadium()->m_shadowLightPosition;
    }
    lbl_80570B80.colour.c[0] = lbl_80570BB0.value;
    lbl_80570B80.colour.c[1] = lbl_80570BD0.value;
    lbl_80570B80.colour.c[2] = lbl_80570BF0.value;

    lbl_80570AF8[0].intensity = 1.0f;
    lbl_80570AF8[1].intensity = 1.0f;
    lbl_80570B40.intensity = 1.0f;
    lbl_80570B40.enabled = true;
    nlVec3Set(lbl_80570B40.worldPosition, 0.0f, 0.0f, -1.0f);

    GLResourcePool* pResource = glGetCurrentResourcePool();
    g_pGameObjectLightRamp = glx_CreatePlatTexture(pResource);
    PlatTexture* pRampTexture = g_pGameObjectLightRamp;
    glRegisterTexture(pParams->lightRamp, pRampTexture, pResource);
    g_pGameObjectLightRamp->Create(0x100, 4, GXTex_RGBA8, pResource, 1, true, false);
    FillInGameObjectLightRamp();
}

void UpdateGameObjectLighting()
{
    if (!lbl_806DCC68)
        return;

    if (!DrawableCharacter::sCameraRelativeLighting && !gAlwaysUseCameraRelativeCharacterLighting && lbl_806E1428 != 1)
        return;

    SetCameraRelativeLightData(&lbl_80570AF8);
}

void SetCameraRelativeLightData(void* pLightData)
{
    static nlVector3 keyLightInViewSpace;
    static nlVector3 fillLightInViewSpace;
    static bool initedLightInViewSpace;
    nlVector3 initialDirection;
    nlVector3 viewVec;
    nlVector3 transformedDir;
    nlVector3 keyDirection;
    nlVector3 fillDirection;
    nlMatrix4 matY;
    nlMatrix4 matZ;
    nlMatrix4 viewRotMat;

    if (!initedLightInViewSpace)
    {
        initialDirection = sInitialDirection;
        nlMakeRotationMatrixY(matY, 0.7853982f);
        nlMakeRotationMatrixZ(matZ, -0.69813174f);
        nlMultDirVectorMatrix(keyLightInViewSpace, initialDirection, matY);
        nlMultDirVectorMatrix(keyDirection, keyLightInViewSpace, matZ);
        keyLightInViewSpace = keyDirection;

        nlMakeRotationMatrixY(matY, 0.5235988f);
        nlMakeRotationMatrixZ(matZ, 0.34906587f);
        nlMultDirVectorMatrix(fillLightInViewSpace, initialDirection, matY);
        nlMultDirVectorMatrix(fillDirection, fillLightInViewSpace, matZ);
        fillLightInViewSpace = fillDirection;

        initedLightInViewSpace = true;
    }

    cCameraManager::GetViewVector(viewVec);

    u16 u16Angle = nlVector3ToAngle(viewVec);
    f32 radAngle = (f32)u16Angle * 0.0000958738f;

    nlMakeRotationMatrixZ(viewRotMat, radAngle);

    StadiumLightingParams* params = &gStadiumGameObjectLightingParams;
    GameObjectLightArray* pLights = (GameObjectLightArray*)pLightData;

    nlMultDirVectorMatrix(transformedDir, keyLightInViewSpace, viewRotMat);

    pLights->lights[0].enabled = true;
    nlVec3Set(pLights->lights[0].worldPosition, -transformedDir.x, -transformedDir.y, -transformedDir.z);
    pLights->lights[0].intensity = params->keyLightIntensity;

    nlMultDirVectorMatrix(transformedDir, fillLightInViewSpace, viewRotMat);

    pLights->lights[1].enabled = true;
    nlVec3Set(pLights->lights[1].worldPosition, -transformedDir.x, -transformedDir.y, -transformedDir.z);
    pLights->lights[1].intensity = params->fillLightIntensity;
}

void FillInGameObjectLightRamp()
{
    StadiumLightingParams* pRampParams = &gStadiumGameObjectLightingParams;
    u8* pTextureData;
    s32 i;
    f32 deltaR;
    f32 deltaG;
    f32 deltaB;

    deltaR = (f32)(pRampParams->rampEndR - pRampParams->rampStartR);
    deltaG = (f32)(pRampParams->rampEndG - pRampParams->rampStartG);
    deltaB = (f32)(pRampParams->rampEndB - pRampParams->rampStartB);

    pTextureData = (u8*)g_pGameObjectLightRamp->m_LinearData;
    for (i = 0; i < 0x100; i += 8)
    {
        {
            f32 t = (f32)i / 256.0f;
            pTextureData[0] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[1] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[2] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[3] = 0xFF;
        }
        {
            f32 t = (f32)(i + 1) / 256.0f;
            pTextureData[4] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[5] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[6] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[7] = 0xFF;
        }
        {
            f32 t = (f32)(i + 2) / 256.0f;
            pTextureData[8] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[9] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[10] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[11] = 0xFF;
        }
        {
            f32 t = (f32)(i + 3) / 256.0f;
            pTextureData[12] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[13] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[14] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[15] = 0xFF;
        }
        {
            f32 t = (f32)(i + 4) / 256.0f;
            pTextureData[16] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[17] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[18] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[19] = 0xFF;
        }
        {
            f32 t = (f32)(i + 5) / 256.0f;
            pTextureData[20] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[21] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[22] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[23] = 0xFF;
        }
        {
            f32 t = (f32)(i + 6) / 256.0f;
            pTextureData[24] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[25] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[26] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[27] = 0xFF;
        }
        {
            f32 t = (f32)(i + 7) / 256.0f;
            pTextureData[28] = (u8)(t * deltaR + (f32)pRampParams->rampStartR);
            pTextureData[29] = (u8)(t * deltaG + (f32)pRampParams->rampStartG);
            pTextureData[30] = (u8)(t * deltaB + (f32)pRampParams->rampStartB);
            pTextureData[31] = 0xFF;
        }
        pTextureData += 0x20;
    }

    for (i = 1; i < 4; i++)
    {
        memcpy((u8*)g_pGameObjectLightRamp->m_LinearData + (i * 0x400), g_pGameObjectLightRamp->m_LinearData, 0x400);
    }

    g_pGameObjectLightRamp->Swizzle(false);
    g_pGameObjectLightRamp->Prepare();
}

u32 GetGameObjectLightRamp()
{
    return gStadiumGameObjectLightingParams.lightRamp;
}

static const GXLightID sGameObjectLightIDs[6] = {
    GX_LIGHT0, GX_LIGHT1, GX_LIGHT2, GX_LIGHT3, GX_LIGHT4, GX_LIGHT5,
};

static const u32 sGameObjectLightMasks[6] = {
    GX_LIGHT0,
    GX_LIGHT0 | GX_LIGHT1,
    GX_LIGHT0 | GX_LIGHT1 | GX_LIGHT2,
    GX_LIGHT0 | GX_LIGHT1 | GX_LIGHT2 | GX_LIGHT3,
    GX_LIGHT0 | GX_LIGHT1 | GX_LIGHT2 | GX_LIGHT3 | GX_LIGHT4,
    GX_LIGHT0 | GX_LIGHT1 | GX_LIGHT2 | GX_LIGHT3 | GX_LIGHT4 | GX_LIGHT5,
};

static const GXLightID sSpecularLightIDs[2] = { GX_LIGHT6, GX_LIGHT7 };

nlMatrix4 lbl_80570C18;

unsigned long GetGameObjectLightTexture()
{
    return lbl_806DCC60;
}

void SetGameObjectLightTexture(unsigned long texture)
{
    lbl_806DCC60 = texture;
}

void SetGameObjectLightingMode(int mode)
{
    lbl_806E1428 = mode;
}

void LoadGameObjectLights(int arg0, GLView* arg1, bool arg2)
{
    static GLView* lbl_806E143C;
    static bool lbl_806E1440;
    s32 var0;
    nlMatrix4 var1;

    if (lbl_806E143C == arg1 && arg2 == lbl_806E1440)
        return;

    GLViewInterface* var2 = arg1->m_Interface;
    lbl_806E143C = arg1;
    var2->GetViewMatrix(var1);
    lbl_806E1440 = arg2;

    for (var0 = 0; var0 < arg0; var0++)
    {
        GameObjectLight* var3 = GetGameObjectLight(var0, arg2);
        LoadGameObjectLight(var0, var3, var1);
    }
}

void LoadGameObjectLight(int lightId, const GameObjectLight* pLight, const nlMatrix4& mview)
{
    GXLightObj light;
    nlVector3 var1;
    nlVector3 var0;
    nlVector3 initialDirection;
    nlVector3 viewPos;
    nlVector3 viewDir;
    nlVector3 worldDir;

    if (pLight->unknown01)
    {
        f32 brightness = 255.0f * pLight->intensity;
        s32 var3 = (s32)(brightness * lbl_806DCC44);
        if (var3 > 255)
            var3 = 255;

        GXColor colour = {
            (u8)((var3 * pLight->colour.c[0]) >> 8),
            (u8)((var3 * pLight->colour.c[1]) >> 8),
            (u8)((var3 * pLight->colour.c[2]) >> 8),
            0xFF,
        };
        GXInitLightColor(&light, colour);
    }
    else
    {
        f32 brightness = 255.0f * pLight->intensity;
        s32 var3 = (s32)(brightness * lbl_806DCC44);
        if (var3 > 255)
            var3 = 255;

        GXColor colour = { (u8)var3, (u8)var3, (u8)var3, 0xFF };
        GXInitLightColor(&light, colour);
    }

    if (pLight->enabled)
    {
        var0 = pLight->worldPosition;
    }
    else
    {
        float angleY = pLight->unknown08;
        angleY = (3.1415927f * angleY) / 180.0f;
        static const nlVector3 sDirection = { 1.0f, 0.0f, 0.0f };
        initialDirection = sDirection;
        nlMatrix4 matY;
        nlMatrix4 matZ;

        nlMakeRotationMatrixY(matY, angleY);
        float angleZ = pLight->unknown0C;
        nlMakeRotationMatrixZ(
            matZ, (3.1415927f * angleZ) / 180.0f);
        nlMultDirVectorMatrix(var1, initialDirection, matY);
        nlMultDirVectorMatrix(var1, matZ);

        nlVec3Set(var0, -var1.x, -var1.y, -var1.z);
    }

    if (pLight->unknown02)
    {
        nlMultPosVectorMatrix(viewPos, var0, mview);
        GXInitLightPos(&light, viewPos.x, viewPos.y, viewPos.z);
        GXInitLightAttnA(&light, 1.0f, 0.0f, 0.0f);
        GXInitLightDistAttn(
            &light, 0.666f * pLight->unknown20, 0.5f, GX_DA_STEEP);
    }
    else
    {
        nlVector3 origin = {
            0.0f,
            0.0f,
            0.0f,
        };

        float worldY = var0.y - origin.y;
        float worldX = var0.x - origin.x;
        float worldZ = var0.z - origin.z;

        worldDir.x = worldX;
        worldDir.y = worldY;
        worldDir.z = worldZ;

        if (worldDir.GetLengthSq3D() <= 0.0f)
            worldDir.x = 1.0f;

        {
            float lengthSq = worldDir.GetLengthSq3D();
            float recipLength = nlRecipSqrt(lengthSq, true);

            nlVec3Scale(worldDir, recipLength);
        }

        nlMultDirVectorMatrix(viewDir, worldDir, mview);
        nlVec3Scale(viewDir, 1048576.0f);

        GXInitLightPos(&light, viewDir.x, viewDir.y, viewDir.z);
        GXInitLightAttnA(&light, 1.0f, 0.0f, 0.0f);
        GXInitLightDistAttn(&light, 1048576.0f, 1.0f, GX_DA_OFF);
    }

    GXLoadLightObjImm(&light, sGameObjectLightIDs[lightId]);
}

void SetGameObjectLightingEnabled(bool arg0, int arg1, bool arg2)
{
    if (arg0)
    {
        if (!arg2)
        {
            nlColour colour = {
                0xFF,
                0xFF,
                0xFF,
                0xFF,
            };
            gxSetChanMatColour(0, colour);
        }

        s32 var0 = arg1 - 1;
        GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, (GXColorSrc)(arg2 != 0),
            (GXLightID)sGameObjectLightMasks[var0], GX_DF_CLAMP, GX_AF_SPOT);
    }
    else
    {
        s32 var0 = arg1 - 1;
        GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX,
            (GXLightID)sGameObjectLightMasks[var0], GX_DF_NONE, GX_AF_NONE);
    }
}

void LoadGameObjectSpecularLight(int index, GameObjectLight* lightData, float exponent, const nlMatrix4& viewMatrix)
{
    if (index < 0 || index >= 2)
        return;

    GXLightObj light;
    nlVector3 var1;
    nlVector3 var0;
    nlVector3 initialDirection;
    nlVector3 viewDir;
    nlVector3 worldDir;

    s32 var3 = (s32)(255.0f * lightData->intensity);
    if (var3 > 255)
        var3 = 255;

    GXColor colour = { (u8)var3, (u8)var3, (u8)var3, 0xFF };
    GXInitLightColor(&light, colour);

    if (lightData->enabled)
    {
        var0 = lightData->worldPosition;
    }
    else
    {
        float angleY = lightData->unknown08;
        angleY = (3.1415927f * angleY) / 180.0f;
        static const nlVector3 sDirection = { 1.0f, 0.0f, 0.0f };
        initialDirection = sDirection;
        nlMatrix4 matY;
        nlMatrix4 matZ;

        nlMakeRotationMatrixY(matY, angleY);
        float angleZ = lightData->unknown0C;
        nlMakeRotationMatrixZ(
            matZ, (3.1415927f * angleZ) / 180.0f);
        nlMultDirVectorMatrix(var1, initialDirection, matY);
        nlMultDirVectorMatrix(var1, matZ);

        nlVec3Set(var0, -var1.x, -var1.y, -var1.z);
    }

    nlVector3 origin = {
        0.0f,
        0.0f,
        0.0f,
    };

    float worldY = var0.y - origin.y;
    float worldX = var0.x - origin.x;
    float worldZ = var0.z - origin.z;

    worldDir.x = worldX;
    worldDir.y = worldY;
    worldDir.z = worldZ;

    {
        float lengthSq = worldDir.GetLengthSq3D();
        float recipLength = nlRecipSqrt(lengthSq, true);

        nlVec3Scale(worldDir, recipLength);
    }

    nlMultDirVectorMatrix(viewDir, worldDir, viewMatrix);
    nlVec3Set(viewDir, -viewDir.x, -viewDir.y, -viewDir.z);
    GXInitSpecularDir(&light, viewDir.x, viewDir.y, viewDir.z);

    GXInitLightAttn(&light, 0.0f, 0.0f, 1.0f,
        exponent / 2.0f, 0.0f,
        1.0f - exponent / 2.0f);

    GXLoadLightObjImm(&light, sSpecularLightIDs[index]);
}

void SetGameObjectSpecularLightingEnabled(int enabled, int)
{
    if (enabled)
    {
        nlColour colour = {
            0xFF,
            0xFF,
            0xFF,
            0xFF,
        };
        gxSetChanMatColour(1, colour);

        nlColour ambient = {
            0,
            0,
            0,
            0,
        };
        gxSetChanAmbColour(1, ambient);

        GXSetChanCtrl(GX_COLOR1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT6,
            GX_DF_NONE, GX_AF_SPEC);
    }
    else
    {
        GXSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT6,
            GX_DF_NONE, GX_AF_NONE);
    }
}

void SetGameObjectAmbientLightingEnabled(int enabled)
{
    if (enabled)
    {
        gxSetChanAmbColour(0, (nlColour){
            (u8)lbl_806DCC4C,
            (u8)lbl_806DCC50,
            (u8)lbl_806DCC54,
            0,
        });
    }
    else
    {
        gxSetChanAmbColour(0, (nlColour){ 0, 0, 0, 0 });
    }
}

void LoadShadowLightingLookup(unsigned long textureHandle)
{
    if (textureHandle != (u32)-1 && glTextureLoad(textureHandle))
    {
        lbl_806DCC6C = textureHandle;
        gpShadowLightingLookup = new (8, false) LightingLookup;
        gpShadowLightingLookup->LoadTexture(textureHandle);
    }
    else
    {
        lbl_806DCC6C = -1;
    }
}

void ApplyGameObjectShadowLighting(int arg0, unsigned long arg1)
{
    if (!IsShadowLookupActive())
        return;

    u32 numTevStages;
    u32 numTexGens;
    numTexGens = gxGetNumTexGens();
    numTevStages = gxGetNumTevStages();
    gxSetNumTexGens(numTexGens + 1);
    gxSetNumTevStages(numTevStages + 1);
    lbl_806DCC74 = numTexGens;

    if (arg1 != 0)
    {
        gxSetTevOrder(numTevStages, 0xFF, 0xFF, 0xFF);
        gxSetTevColourIn(numTevStages, 15, 0, 6, 15);
        gxSetTevAlphaIn(numTevStages, 7, 7, 7, 0);

        GXColor colour = *(GXColor*)&arg1;
        GXSetTevColor(GX_TEVREG2, colour);
    }
    else
    {
        gxSetTevOrder(numTevStages, numTexGens, numTexGens, 0xFF);
        if (arg0 && lbl_806DCC5A)
            gxSetTexCoordGen(numTexGens, 1, 0, 0, false, 0x76);
        else
            gxSetTexCoordGen(numTexGens, 1, 0, 0x36, false, 0x76);

        gxSetTevColourIn(numTevStages, 15, 0, 8, 15);
        gxSetTevAlphaIn(numTevStages, 7, 7, 7, 0);

        glTextureBinding textureState(lbl_806DCC6C, !lbl_806E1413, !lbl_806E1413);
        glx_BindTexture(numTexGens, &textureState);

        nlMatrix4 transform;
        nlMakeScaleMatrix(transform, gShadowLookupScaleX,
            gShadowLookupScaleY, 1.0f);
        transform.SetRow4_(3, gShadowLookupTransX,
            gShadowLookupTransY, 0.0f, 1.0f);

        Mtx gxTransform;
        glxCopyMatrix(gxTransform, transform);

        Mtx textureMatrix;
        PSMTXConcat(lbl_80511490, gxTransform, textureMatrix);

        if (arg0 && lbl_806DCC5A)
        {
            Mtx inverse;
            glxCopyMatrix(inverse, lbl_80570C18);
            if (lbl_806DCC59)
                PSMTXConcat(textureMatrix, inverse, textureMatrix);
            else
                PSMTXConcat(inverse, textureMatrix, textureMatrix);
        }

        GXLoadTexMtxImm(textureMatrix, 0x76, GX_MTX3x4);
    }
}

void RestoreGameObjectShadowLighting()
{
    if (IsShadowLookupActive() && lbl_806DCC74 >= 0)
    {
        gxSetNumTexGens(gxGetNumTexGens() - 1);
        gxSetNumTevStages(gxGetNumTevStages() - 1);
        gxSetTexCoordGen(lbl_806DCC74, 1, lbl_806DCC74 + 4, 0x3C);
        lbl_806DCC74 = -1;
    }
}

void SetGameObjectShadowModelMatrix(unsigned long matrix)
{
    if (!IsShadowLookupActive())
        return;

    if (matrix == (u32)-1)
    {
        lbl_806DCC70 = -1;
    }
    else if (matrix != lbl_806DCC70)
    {
        lbl_806DCC70 = matrix;

        nlMatrix4 transform;
        glGetMatrix(matrix, transform);

        Mtx textureMatrix;
        glxCopyMatrix(textureMatrix, transform);
        GXLoadTexMtxImm(textureMatrix, 0x36, GX_MTX3x4);
    }
}

void SetGameObjectShadowViewMatrix(const nlMatrix4* matrix)
{
    if (IsShadowLookupActive())
        nlInvertMatrix(lbl_80570C18, *matrix);
}

bool IsShadowLookupActive()
{
    if (!lbl_806DCC58)
        return false;

    if (lbl_806DCC6C == (u32)-1)
        return false;

    if (gpShadowLightingLookup == 0)
        return false;

    if (!g_bRenderWorldEffects)
        return false;

    return true;
}

nlColour SampleShadowLookup(const nlVector2* arg0, bool arg1)
{
    if (!IsShadowLookupActive())
    {
        nlColour var0;
        nlColourSet(var0, 0xFF, 0xFF, 0xFF, 0xFF);
        return var0;
    }

    if (lbl_806DCC5C <= 0xFF)
    {
        nlColour var0;
        nlColourSet(var0, 0xFF, 0xFF, 0xFF, 0xFF);
        return var0;
    }

    f32 var0 = arg0->x;
    var0 *= gShadowLookupScaleX.value;
    var0 = var0 + gShadowLookupTransX.value;
    f32 var1 = arg0->y;
    var1 *= gShadowLookupScaleY.value;
    var1 = var1 + gShadowLookupTransY.value;
    var0 = 0.5f * var0 + 0.5f;
    var1 = -0.5f * var1 + 0.5f;
    var0 *= (f32)gpShadowLightingLookup->mWidth;
    var1 *= (f32)gpShadowLightingLookup->mHeight;
    return gpShadowLightingLookup->SampleFilteredColour(var0, var1, arg1);
}

int GetShadowLookupLevel(const nlVector3* arg0)
{
    nlColour var0 = SampleShadowLookup((const nlVector2*)arg0, true);
    return (var0.c[0] * 140 + var0.c[1] * 88 + var0.c[2] * 29) >> 8;
}

void ReleaseShadowLightingLookup()
{
    if (gpShadowLightingLookup != 0)
    {
        delete gpShadowLightingLookup;
        gpShadowLightingLookup = 0;
    }
    lbl_806DCC6C = -1;
}

void SetImpostorShadowLevel(ImpostorModel* arg0, glModel* arg1)
{
    int var0 = GetShadowLookupLevel(&arg0->mWorldMatrix.GetTranslation());
    nlColour var1;
    nlColourSet(var1, var0, var0, var0, 1);
    unsigned long var2 = *(unsigned long*)&var1;
    static unsigned long var3 = nlStringLowerHash("shadowLevel");
    for (glModelPacket* var4 = arg1->packets; var4 < arg1->packets + arg1->numPackets; ++var4)
    {
        glSetMaterialUnsignedParameter(var4, var3, var2);
    }
}

void SetImpostorPacketShadowLevels(ImpostorModel*, glModel* arg1)
{
    static unsigned long var0 = nlStringLowerHash("shadowLevel");
    for (glModelPacket* var1 = arg1->packets; var1 < arg1->packets + arg1->numPackets; ++var1)
    {
        nlMatrix4 var2;
        glGetMatrix(var1->matrix, var2);
        nlVector3 var3 = var2.GetTranslation();
        nlColour var4 = SampleShadowLookup((const nlVector2*)&var3, false);
        unsigned long var5 = *(unsigned long*)&var4;
        if (glHasMaterialParameter(var1, var0))
        {
            glSetMaterialUnsignedParameter(var1, var0, var5);
        }
    }
}

GameObjectLight::GameObjectLight()
{
    enabled = false;
    unknown01 = false;
    unknown02 = false;
}
