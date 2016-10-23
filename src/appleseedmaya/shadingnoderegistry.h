
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2016 Esteban Tovagliari, The appleseedhq Organization
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

#ifndef APPLESEED_MAYA_SHADING_NODE_REGISTRY_H
#define APPLESEED_MAYA_SHADING_NODE_REGISTRY_H

// Standard headers.
#include <string>
#include <vector>

// Maya headers.
#include <maya/MObject.h>
#include <maya/MStatus.h>

// appleseed.foundation headers.
#include "foundation/utility/containers/dictionary.h"

// appleseed.renderer headers.
#include "renderer/api/shadergroup.h"


struct OSLShaderInfo
{
    OSLShaderInfo();

    explicit OSLShaderInfo(const renderer::ShaderQuery& q);

    bool getMetadataValue(const char *key, std::string& value)
    {
        if(metadata.dictionaries().exist(key))
        {
            const foundation::Dictionary& dict = metadata.dictionary(key);
            value = dict.get("value");
            return true;
        }

        return false;
    }

    template <typename T>
    bool getMetadataValue(const char *key, T& value) const
    {
        if(metadata.dictionaries().exist(key))
        {
            const foundation::Dictionary& dict = metadata.dictionary(key);
            value = dict.get<T>("value");
            return true;
        }

        return false;
    }

    std::string shaderName;
    std::string shaderType;

    std::string mayaName;
    std::string mayaClassification;
    unsigned int typeId;

    foundation::Dictionary metadata;
    std::vector<foundation::Dictionary> paramInfo;
};


class ShadingNodeRegistry
{
  public:

    static MStatus registerShadingNodes(MObject plugin);
    static MStatus unregisterShadingNodes(MObject plugin);

    static const OSLShaderInfo *getShaderInfo(const MString& nodeName);

  private:

    // Non-copyable.
    ShadingNodeRegistry(const ShadingNodeRegistry&);
    ShadingNodeRegistry& operator=(const ShadingNodeRegistry&);
};

#endif  // !APPLESEED_MAYA_SHADING_NODE_REGISTRY_H