#include "Game/Render/NisPlayerOverlay.h"

#include "Game/NisPlayer.h"
#include "Game/Camera/CameraMan.h"
#include "Game/PoseAccumulator.h"
#include "Game/Render/ImpostorModel.h"
#include "Game/Render/RLView.h"
#include "NL/gl/glDraw2.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"
#include "NL/nlColour.h"
#include "NL/nlString.h"
#include "NL/platvmath.h"

#include "Game/UnidentifiedStaticStorage.h"

u32 lbl_806E1A20 = glGetTexture("global/checkers");
u8 lbl_806E1A24;
u8 lbl_806E1A25;
u32 lbl_806E1A28 = nlStringLowerHash("Holotron_Bone01");

static bool lbl_806DEF60 = true;
static bool lbl_806DEF61 = true;
static float lbl_806DEF64 = 22.0f;
static float lbl_806DEF68 = 17.0f;
static float lbl_806DEF6C = 7.0f;
static int lbl_806DEF70 = 0x80;
static int lbl_806DEF74 = 0xFF;
static int lbl_806DEF78 = 0xFF;

static inline void CopyCameraRotation(nlMatrix4& rotation, const nlMatrix4& view)
{
    rotation.m11 = view.m11;
    rotation.m12 = view.m12;
    rotation.m13 = view.m13;
    rotation.m14 = 0.0f;
    rotation.m21 = view.m21;
    rotation.m22 = view.m22;
    rotation.m23 = view.m23;
    rotation.m24 = 0.0f;
    rotation.m31 = view.m31;
    rotation.m32 = view.m32;
    rotation.m33 = view.m33;
    rotation.m34 = 0.0f;
    rotation.m41 = 0.0f;
    rotation.m42 = 0.0f;
    rotation.m43 = 0.0f;
    rotation.m44 = 1.0f;
}

/**
 * Offset/Address/Size: 0x270 | 0x8028400C | size: 0x14
 */
NisPlayerOverlay_80523878::NisPlayerOverlay_80523878(NisPlayer* player)
{
    mPlayer = player;
}

/**
 * Offset/Address/Size: 0x284 | 0x80284020 | size: 0x4
 */
void NisPlayerOverlay_80523878::Render()
{
}

/**
 * Offset/Address/Size: 0x288 | 0x80284024 | size: 0x14
 */
NisPlayerOverlay_8052385C::NisPlayerOverlay_8052385C(NisPlayer* player)
{
    mPlayer = player;
}

/**
 * Offset/Address/Size: 0x29C | 0x80284038 | size: 0xB8
 */
