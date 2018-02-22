"""
OpenCTM Exporter for Maya.
"""
import maya.api.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx

import sys
from ctypes import *
import openctm

__author__ = "Jonas Innala"
__version__ = "0.2"

kPluginTranslatorTypeName = "ctm"
kPluginTranslatorOptionsUIFunction = "exportOptions"
kPluginTranslatorDefaultOptions = "space=world;format=binary;verbose=true;"


class OpenCTMExporter(OpenMayaMPx.MPxFileTranslator):
    def __init__(self):
        OpenMayaMPx.MPxFileTranslator.__init__(self)

    def maya_useNewAPI():
        pass

    def haveWriteMethod(self):
        return True

    def haveReadMethod(self):
        return True

    def filter(self):
        return "*.ctm"

    def defaultExtension(self):
        return "ctm"

    def writer(self, fileObject, optionString, accessMode):
        fileName = fileObject.fullName()
        selection = OpenMaya.MSelectionList()
        all = (accessMode == self.kExportAccessMode or accessMode ==
               self.kSaveAccessMode)
        dagIterator = None
        if(all):
            dagIterator = OpenMaya.MItDag(
                OpenMaya.MItDag.kBreadthFirst, OpenMaya.MFn.kGeometric)
        else:
            selection = OpenMaya.MGlobal.getActiveSelectionList()
            dagIterator = OpenMaya.MItSelectionList(
                selection, OpenMaya.MFn.kGeometric)

        ctmindices = []
        ctmvertices = []
        ctmnormals = []
        ctmtexcoords = []
        indicesOffset = 0
        while not dagIterator.isDone():
            if (all):
                dagPath = dagIterator.getPath()
            else:
                dagPath = dagIterator.getDagPath()
            fnMesh = None
            try:
                fnMesh = OpenMaya.MFnMesh(dagPath)
            except Exception as e:
                dagIterator.next()
                continue
            meshPoints = fnMesh.getPoints(OpenMaya.MSpace.kWorld)

            meshNormals = fnMesh.getNormals()

            UVSets = fnMesh.getUVSetNames()
            u, v = fnMesh.getUVs(UVSets[0])
            iterPolys = OpenMaya.MItMeshPolygon(dagPath)
            offset = 0
            maxPoints = 0
            normals = {}
            uvs = {}
            while not iterPolys.isDone():
                if not iterPolys.hasValidTriangulation():
                    raise ValueError("The mesh has not valid triangulation")

                uvSet = iterPolys.getUVSetNames()

                polygonVertices = iterPolys.getVertices()
                numTriangles = iterPolys.numTriangles()

                offset = len(ctmvertices)
                localindices = []
                for i in range(numTriangles):

                    points, indices = iterPolys.getTriangle(i)
                    ctmindices.append(indicesOffset)
                    indicesOffset += 1
                    ctmindices.append(indicesOffset)
                    indicesOffset += 1
                    ctmindices.append(indicesOffset)
                    indicesOffset += 1
                    localindices.append(int(indices[0]))
                    localindices.append(int(indices[1]))
                    localindices.append(int(indices[2]))

                    localIndex = []
                    for gt in range(len(indices)):
                        for gv in range(len(polygonVertices)):
                            if indices[gt] == polygonVertices[gv]:
                                localIndex.append(gv)
                                break
                    normals[indices[0]] = meshNormals[
                        iterPolys.normalIndex(localIndex[0])]
                    normals[indices[1]] = meshNormals[
                        iterPolys.normalIndex(localIndex[1])]
                    normals[indices[2]] = meshNormals[
                        iterPolys.normalIndex(localIndex[2])]

                    uvID = [0, 0, 0]

                    if (iterPolys.hasUVs()):
                        for vtxInPolygon in range(3):
                            uvID[vtxInPolygon] = iterPolys.getUVIndex(
                                localIndex[vtxInPolygon], UVSets[0])
                        uvs[int(indices[0])] = (u[uvID[0]], v[uvID[0]])
                        uvs[int(indices[1])] = (u[uvID[1]], v[uvID[1]])
                        uvs[int(indices[2])] = (u[uvID[2]], v[uvID[2]])

                for i in localindices:
                    ctmvertices.append(float(meshPoints[i].x))
                    ctmvertices.append(float(meshPoints[i].y))
                    ctmvertices.append(float(meshPoints[i].z))
                    ctmnormals.append(normals[i][0])
                    ctmnormals.append(normals[i][1])
                    ctmnormals.append(normals[i][2])
                    if (iterPolys.hasUVs()):
                        ctmtexcoords.append(uvs[i][0])
                        ctmtexcoords.append(uvs[i][1])

                iterPolys.next(None)
            dagIterator.next()

        pindices = cast((openctm.CTMuint * len(ctmindices))
                        (), POINTER(openctm.CTMuint))
        pvertices = cast((openctm.CTMfloat * len(ctmvertices))
                         (), POINTER(openctm.CTMfloat))
        pnormals = cast((openctm.CTMfloat * len(ctmnormals))
                        (), POINTER(openctm.CTMfloat))
        if ctmtexcoords:
            ptexcoords = cast((openctm.CTMfloat * len(ctmtexcoords))
                              (), POINTER(openctm.CTMfloat))
        for i in range(len(ctmindices)):
            pindices[i] = openctm.CTMuint(ctmindices[i])
        for i in range(len(ctmvertices)):
            pvertices[i] = openctm.CTMfloat(ctmvertices[i])
            pnormals[i] = openctm.CTMfloat(ctmnormals[i])
        if ctmtexcoords:
            for i in range(len(ctmtexcoords)):
                ptexcoords[i] = openctm.CTMfloat(ctmtexcoords[i])

        context = openctm.ctmNewContext(openctm.CTM_EXPORT)
        comment = "Exported with OpenCTM exporter using Maya"
        openctm.ctmFileComment(context, c_char_p(comment))
        openctm.ctmDefineMesh(context, pvertices, openctm.CTMuint(
            len(ctmvertices) / 3), pindices,
            openctm.CTMuint(len(ctmindices) / 3), pnormals)
        if ctmtexcoords:
            openctm.ctmAddUVMap(context, ptexcoords, c_char_p(), c_char_p())
        openctm.ctmSave(context, c_char_p(fileName))
        openctm.ctmFreeContext(context)
        e = openctm.ctmGetError(context)

        if e != 0:
            s = openctm.ctmErrorString(e)
            print(s)
            raise Exception(s)
        else:
            return True

    def reader(self, fileObject, optionString, accessMode):
        try:
            optionsList = map(lambda x: map(
                str, x.split('=')), optionString.split(';'))
            optionsList = [x for x in optionsList if len(x) == 2]
            options = {}
            for option in optionsList:
                options[option[0]] = option[1]

            fullName = fileObject.fullName()
            # print("ctm::reader - Reading %s" % fullName)
            # print("ctm::reader - Options %s" % optionString)
            # print("ctm::reader - Mode    %s" % accessMode)

            # Create Maya mesh data
            self.importCtm(fullName, options)

        except Exception as e:
            sys.stderr.write("Failed to read file information\n")
            raise e

    def importCtm(self, ctmPath, importOptions):
        verbose = False
        if importOptions:
            if ('verbose' in importOptions and
                    importOptions['verbose'] == 'true'):
                verbose = True

        context = openctm.ctmNewContext(openctm.CTM_IMPORT)

        # Extract file
        openctm.ctmLoad(context, ctmPath)
        e = openctm.ctmGetError(context)
        if e != 0:
            s = openctm.ctmErrorString(e)
            print(s)
            openctm.ctmFreeContext(context)
            raise Exception(s)
        # openctm.ctmCompressionMethod(context, openctm.CTM_METHOD_MG2)
        # Select compression level (0-9) - default 1
        # openctm.ctmCompressionLevel(context, 4)
        fnMesh = OpenMaya.MFnMesh()

        # Extract indices
        triCount = openctm.ctmGetInteger(context, openctm.CTM_TRIANGLE_COUNT)
        # ctmIndices = openctm.ctmGetIntegerArray(context, openctm.CTM_INDICES)

        polyCount = [3] * triCount
        # Extract vertices
        vertCount = openctm.ctmGetInteger(context, openctm.CTM_VERTEX_COUNT)
        ctmVertices = openctm.ctmGetFloatArray(context, openctm.CTM_VERTICES)
        vertices = OpenMaya.MFloatPointArray()
        vertices.setLength(vertCount)

        pointToIndex = {}
        nrSkippedVertices = 0
        newIndices = []
        for i in range(vertCount):
            currIndex = i * 3
            p = (
                float(ctmVertices[currIndex]),
                float(ctmVertices[currIndex + 1]),
                float(ctmVertices[currIndex + 2])
            )
            if p not in pointToIndex:
                index = i - nrSkippedVertices
                pointToIndex[p] = index
                vertices[index].x = p[0]
                vertices[index].y = p[1]
                vertices[index].z = p[2]
                newIndices.append(index)
            else:
                newIndices.append(pointToIndex[p])
                nrSkippedVertices += 1
        vertices.setLength(len(pointToIndex))

        if verbose:
            print("Vertices Count : %d" % vertCount)
            print("Unique Vertices Count : %d" % len(pointToIndex))
            print("Triangles Count: %s" % triCount)

        newMesh = fnMesh.create(vertices, polyCount, newIndices)

        # Extract normals
        if openctm.ctmGetInteger(context, openctm.CTM_HAS_NORMALS) == openctm.CTM_TRUE:
            normals = openctm.ctmGetFloatArray(context, openctm.CTM_NORMALS)

        # Extract texture coordinates
        if openctm.ctmGetInteger(context, openctm.CTM_UV_MAP_COUNT) > 0:

            texCoords = openctm.ctmGetFloatArray(context, openctm.CTM_UV_MAP_1)
            textureFilename = openctm.ctmGetUVMapString(
                context, openctm.CTM_UV_MAP_1, openctm.CTM_FILE_NAME)
            if textureFilename:
                pass

        # Extract colors
        colorAttrib = openctm.ctmGetNamedAttribMap(context, "Color")
        if colorAttrib != openctm.CTM_NONE:
            colors = openctm.ctmGetFloatArray(context, colorAttrib)
        # Assign initial shading group
        initialSG = OpenMaya.MObject()
        slist = OpenMaya.MGlobal.getSelectionListByName("initialShadingGroup")
        initialSG = slist.getDependNode(0)

        fnSG = OpenMaya.MFnSet(initialSG)
        if fnSG.restriction() == OpenMaya.MFnSet.kRenderableOnly:
            fnSG.addMember(newMesh)


