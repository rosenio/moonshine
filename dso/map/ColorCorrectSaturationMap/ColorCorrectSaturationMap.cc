// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file ColorCorrectSaturationMap.cc

#include "attributes.cc"
#include "ColorCorrectSaturationMap_ispc_stubs.h"

#include <moonray/rendering/shading/MapApi.h>
#include <moonray/rendering/shading/ColorCorrect.h>

using namespace moonray::shading;
using namespace scene_rdl2::math;
using namespace moonray::pbr;

//----------------------------------------------------------------------------

RDL2_DSO_CLASS_BEGIN(ColorCorrectSaturationMap, scene_rdl2::rdl2::Map)

public:
    ColorCorrectSaturationMap(const scene_rdl2::rdl2::SceneClass &sceneClass, const std::string &name);
    ~ColorCorrectSaturationMap() override;
    void update() override;

private:
    static void sample(const scene_rdl2::rdl2::Map *self, moonray::shading::TLState *tls,
                       const moonray::shading::State &state, Color *sample);

RDL2_DSO_CLASS_END(ColorCorrectSaturationMap)

//----------------------------------------------------------------------------

ColorCorrectSaturationMap::ColorCorrectSaturationMap(const scene_rdl2::rdl2::SceneClass &sceneClass, const std::string &name):
    Parent(sceneClass, name)
{
    mSampleFunc = ColorCorrectSaturationMap::sample;
    mSampleFuncv = (scene_rdl2::rdl2::SampleFuncv) ispc::ColorCorrectSaturationMap_getSampleFunc();
}

ColorCorrectSaturationMap::~ColorCorrectSaturationMap()
{
}

void
ColorCorrectSaturationMap::update()
{
}

void
ColorCorrectSaturationMap::sample(const scene_rdl2::rdl2::Map *self, moonray::shading::TLState *tls,
                        const moonray::shading::State &state, Color *sample)
{
    const ColorCorrectSaturationMap* me = static_cast<const ColorCorrectSaturationMap*>(self);

    const Color input = evalColor(me, attrInput, tls, state);
    Color result(input);

    const float mix = evalFloat(me, attrMix, tls, state);
    if (!me->get(attrOn) || isZero(mix)) {
        *sample = result;
        return;
    }

    Color saturation;
    if (me->get(attrSaturationPerChannel)) {
        saturation = Color(evalFloat(me, attrSaturationR, tls, state),
                           evalFloat(me, attrSaturationG, tls, state),
                           evalFloat(me, attrSaturationB, tls, state));
    } else {
        saturation = Color(evalFloat(me, attrSaturation, tls, state));
    }
    applySaturation(saturation, result);

    if (!isEqual(mix, 1.0f)) {
        result.r = lerpOpt(input.r, result.r, mix);
        result.g = lerpOpt(input.g, result.g, mix);
        result.b = lerpOpt(input.b, result.b, mix);
    }

    *sample = result;
}

