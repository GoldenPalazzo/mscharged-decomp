#ifndef GAME_GL_GL_SKIN_MESH_H
#define GAME_GL_GL_SKIN_MESH_H

#include "NL/nlMath.h"
#include "NL/gl/glModel.h"
#include "types.h"

class cPoseAccumulator;
struct glModel;

extern int gMorphOverrideID;
extern float gMorphOverrideWeight;
extern unsigned char gMorphOverrideEnabled;

struct MorphWeight
{
    /* 0x00 */ unsigned long morphID;
    /* 0x04 */ float morphWeight;
};

struct MorphDelta
{
    /* 0x00 */ nlVector3 delta;
    /* 0x0C */ int index;
};

class GLSkinMesh
{
public:
    GLSkinMesh()
        : pModel(0)
        , hierarchySignature(0)
        , m_Unknown0C(0)
        , morphWeights(0)
        , numMorphs(0)
        , numActiveMorphs(0)
        , morphWeightsChanged(true)
    {
    }

    virtual ~GLSkinMesh();
    virtual void SetModel(glModel* model);
    virtual glModel* GetModel();
    virtual void Pose(cPoseAccumulator* pPoseAccumulator) = 0;
    virtual void PrepareToRender() = 0;
    virtual void GetPoseMatrix(nlMatrix4* matrix, int nodeIndex) = 0;

    unsigned long GetNumPackets() { return GetModel()->GetNumPackets(); }
    int GetModelIndex() const { return m_Unknown0C; }

    void SetNumMorphs(unsigned long count);
    void SetMorphID(unsigned long index, unsigned long id);
    float GetMorphWeight(int index) const { return morphWeights[index].morphWeight; }
    void UpdateMorphWeights(cPoseAccumulator* pPoseAccumulator);
    void ApplyMorphOverride();
    unsigned long CountActiveMorphs() const
    {
        unsigned long count = 0;
        for (unsigned long i = 0; i < numMorphs; ++i)
        {
            if (morphWeights[i].morphWeight > 0.0f)
            {
                ++count;
            }
        }
        return count;
    }

    /* 0x04 */ glModel* pModel;
    /* 0x08 */ unsigned long hierarchySignature;
    /* 0x0C */ int m_Unknown0C;
    /* 0x10 */ MorphWeight* morphWeights;
    /* 0x14 */ unsigned long numMorphs;
    /* 0x18 */ unsigned long numActiveMorphs;
    /* 0x1C */ bool morphWeightsChanged;
};

#endif // GAME_GL_GL_SKIN_MESH_H
