// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file HairToonMaterial.cc
/// $Id$
///
//

#include "attributes.cc"
#include "HairToonMaterial_ispc_stubs.h"
#include "labels.h"

#include <moonray/rendering/shading/MaterialApi.h>

#include <moonshine/material/dwabase/DwaBase.h>
#include <moonshine/material/dwabase/DwaBaseLayerable.h>

using namespace scene_rdl2::math;
using namespace moonray;
using namespace moonshine::dwabase;

namespace {

DECLARE_DWA_BASE_LABELS()

DwaBaseAttributeKeys
collectAttributeKeys()
{
    DwaBaseAttributeKeys keys;

    keys.mPresence = attrPresence;
    keys.mShowEmission = attrShowEmission;
    keys.mEmission = attrEmission;

    keys.mShowHairDiffuse = attrShowHairDiffuse;
    keys.mHairDiffuse = attrHairDiffuse;

    keys.mHairColor = attrHairColor;
    keys.mHairiDiffuseUseIndependentFrontAndBackColor = attrUseIndependentFrontAndBackHairColor;
    keys.mHairDiffuseFrontColor = attrFrontHairColor;
    keys.mHairDiffuseBackColor = attrBackHairColor;

    keys.mHairSubsurfaceBlend = attrHairSubsurfaceBlend;
    keys.mSubsurface = attrHairSubsurface;
    keys.mScatteringColor = attrHairScatteringColor;
    keys.mScatteringRadius = attrHairScatteringRadius;
    keys.mSubsurfaceTraceSet = attrHairSubsurfaceTraceSet;
    keys.mEnableSubsurfaceInputNormal = attrHairEnableSubsurfaceInputNormal;

    keys.mInputNormal = attrHairInputNormal;
    keys.mInputNormalDial = attrHairInputNormalDial;

    keys.mHairToonS1Keys.mShow = attrHairToonS1Show;
    keys.mHairToonS1Keys.mModel = attrHairToonS1Model;
    keys.mHairToonS1Keys.mRoughness = attrHairToonS1Roughness;
    ASSIGN_TOON_SPEC_ATTR_KEYS(keys, HairToonS1);

    keys.mHairToonS2Keys.mShow = attrHairToonS2Show;
    keys.mHairToonS2Keys.mModel = attrHairToonS2Model;
    keys.mHairToonS2Keys.mRoughness = attrHairToonS2Roughness;
    ASSIGN_TOON_SPEC_ATTR_KEYS(keys, HairToonS2);

    keys.mHairToonS3Keys.mShow = attrHairToonS3Show;
    keys.mHairToonS3Keys.mModel = attrHairToonS3Model;
    keys.mHairToonS3Keys.mRoughness = attrHairToonS3Roughness;
    ASSIGN_TOON_SPEC_ATTR_KEYS(keys, HairToonS3);

    return keys;
}

} // end anonymous namespace


//---------------------------------------------------------------------------
/// @brief A Kajiya Kay Diffuse Hair Shading Material
RDL2_DSO_CLASS_BEGIN(HairToonMaterial, DwaBase)

public:
    HairToonMaterial(const SceneClass& sceneClass, const std::string& name);

private:
    virtual void update() override;

    static void shade(const Material* self,
                      moonray::shading::TLState *tls,
                      const moonray::shading::State& state,
                      moonray::shading::BsdfBuilder& bsdfBuilder);

    static Vec3f evalSubsurfaceNormal(const Material* self,
                                      moonray::shading::TLState *tls,
                                      const moonray::shading::State& state);

RDL2_DSO_CLASS_END(HairToonMaterial)


HairToonMaterial::HairToonMaterial(const SceneClass& sceneClass, const std::string& name) :
    DwaBase(sceneClass, name,
            collectAttributeKeys(),
            ispc::HairToonMaterial_collectAttributeFuncs(),
            sLabels,
            ispc::Model::HairToon)
{
    mType |= INTERFACE_DWABASEHAIRLAYERABLE;

    mShadeFunc = HairToonMaterial::shade;
    mShadeFuncv = (ShadeFuncv) ispc::HairToonMaterial_getShadeFunc();
}

void
HairToonMaterial::update()
{
    // must call DwaBase::update()!
    DwaBase::update();

    // set bssrdf normal map
    ispc::DwaBase* dwabase = getISPCBaseMaterialStruct();
    dwabase->mAttrFuncs.mEvalSubsurfaceNormal = getEnableSubsurfaceInputNormal() ?
            (intptr_t)HairToonMaterial::evalSubsurfaceNormal : 0;

    if (get(attrHairToonS1Show)) {
        const std::vector<float>& positions = get(attrHairToonS1RampPositions);
        const std::vector<float>& values = get(attrHairToonS1RampValues);
        const std::vector<int>& interps = get(attrHairToonS1RampInterpolators);
        dwabase->mHairToonS1Data.mRampNumPoints = positions.size();
        for (size_t i = 0; i < positions.size(); ++i) {
            dwabase->mHairToonS1Data.mRampPositions[i] = positions[i];
            dwabase->mHairToonS1Data.mRampValues[i] = values[i];
            dwabase->mHairToonS1Data.mRampInterpolators[i] = (ispc::RampInterpolatorMode)interps[i];
        }
    }

    if (get(attrHairToonS2Show)) {
        const std::vector<float>& positions = get(attrHairToonS2RampPositions);
        const std::vector<float>& values = get(attrHairToonS2RampValues);
        const std::vector<int>& interps = get(attrHairToonS2RampInterpolators);
        dwabase->mHairToonS2Data.mRampNumPoints = positions.size();
        for (size_t i = 0; i < positions.size(); ++i) {
            dwabase->mHairToonS2Data.mRampPositions[i] = positions[i];
            dwabase->mHairToonS2Data.mRampValues[i] = values[i];
            dwabase->mHairToonS2Data.mRampInterpolators[i] = (ispc::RampInterpolatorMode)interps[i];
        }
    }

    if (get(attrHairToonS3Show)) {
        const std::vector<float>& positions = get(attrHairToonS3RampPositions);
        const std::vector<float>& values = get(attrHairToonS3RampValues);
        const std::vector<int>& interps = get(attrHairToonS3RampInterpolators);
        dwabase->mHairToonS3Data.mRampNumPoints = positions.size();
        for (size_t i = 0; i < positions.size(); ++i) {
            dwabase->mHairToonS3Data.mRampPositions[i] = positions[i];
            dwabase->mHairToonS3Data.mRampValues[i] = values[i];
            dwabase->mHairToonS3Data.mRampInterpolators[i] = (ispc::RampInterpolatorMode)interps[i];
        }
    }
}

void
HairToonMaterial::shade(const Material* self,
                        moonray::shading::TLState *tls,
                        const moonray::shading::State &state,
                        moonray::shading::BsdfBuilder &bsdfBuilder)
{
    const HairToonMaterial* me = static_cast<const HairToonMaterial*>(self);
    const ispc::DwaBase* dwabase = me->getISPCBaseMaterialStruct();

    ispc::DwaBaseParameters params;
    me->resolveParameters(tls, state, false, params);
    me->createLobes(me, tls, state, bsdfBuilder, params, dwabase->mUParams, sLabels);
}

Vec3f
HairToonMaterial::evalSubsurfaceNormal(const Material* self,
                                       moonray::shading::TLState *tls,
                                       const moonray::shading::State& state)
{
    const HairToonMaterial* me = static_cast<const HairToonMaterial*>(self);
    return me->resolveSubsurfaceNormal(tls, state);
}

//---------------------------------------------------------------------------