void NisPlayerOverlay_8052385C::Render()
{
    glSetDefaultState(false);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTexture(glGetTexture("target/pip"), GLTT_Diffuse);
    glSetRasterState(GLS_DepthTest, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTextureState(glHandleizeTextureState());

    glPoly2 poly;
    poly.SetupRectangle(360.0f, 270.0f, 240.0f, 150.0f, 0.0f);

    nlColour colour;
    nlColourSet(colour, 0xFF, 0xFF, 0xFF, 0xFF);
    poly.SetColour(colour);
    poly.Attach(GetLayerView(eCLV_FrontEnd), 0, 0);
}

/**
 * Offset/Address/Size: 0x354 | 0x802840F0 | size: 0x14
 */
NisPlayerOverlay_80523840::NisPlayerOverlay_80523840(NisPlayer* player)
{
    mPlayer = player;
}

/**
 * Offset/Address/Size: 0xF8 | 0x80284104 | size: 0x588
 */
void NisPlayerOverlay_80523840::Render()
{
    glQuad3 quad;
    nlMatrix4 transform;

    glSetDefaultState(true);
    glSetRasterState(GLS_DepthTest, 1);
    glSetRasterState(GLS_DepthWrite, 0);
    if (!lbl_806E1A24)
    {
        glSetRasterState(GLS_AlphaBlend, 2);
    }
    glSetRasterState(GLS_Culling, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(glGetTexture("target/pip"), GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    nlMakeRotationMatrixX(transform, 1.5707964f);
    if (lbl_806DEF60)
    {
        nlTransposeMatrix(transform, cCameraManager::PeekCamera()->GetViewMatrix());
        if (lbl_806DEF61)
        {
            mPlayer->mUseViewMatrixOverride = true;
            if (!mPlayer->mUnidentified3439C)
            {
                nlMatrix4 initialRotation;
                CopyCameraRotation(initialRotation, mPlayer->mCamera[0].GetViewMatrix());
                nlInvertMatrix(mPlayer->mInitialCameraRotationInverse, initialRotation);
                mPlayer->mUnidentified3439C = true;
            }

            nlVector3 headPosition;
            mPlayer->mPlaying[1]->GetMainCharacterHeadPosition(headPosition);
            cAnimCamera* secondaryCamera = mPlayer->GetSecondaryCamera();
            nlMatrix4 cameraInverse;
            nlMatrix4 cameraAdjustment;
            nlMatrix4 translation;
            nlMatrix4 rotation;
            nlMatrix4 viewOverride;
            nlInvertMatrix(cameraInverse, cCameraManager::PeekCamera()->GetViewMatrix());
            translation.SetIdentity();
            nlVec3Scale(headPosition, -1.0f);
            translation.SetTranslation(headPosition);
            CopyCameraRotation(rotation, mPlayer->mCamera[0].GetViewMatrix());
            nlMultMatrices(rotation, mPlayer->mInitialCameraRotationInverse);
            nlMultMatrices(cameraAdjustment, translation, rotation);
            nlVec3Scale(headPosition, -1.0f);
            translation.SetTranslation(headPosition);
            nlMultMatrices(cameraAdjustment, translation);
            nlMultMatrices(viewOverride, cameraAdjustment, secondaryCamera->GetViewMatrix());
            mPlayer->mViewMatrixOverride = viewOverride;
        }
    }

    ImpostorModel* holotron = mPlayer->mPlaying[0]->FindImpostor("holotron");
    nlVector3 position = holotron->mPoseAccumulator->GetNodeMatrixByHashID(lbl_806E1A28).GetTranslation();
    position.z += lbl_806DEF68 + lbl_806DEF6C;
    transform.SetTranslation(position);
    quad.SetupRotatedRectangle(lbl_806DEF64, lbl_806DEF68, transform, false, true);
    if (lbl_806E1A24)
    {
        quad.SetColour(0xFF, 0xFF, 0xFF, 0xFF);
    }
    else
    {
        quad.SetColour(lbl_806DEF70, lbl_806DEF74, lbl_806DEF78, 0x80);
    }
    glAttachQuad3((eGLView)GetLayerView(eCLV_Particles), 1, &quad);
    if (lbl_806E1A25)
    {
        glAttachQuad3((eGLView)GetLayerView(eCLV_Particles), 1, &quad);
    }
    glSetDefaultState(false);
}

/**
 * Offset/Address/Size: 0x8F0 | 0x8028468C | size: 0x10
 */
void fn_8028468C(float x, float y, float z)
{
    lbl_806DEF6C = x;
    lbl_806DEF64 = y;
    lbl_806DEF68 = z;
}

/**
 * Offset/Address/Size: 0x900 | 0x8028469C | size: 0x8
 */
void fn_8028469C(bool value)
{
    lbl_806DEF61 = value;
}

/**
 * Offset/Address/Size: 0x908 | 0x802846A4 | size: 0x14
 */
NisPlayerOverlay_80523824::NisPlayerOverlay_80523824(NisPlayer* player)
{
    mPlayer = player;
}

/**
 * Offset/Address/Size: 0x91C | 0x802846B8 | size: 0x4
 */
void NisPlayerOverlay_80523824::Render()
{
}

/**
 * Offset/Address/Size: 0x920 | 0x802846BC | size: 0x28
 */
int NisPlayerOverlay_80523824::Update(float dt)
{
    mPlayer->SwapCameras();
    return 1;
}

/**
 * Offset/Address/Size: 0x948 | 0x802846E4 | size: 0x4C
 */
NisPlayerOverlay_80523808::NisPlayerOverlay_80523808(NisPlayer* player, float duration)
{
    mPlayer = player;
    mDuration = duration;
    Reset();
}

/**
 * Offset/Address/Size: 0x994 | 0x80284730 | size: 0xC
 */
void NisPlayerOverlay_80523808::Reset()
{
    mTime = 0.0f;
}

/**
 * Offset/Address/Size: 0x9A0 | 0x8028473C | size: 0xF8
 */
void NisPlayerOverlay_80523808::Render()
{
    glSetDefaultState(false);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTexture(glGetTexture("target/pip"), GLTT_Diffuse);
    glSetRasterState(GLS_DepthTest, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTextureState(glHandleizeTextureState());

    float u;
    float t = mTime / mDuration;
    u = 1.0f - t;

    glPoly2 poly;
    poly.SetupRectangle(360.0f * u, 270.0f * u, 640.0f * t + 240.0f * u,
        480.0f * t + 150.0f * u, 0.0f);

    nlColour colour;
    nlColourSet(colour, 0xFF, 0xFF, 0xFF, 0xFF);
    poly.SetColour(colour);
    poly.Attach(GetLayerView(eCLV_FrontEnd), 0, 0);
}

/**
 * Offset/Address/Size: 0xA98 | 0x80284834 | size: 0x4C
 */
int NisPlayerOverlay_80523808::Update(float dt)
{
    mTime += dt;
    if (mTime <= mDuration)
    {
        return 3;
    }

    mPlayer->SwapCameras();
    return 0;
}

/**
 * Offset/Address/Size: 0xAE4 | 0x80284880 | size: 0x8
 */
int NisPlayerOverlay_80523808::GetOverlayType()
{
    return 1;
}

/**
 * Offset/Address/Size: 0xAEC | 0x80284888 | size: 0x8
 */
int NisPlayerOverlay_80523824::GetOverlayType()
{
    return 1;
}

/**
 * Offset/Address/Size: 0xAF4 | 0x80284890 | size: 0x8
 */
int NisPlayerOverlay_80523840::GetOverlayType()
{
    return 4;
}

/**
 * Offset/Address/Size: 0xAFC | 0x80284898 | size: 0x8
 */
int NisPlayerOverlay_80523840::Update(float dt)
{
    return 4;
}

/**
 * Offset/Address/Size: 0xB04 | 0x802848A0 | size: 0x8
 */
int NisPlayerOverlay_8052385C::GetOverlayType()
{
    return 1;
}

/**
 * Offset/Address/Size: 0xB0C | 0x802848A8 | size: 0x8
 */
int NisPlayerOverlay_8052385C::Update(float dt)
{
    return 1;
}

/**
 * Offset/Address/Size: 0xB14 | 0x802848B0 | size: 0x8
 */
int NisPlayerOverlay_80523878::GetOverlayType()
{
    return 0;
}

/**
 * Offset/Address/Size: 0xB1C | 0x802848B8 | size: 0x8
 */
int NisPlayerOverlay_80523878::Update(float dt)
{
    return 0;
}

NisPlayerOverlay_80523878::~NisPlayerOverlay_80523878()
{
}

NisPlayerOverlay_8052385C::~NisPlayerOverlay_8052385C()
{
}

NisPlayerOverlay_80523840::~NisPlayerOverlay_80523840()
{
}

NisPlayerOverlay_80523824::~NisPlayerOverlay_80523824()
{
}

NisPlayerOverlay_80523808::~NisPlayerOverlay_80523808()
{
}

#include "Game/Render/NisPlayerOverlay.inl"
