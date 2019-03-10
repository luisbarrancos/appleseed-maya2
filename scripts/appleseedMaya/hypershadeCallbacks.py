
#
# This source file is part of appleseed.
# Visit https://appleseedhq.net/ for additional information and resources.
#
# This software is released under the MIT license.
#
# Copyright (c) 2016-2019 Esteban Tovagliari, The appleseedhq Organization
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

"""
Reference:
 ${MAYA_LOCATION}/docs/Commands/callbacks.html
"""

# Maya imports.
import maya.cmds as mc
import maya.mel as mel

# appleseedMaya imports.
from logger import logger


def hyperShadePanelBuildCreateMenuCallback():
    mc.menuItem(label="appleseed")
    mc.menuItem(divider=True)


def hyperShadePanelBuildCreateSubMenuCallback():
    return "rendernode/appleseed"


def hyperShadePanelPluginChangeCallback(classification, changeType):
    return "rendernode/appleseed" in classification


def createRenderNodeSelectNodeCategoriesCallback(flag, treeLister):
    if flag == "allWithAppleseedUp":
        mc.treeLister(treeLister, edit=True, selectPath="appleseed")


def createRenderNodePluginChangeCallback(classification):
    return "rendernode/appleseed" in classification


def renderNodeClassificationCallback():
    return "rendernode/appleseed"


def createAsRenderNode(nodeType=None, postCommand=None):

    classification = mc.getClassification(nodeType)
    logger.debug(
        "CreateAsRenderNode called: nodeType = {0}, class = {1}, pcmd = {2}".format(
            nodeType,
            classification,
            postCommand
        )
    )

    classification = mc.getClassification(nodeType)
    logger.debug(
        "CreateAsRenderNode called: nodeType = {0}, class = {1}, pcmd = {2}".format(
            nodeType,
            classification,
            postCommand
        )
    )

    for cl in classification:
        if "rendernode/appleseed/surface" in cl.lower():

            mat = mc.shadingNode(nodeType, asShader=True)
            shadingGroup = mc.sets(
                renderable=True,
                noSurfaceShader=True,
                empty=True,
                name=mat + "SG"
            )
            mc.connectAttr(mat + ".outColor", shadingGroup + ".surfaceShader")
            logger.debug("Created shading node {0} asShader".format(mat))

        elif "rendernode/appleseed/volume" in cl.lower():

            mat = mc.shadingNode(nodeType, asShader=True)
            shadingGroup = mc.sets(
                renderable=True,
                noSurfaceShader=True,
                empty=True,
                name=mat + "SG"
            )
            mc.connectAttr(mat + ".outColor", shadingGroup + ".volumeShader")
            logger.debug("Created shading node {0} asShader".format(mat))

        elif "rendernode/appleseed/displacement" in cl.lower():

            mat = mc.shadingNode(nodeType, asShader=True)
            shadingGroup = mc.sets(
                renderable=True,
                noSurfaceShader=True,
                empty=True,
                name=mat + "SG"
            )
            mc.connectAttr(mat + ".displacement", shadingGroup + ".displacementShader")
            logger.debug("Created shading node {0} asShader".format(mat))

        elif "rendernode/appleseed/texture/2d" in cl.lower():

            mat = mc.shadingNode(nodeType, asTexture=True)
            # We should connect our own manifold2d instead of the maya
            # place2dTexture, but we would loose the manipulator though.
            # TODO: add 2D manipulator
            placeTex = mc.shadingNode("place2dTexture", asUtility=True)
            mc.connectAttr(placeTex + ".outUV", mat + ".uv")
            mc.connectAttr(placeTex + ".outUvFilterSize", mat + ".uvFilterSize")
            logger.debug("Created shading node {0} asTexture2D".format(mat))

        elif "rendernode/appleseed/texture/3d" in cl.lower():

            mat = mc.shadingNode(nodeType, asTexture=True)
            # The same as above. Adding the manifold3d means loosing the 3d
            # placement manipulator.
            # TODO: add 3d manipulator
            placeTex = mc.shadingNode("place3dTexture", asUtility=True)
            mc.connectAttr(placeTex + ".wim[0]", mat + ".placementMatrix")
            logger.debug("Created shading node {0} asTexture3D".format(mat))

        elif "rendernode/appleseed/texture/environment" in cl.lower():

            mat = mc.shadingNode(nodeType, asTexture=True)
            # Same as place3d, or a custom environment placement node and
            # manipulator would be needed.
            placeTex = mc.shadingNode("place3dTexture", asUtility=True)
            mc.connectAttr(placeTex + ".wim[0]", mat + ".placementMatrix")
            logger.debug("Created shading node {0} environment".format(mat))

        else:
            mat = mc.shadingNode(nodeType, asUtility=True)
            logger.debug("Created shading node {0} asUtility".format(mat))

    if postCommand is not None:
        postCommand = postCommand.replace("%node", mat)
        postCommand = postCommand.replace("%type", '\"\"')
        mel.eval(postCommand)

    return ""