def translatorCreator():
    return OpenMayaMPx.asMPxPtr(OpenCTMExporter())


def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(
        mobject, "Autodesk", __version__, "Any")

    try:
        ctmOptions = createMelPythonCallback(
            "ctm", "exportOptions", parametersList=[
                ('string', 'parent'),
                ('string', 'action'),
                ('string', 'initialSettings'),
                ('string', 'resultCallback')],
            returnType="int")
        mplugin.registerFileTranslator(
            kPluginTranslatorTypeName,
            None,
            translatorCreator,
            ctmOptions,
            kPluginTranslatorDefaultOptions)

    except Exception as e:
        sys.stderr.write("Failed to register command: %s\n" %
                         kPluginTranslatorTypeName)
        raise e


def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    print("Plug-in OpenCTM Exporter uninitialized")
    try:
        mplugin.deregisterFileTranslator(kPluginTranslatorTypeName)
    except Exception as e:
        sys.stderr.write("Failed to unregister command: %s\n" % kPluginCmdName)
        raise e


def createMelPythonCallback(module, function, options=False,
                            parametersList=None, returnType=None,
                            returnTypeIsArray=False):
    mel = ""

    # Arbitrary parameter list and return type
    if parametersList:
        mel += "global proc "

        # return value
        if returnType is None:
            mel += "string"
        else:
            mel += returnType

        if returnTypeIsArray:
            mel += "[]"

        # function name
        mel += " melPythonCallbackOptions_%s_%s(" % (module, function)

        # parameter list
        paramString = ""
        for (paramType, paramName) in parametersList:
            paramString = paramString + \
                ("%s $%s" % (paramType, paramName)) + ", "
        paramString = paramString[:-2]
        mel += "%s) { " % paramString

        # return type
        if returnType is None:
            mel += "    string $result"
        else:
            mel += "    %s $result" % returnType

        if returnTypeIsArray:
            mel += "[]"

        # python module and function
        mel += " = python( \"import %s; %s.%s(" % (module, module, function)

        # arguments for python function
        for (paramType, paramName) in parametersList:
            mel += "\\\"\" + $%s + \"\\\"," % paramName

        mel += ")\" ); "
        mel += "    return $result; "
        mel += "} "

        # mel function call
        mel += "melPythonCallbackOptions_%s_%s" % (module, function)

    # Return value for python mel command is documented as string[] but seems to return
    # string in some cases. Commands that don't have options are assumed to return string.
    # Commands with options are assumed to return string[]
    elif options:

        mel += "global proc string[] melPythonCallbackOptions_%s_%s(string $options) { " % (
            module, function)
        mel += "    string $result[] = python( \"import %s; %s.%s(\\\"\" + $options + \"\\\")\" ); " % (
            module, module, function)
        mel += "    return $result; "
        mel += "} "
        mel += "melPythonCallbackOptions_%s_%s" % (module, function)
    else:
        mel += "global proc string melPythonCallback_%s_%s() { " % (
            module, function)
        mel += "    string $result = `python( \"import %s; %s.%s()\" )`; " % (
            module, module, function)
        mel += "    return $result; "
        mel += "} "
        mel += "melPythonCallback_%s_%s" % (module, function)

    return mel
