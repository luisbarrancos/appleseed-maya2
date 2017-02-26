
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
#include "appleseedmaya/exporters/layeredtextureexporter.h"

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

struct LayeredTextureEntry
{
    LayeredTextureEntry(const MColor& col, float alpha, int mode, bool vis)
      : m_col(col)
      , m_alpha(alpha)
      , m_mode(mode)
      , m_vis(vis)
    {
    }

    MColor  m_col;
    float   m_alpha;
    int     m_mode;
    bool    m_vis;
};

} // unnamed

void LayeredTextureExporter::registerExporter()
{
    NodeExporterFactory::registerShadingNodeExporter(
        "layeredTexture",
        &LayeredTextureExporter::create);
}

ShadingNodeExporter *LayeredTextureExporter::create(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
{
    return new LayeredTextureExporter(object, shaderGroup);
}

LayeredTextureExporter::LayeredTextureExporter(
    const MObject&      object,
    asr::ShaderGroup&   shaderGroup)
  : ShadingNodeExporter(object, shaderGroup)
{
}

void LayeredTextureExporter::exportParameterValue(
    const MPlug&        plug,
    const OSLParamInfo& paramInfo,
    asr::ParamArray&    shaderParams) const
{
    MFnDependencyNode depNodeFn(node());
    MStatus status;

    if (paramInfo.paramName == "in_color")
    {
        MPlug plug = depNodeFn.findPlug("inputs", &status);

        std::vector<LayeredTextureEntry> layeredTextureColors;
        layeredTextureColors.reserve(plug.numElements());

        for(size_t i = 0, e = plug.numElements(); i < e; ++i)
        {
            MPlug entry = plug.elementByPhysicalIndex(i);
            MPlug color = entry.child(0);
            MPlug alpha = entry.child(1);
            MPlug mode  = entry.child(2);
            MPlug vis   = entry.child(3);

            MColor c;
            AttributeUtils::get(color, c);

            float a;
            AttributeUtils::get(alpha, a);

            int m;
            AttributeUtils::get(mode, m);

            bool v;
            AttributeUtils::get(vis, v);

            layeredTextureColors.push_back(LayeredTextureEntry(c, a, m, v));
        }

        std::stringstream ssc;
        ssc << "color[] ";

        std::stringstream ssa;
        ssa << "float[] ";

        std::stringstream ssm;
        ssm << "int[] ";

        std::stringstream ssv;
        ssv << "int[] ";

        for(size_t i = 0, e = layeredTextureColors.size(); i < e; ++i)
        {
            ssc << layeredTextureColors[i].m_col.r  << " " << layeredTextureColors[i].m_col.g    << " " << layeredTextureColors[i].m_col.b << " ";
            ssa << layeredTextureColors[i].m_alpha  << " ";
            ssm << layeredTextureColors[i].m_mode   << " ";
            ssv << layeredTextureColors[i].m_vis    << " ";
        }

        shaderParams.insert("in_color",     ssc.str().c_str());
        shaderParams.insert("in_alpha",     ssa.str().c_str());
        shaderParams.insert("in_blendMode", ssm.str().c_str());
        shaderParams.insert("in_isVisible", ssv.str().c_str());
    }
    else if (paramInfo.paramName == "in_alpha" || paramInfo.paramName == "in_blendMode" || paramInfo.paramName == "in_isVisible")
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