def createRenderNodeCommandCallback(postCommand, nodeType):
    # logger.debug("createRenderNodeCallback called!")

    for c in mc.getClassification(nodeType):
        if "rendernode/appleseed" in c.lower():

            buildNodeCmd = (
                "import appleseedMaya.hypershadeCallbacks;"
                "appleseedMaya.hypershadeCallbacks.createAsRenderNode"
                "(nodeType=\\\"{0}\\\", postCommand='{1}')").format(nodeType, postCommand)

            return "string $cmd = \"{0}\"; python($cmd);".format(buildNodeCmd)


def buildRenderNodeTreeListerContentCallback(tl, postCommand, filterString):

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/Surface",
        "rendernode/appleseed/surface",
        "-asShader",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/Volume",
        "rendernode/appleseed/volume",
        "-asShader",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/Displacement",
        "rendernode/appleseed/displacement",
        "-asShader",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/2D Textures",
        "rendernode/appleseed/texture/2d",
        "-asTexture",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/3D Textures",
        "rendernode/appleseed/texture/3d",
        "-asTexture",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/Environment Textures",
        "rendernode/appleseed/texture/environment",
        "-asTexture",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)

    melCmd = 'addToRenderNodeTreeLister("{0}", "{1}", "{2}", "{3}", "{4}", "{5}");'.format(
        tl,
        postCommand,
        "appleseed/Utilities",
        "rendernode/appleseed/utility",
        "-asUtility",
        ""
    )
    logger.debug("buildRenderNodeTreeListerContentCallback: mel = %s" % melCmd)
    mel.eval(melCmd)


def firstConnectedShaderCallback(nodeName):
    logger.debug("firstConnectedShaderCallback called!")

    classification = mc.getClassification(mc.nodeType(nodeName))

    if "rendernode/appleseed/surface" in classification.lower():
        return mc.listConnections(nodeName, source=True, destination=False)[0]
    else:
        shader = mc.listConnections(nodeName, type="shadingEngine", source=True, destination=False)
        if shader:
            return shader[0]

    return None


def allConnectedShadersCallback(nodeName):
    logger.debug("allConnectedShadersCallback called!")

    shaders = []
    connections = mc.listConnections(nodeName, source=True, destination=False)

    if not connections:
        return None

    for node in connections:
        classification = mc.getClassification(mc.nodeType(node))

        if any(shader in classification for shader in [
            "rendernode/appleseed/surface",
            "rendernode/appleseed/volume",
            "rendernode/appleseed/displacement"
        ]):
            shaders.append(shader)

    return ":".join(shaders) if shaders else None




def nodeCanBeUsedAsMaterialCallback(nodeId, nodeOwner):
    logger.debug(("nodeCanBeUsedAsMaterialCallback called: "
                  "nodeId = {0}, nodeOwner = {1}").format(nodeId, nodeOwner))

    if nodeOwner == 'appleseedMaya':
        return 1

    return 0
