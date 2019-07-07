
//
// This source file is part of appleseed.
// Visit https://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2017-2019 Luis Barrancos, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "rampshaderexporter.h"

// appleseed-maya headers.
#include "appleseedmaya/attributeutils.h"
#include "appleseedmaya/exporters/exporterfactory.h"
#include "appleseedmaya/shadingnodemetadata.h"

// Build options header.
#include "foundation/core/buildoptions.h"

// appleseed.renderer headers.
#include "renderer/api/utility.h"

// Maya headers.
#include "appleseedmaya/_beginmayaheaders.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MString.h>
#include "appleseedmaya/_endmayaheaders.h"

// Standard headers.
#include <algorithm>
#include <array>
#include <sstream>
#include <vector>

namespace asr = renderer;

namespace
{
    const std::array<const MString, 2> valueAttrNames = {
        "specularRollOff",
        "reflectivity"
    };

    const std::array<const MString, 6> colorAttrNames = {
        "color",
        "transparency",
        "incandescence",
        "specularColor",
        "reflectedColor",
        "environment"
    };

    struct RampShaderColorsEntry
    {
        RampShaderColorsEntry(float pos, const MColor& col, int interp)
          : m_pos(pos)
          , m_col(col)
          , m_interp(interp)
        {
        }

        bool operator<(const RampShaderColorsEntry& other) const
        {
            return m_pos < other.m_pos;
        }

        float   m_pos;
        MColor  m_col;
        int     m_interp;
    };

    struct RampShaderValuesEntry
    {
        RampShaderValuesEntry(float pos, float val, int interp)
          : m_pos(pos)
          , m_val(val)
          , m_interp(interp)
        {
        }

        bool operator<(const RampShaderValuesEntry& other) const
        {
            return m_pos < other.m_pos;
        }

        float   m_pos;
        float   m_val;
        int     m_interp;
    };

    void exportColorParameter(const MFnDependencyNode& depNodeFn,
                              const std::array<const MString, 4>& rampAttrs,
                              asr::ParamArray& shaderParams,
                              MStatus& status)
    {
        const MPlug plug = depNodeFn.findPlug(rampAttrs[0], false, &status);

        std::vector<RampShaderColorsEntry> rampShaderColors;
        rampShaderColors.reserve(plug.numElements());

        for (unsigned int i = 0, e = plug.numElements(); i < e; ++i)
        {
            MPlug entry = plug.elementByPhysicalIndex(i);
            MPlug position = entry.child(0);
            MPlug color = entry.child(1);
            MPlug interp = entry.child(2);

            float p;
            AttributeUtils::get(position, p);

            MColor c;
            AttributeUtils::get(color, c);

            int in;
            AttributeUtils::get(interp, in);

            rampShaderColors.emplace_back(p, c, in);
        }

        std::sort(rampShaderColors.begin(), rampShaderColors.end());

        std::stringstream ssp;
        ssp << "float[] ";

        std::stringstream ssc;
        ssc << "color[] ";

        std::stringstream ssi;
        ssi << "int[] ";

        for (auto & rampShaderColor : rampShaderColors)
        {
            ssp << rampShaderColor.m_pos << " ";
            ssc << rampShaderColor.m_col.r << " "
                << rampShaderColor.m_col.g << " "
                << rampShaderColor.m_col.b << " ";
            ssi << rampShaderColor.m_interp << " ";
        }

        shaderParams.insert(rampAttrs[1].asChar(), ssp.str().c_str());
        shaderParams.insert(rampAttrs[2].asChar(), ssc.str().c_str());
        shaderParams.insert(rampAttrs[3].asChar(), ssi.str().c_str());
    }

    void exportValueParameter(const MFnDependencyNode& depNodeFn,
                              const std::array<const MString, 4>& rampAttrs,
                              asr::ParamArray& shaderParams,
                              MStatus& status)
    {
        MPlug plug = depNodeFn.findPlug(rampAttrs[0], false, &status);

        std::vector<RampShaderValuesEntry> rampShaderValues;
        rampShaderValues.reserve(plug.numElements());

        for (unsigned int i = 0, e = plug.numElements(); i < e; ++i)
        {
            MPlug entry = plug.elementByPhysicalIndex(i);
            MPlug position = entry.child(0);
            MPlug value = entry.child(1);
            MPlug interp = entry.child(2);

            float p;
            AttributeUtils::get(position, p);

            float v;
            AttributeUtils::get(value, v);

            int in;
            AttributeUtils::get(interp, in);

            rampShaderValues.emplace_back(p, v, in);
        }

        std::sort(rampShaderValues.begin(), rampShaderValues.end());

        std::stringstream ssp;
        ssp << "float[] ";

        std::stringstream ssv;
        ssv << "float[] ";

        std::stringstream ssi;
        ssi << "int[] ";

        for (auto & rampShaderValues : rampShaderValues)
        {
            ssp << rampShaderValues.m_pos << " ";
            ssv << rampShaderValues.m_val << " ";
            ssi << rampShaderValues.m_interp << " ";
        }

        shaderParams.insert(rampAttrs[1].asChar(), ssp.str().c_str());
        shaderParams.insert(rampAttrs[2].asChar(), ssv.str().c_str());
        shaderParams.insert(rampAttrs[3].asChar(), ssi.str().c_str());
    }
}

void RampShaderExporter::registerExporter()
{
    NodeExporterFactory::registerShadingNodeExporter(
        "rampShader",
        &RampShaderExporter::create);
}

ShadingNodeExporter* RampShaderExporter::create(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
{
    return new RampShaderExporter(object, shaderGroup);
}

RampShaderExporter::RampShaderExporter(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
  : ShadingNodeExporter(object, shaderGroup)
{
}

void RampShaderExporter::exportParameterValue(
    const MPlug&        plug,
    const OSLParamInfo& paramInfo,
    asr::ParamArray&    shaderParams) const
{
    MFnDependencyNode depNodeFn(node());
    MStatus status;

    if (std::find(valueAttrNames.begin(),
                  valueAttrNames.end(),
                  paramInfo.paramName) != valueAttrNames.end())
    {
        for (auto & attrName : valueAttrNames)
        {
            const std::array<const MString, 4> rampAttrs = {
                attrName,
                "in_" + attrName + "_Position",
                "in_" + attrName + "_Value",
                "in_" + attrName + "_Interp"
            };

            if (paramInfo.paramName == rampAttrs[1])
            {
                exportValueParameter(depNodeFn,
                                     rampAttrs,
                                     shaderParams,
                                     status);
            }
        }
    }
    else if (std::find(colorAttrNames.begin(),
                       colorAttrNames.end(),
                       paramInfo.paramName) != colorAttrNames.end())
    {
        for (auto & attrName : colorAttrNames)
        {
            const std::array<const MString, 4> rampAttrs = {
                attrName,
                "in_" + attrName + "_Position",
                "in_" + attrName + "_Color",
                "in_" + attrName + "_Interp"
            };

            if (paramInfo.paramName == rampAttrs[1])
            {
                exportColorParameter(depNodeFn,
                                     rampAttrs,
                                     shaderParams,
                                     status);
            }
        }
    }
    else
    {
        ShadingNodeExporter::exportParameterValue(
                    plug,
                    paramInfo,
                    shaderParams);
    }
}
