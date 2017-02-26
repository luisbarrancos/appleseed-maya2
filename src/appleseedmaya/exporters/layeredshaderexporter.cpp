
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2017 Luis Barrancos, The appleseedhq Organization
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
#include "appleseedmaya/exporters/layeredshaderexporter.h"

// Standard headers.
#include <algorithm>
#include <sstream>
#include <vector>

// Maya headers.
#include <maya/MFnDependencyNode.h>

// appleseed.renderer headers.
#include "renderer/api/utility.h"

// appleseed.maya headers.
#include "appleseedmaya/attributeutils.h"
#include "appleseedmaya/exporters/exporterfactory.h"
#include "appleseedmaya/shadingnodemetadata.h"

namespace asf = foundation;
namespace asr = renderer;

namespace
{

struct LayeredShaderEntry
{
    LayeredShaderEntry(const MColor& col, float transp, const MColor& glow)
      : m_col(col)
      , m_transp(transp)
      , m_glow(glow)
    {
    }

    MColor  m_col;
    float   m_transp;
    MColor  m_glow;
};

} // unnamed

void LayeredShaderExporter::registerExporter()
{
    NodeExporterFactory::registerShadingNodeExporter(
        "layeredShader",
        &LayeredShaderExporter::create);
}

ShadingNodeExporter *LayeredShaderExporter::create(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
{
    return new LayeredShaderExporter(object, shaderGroup);
}

LayeredShaderExporter::LayeredShaderExporter(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
  : ShadingNodeExporter(object, shaderGroup)
{
}

void LayeredShaderExporter::exportParameterValue(
    const MPlug&        plug,
    const OSLParamInfo& paramInfo,
    asr::ParamArray&    shaderParams) const
{
    MFnDependencyNode depNodeFn(node());
    MStatus status;

    if (paramInfo.paramName == "in_color")
    {
        MPlug plug = depNodeFn.findPlug("inputs", &status);

        std::vector<LayeredShaderEntry> layeredShaderColors;
        layeredShaderColors.reserve(plug.numElements());

        for(size_t i = 0, e = plug.numElements(); i < e; ++i)
        {
            MPlug entry  = plug.elementByPhysicalIndex(i);
            MPlug color  = entry.child(0);
            MPlug transp = entry.child(1);
            MPlug glow   = entry.child(2);

            MColor c;
            AttributeUtils::get(color, c);

            float t;
            AttributeUtils::get(transp, t);

            MColor g;
            AttributeUtils::get(glow, g);

            layeredShaderColors.push_back(LayeredShaderEntry(c, t, g));
        }

        std::stringstream ssc;
        ssc << "color[] ";

        std::stringstream sst;
        sst << "float[] ";

        std::stringstream ssg;
        ssg << "color[] ";

        for(size_t i = 0, e = layeredShaderColors.size(); i < e; ++i)
        {
            ssc << layeredShaderColors[i].m_col.r   << " " << layeredShaderColors[i].m_col.g    << " " << layeredShaderColors[i].m_col.b << " ";
            sst << layeredShaderColors[i].m_transp  << " ";
            ssg << layeredShaderColors[i].m_glow.r  << " " << layeredShaderColors[i].m_glow.g   << " " << layeredShaderColors[i].m_glow.b << " ";
        }

        shaderParams.insert("in_color",         ssc.str().c_str());
        shaderParams.insert("in_transparency",  sst.str().c_str());
        shaderParams.insert("in_glowColor",     ssg.str().c_str());
    }
    else if (paramInfo.paramName == "in_transparency" || paramInfo.paramName == "in_glow")
    {
        // We save the transparency and glow at the same time we save the main colors.
    }
    else
    {
        ShadingNodeExporter::exportParameterValue(
            plug,
            paramInfo,
            shaderParams);
    }
}

