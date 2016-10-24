
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


class OSLMetadataExtractor
{
  public:
    explicit OSLMetadataExtractor(const foundation::Dictionary& metadata)
      : m_metadata(metadata)
    {
    }

    bool exists(const char *key) const
    {
        return m_metadata.dictionaries().exist(key);
    }

    bool getValue(const char *key, std::string& value)
    {
        if(exists(key))
        {
            const foundation::Dictionary& dict = m_metadata.dictionary(key);
            value = dict.get("value");
            return true;
        }

        return false;
    }

    template <typename T>
    bool getValue(const char *key, T& value) const
    {
        if(exists(key))
        {
            const foundation::Dictionary& dict = m_metadata.dictionary(key);
            value = dict.get<T>("value");
            return true;
        }

        return false;
    }

  private:

    // Non-copyable.
    OSLMetadataExtractor(const OSLMetadataExtractor&);
    OSLMetadataExtractor& operator=(const OSLMetadataExtractor&);

    const foundation::Dictionary& m_metadata;
};

class OSLParamInfo
{
  public:
    explicit OSLParamInfo(const foundation::Dictionary& paramInfo);

    // Query info.
    std::string paramName;
    std::string paramType;
    bool validDefault;
    //T default_value
    bool isOutput;
    bool isClosure;
    bool isStruct;
    std::string structName;
    bool isArray;
    int arrayLen;

    // Standard metadata info.
    std::string label;
    std::string help;
    std::string page;
    std::string widget;
    std::string options;
    // More standard metadata options...

    // appleseedMaya custom metadata.
    std::string mayaName;
};

class OSLShaderInfo
{
  public:
    OSLShaderInfo();

    explicit OSLShaderInfo(const renderer::ShaderQuery& q);

    std::string shaderName;
    std::string shaderType;

    std::string mayaName;
    std::string mayaClassification;
    unsigned int typeId;

    std::vector<OSLParamInfo> paramInfo;
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
