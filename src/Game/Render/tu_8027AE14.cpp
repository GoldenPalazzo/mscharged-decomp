#include "Game/Render/tu_8027AE14.h"
#include "Game/BasicStadium.h"
#include "Game/UnidentifiedStaticStorage.h"
#include "Game/World/WorldDrawable.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glView.h"

extern "C"
{
    float lbl_806DEE88 = 0.5f;
    float lbl_806DEE8C = 0.1f;
    float lbl_806DEE90 = 0.3f;
    float lbl_806DEE94 = 0.2f;
}

bool lbl_806E19B8;
StadiumDrawable_8027ADC0* lbl_806E19BC;


void StadiumDrawable_8027ADC0::Initialize(WorldObjectLoadContext* context)
{
    WorldDrawable::Initialize(context);
    lbl_806E19BC = this;
}

void StadiumDrawable_8027ADC0::ReleaseResources()
{
}

void StadiumDrawable_8027ADC0::Draw()
{
    if (m_Unknown70 != 0 || lbl_806E19B8)
        WorldDrawable::Draw();
}

UnidentifiedObject_8027AE14::UnidentifiedObject_8027AE14(const nlVector3& param1)
    : TimedObject(lbl_806DEE88
          + nlRandomf(-lbl_806DEE8C, lbl_806DEE8C, &nlDefaultSeed))
    , mUnidentified010(param1)
    , mUnidentified020(false)
{
    mUnidentified01C = lbl_806DEE90
        + nlRandomf(-lbl_806DEE94, lbl_806DEE94, &nlDefaultSeed);
}

UnidentifiedObject_8027AE14::~UnidentifiedObject_8027AE14()
{
}

static inline void OrientTowardPosition(
    StadiumDrawable_8027ADC0* drawable, const nlVector3& position)
{
    nlVector3 direction;
    nlVec3Sub(direction, position, drawable->mWorldMatrix.GetTranslation());
    nlVec3Normalize(direction, direction);

    nlVector3 up;
    nlVec3Set(up, 0.0f, 1.0f, 0.0f);
    nlVector3 right;
    nlVec3CrossProduct(right, direction, up);
    nlVec3CrossProduct(up, right, direction);

    drawable->mWorldMatrix.SetRow_(0, direction);
    drawable->mWorldMatrix.SetRow_(1, up);
    drawable->mWorldMatrix.SetRow_(2, right);
}

void UnidentifiedObject_8027AE14::Update(float)
{
    if (lbl_806E19BC != 0)
    {
        OrientTowardPosition(lbl_806E19BC, mUnidentified010);

        nlMatrix4 transform = *lbl_806E19BC->GetWorldMatrix();
        glModel* model = glModelDupNoStreams(
            lbl_806E19BC->GetModel(), false, glGetCurrentResourcePool());
        glModelSetMatrix(model, transform);
        BasicStadium::GetCurrentStadium()->m_pAlphaView->AttachModel(model, 0);
    }
}

StadiumDrawable_8027ADC0::~StadiumDrawable_8027ADC0()
{
}
