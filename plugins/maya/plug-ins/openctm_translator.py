"""
OpenCTM Exporter for Maya.
"""
import maya.api.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx

import sys
from ctypes import *
import openctm

__author__ = "Jonas Innala, Francesco Leacche"
__version__ = "0.2"

kPluginTranslatorTypeName = "ctm"
kOptionScript = "OpenCTMExporterScript"


class OpenCTMTranslator(OpenMayaMPx.MPxFileTranslator):
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

    def reader(self, fileObject, optionString, accessMode):
        try:
            options = self._parseOptionString(optionString)
            fullName = fileObject.fullName()
            if options.get('verbose', False):
                print("ctm::reader - Reading %s" % fullName)
                print("ctm::reader - Options %s" % optionString)
                print("ctm::reader - Mode    %s" % accessMode)
            # Create Maya mesh data
            self._importCtm(fullName, options)

        except Exception as e:
            sys.stderr.write("Failed to read file information\n")
            raise

    def writer(self, fileObject, optionString, accessMode):
        try:
            options = self._parseOptionString(optionString)
            fullName = fileObject.fullName()
            if options.get('verbose', False):
                print("ctm::writer - Writing    %s" % fullName)
                print("ctm::writer - Options    %s" % optionString)
                print("ctm::writer - Mode       %s" % accessMode)
                print("ctm::writer - Export Normals %r" % options.get(
                    'normals', False))
                print("ctm::writer - Export UVs %r" % options.get(
                    'uvs', False))
                print("ctm::writer - Export Colors %r" % options.get(
                    'colors', False))
            exportAll = (accessMode == self.kExportAccessMode or
                         accessMode == self.kSaveAccessMode)
            self._exportCtm(fullName, options, exportAll)

        except Exception as e:
            sys.stderr.write("Failed to export file\n")
            raise

    def _parseOptionString(self, optionString):
        return {k[0]: bool(int(k[1])) for k in [
            x.split('=') for x in optionString.split(';')] if len(k) == 2}

    def _exportCtm(self, fileName, exportOptions, exportAll):
        exportNormals = exportOptions.get('normals', False)
        exportUVs = exportOptions.get('uvs', False)
        # TODO: implement colors export
        exportColors = exportOptions.get('colors', False)
        if exportAll:
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
            if exportAll:
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

            meshNormals = normals = None
            if exportNormals:
                meshNormals = fnMesh.getNormals()
                normals = {}

            UVSets = u = v = uvs = None
            if exportUVs:
                UVSets = fnMesh.getUVSetNames()
                u, v = fnMesh.getUVs(UVSets[0])
                uvs = {}

            iterPolys = OpenMaya.MItMeshPolygon(dagPath)
            offset = 0
            maxPoints = 0
            while not iterPolys.isDone():
                if not iterPolys.hasValidTriangulation():
                    raise ValueError("The mesh has not valid triangulation")
                # Not used
                # uvSet = iterPolys.getUVSetNames()

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
                    localindices.append(indices[0])
                    localindices.append(indices[1])
                    localindices.append(indices[2])

                    localIndex = []
                    for gt in range(len(indices)):
                        for gv in range(len(polygonVertices)):
                            if indices[gt] == polygonVertices[gv]:
                                localIndex.append(gv)
                                break
                    if exportNormals:
                        normals[indices[0]] = meshNormals[
                            iterPolys.normalIndex(localIndex[0])]
                        normals[indices[1]] = meshNormals[
                            iterPolys.normalIndex(localIndex[1])]
                        normals[indices[2]] = meshNormals[
                            iterPolys.normalIndex(localIndex[2])]

                    if exportUVs and iterPolys.hasUVs():
                        uvID = [0, 0, 0]
                        for vtxInPolygon in range(3):
                            uvID[vtxInPolygon] = iterPolys.getUVIndex(
                                localIndex[vtxInPolygon], UVSets[0])
                        uvs[indices[0]] = (u[uvID[0]], v[uvID[0]])
                        uvs[indices[1]] = (u[uvID[1]], v[uvID[1]])
                        uvs[indices[2]] = (u[uvID[2]], v[uvID[2]])

                for i in localindices:
                    ctmvertices.append(meshPoints[i].x)
                    ctmvertices.append(meshPoints[i].y)
                    ctmvertices.append(meshPoints[i].z)
                    if exportNormals:
                        ctmnormals.append(normals[i][0])
                        ctmnormals.append(normals[i][1])
                        ctmnormals.append(normals[i][2])
                    if exportUVs and iterPolys.hasUVs():
                        ctmtexcoords.append(uvs[i][0])
                        ctmtexcoords.append(uvs[i][1])

                iterPolys.next(None)
            dagIterator.next()

        pindices = cast((openctm.CTMuint * len(ctmindices))
                        (), POINTER(openctm.CTMuint))
        pvertices = cast((openctm.CTMfloat * len(ctmvertices))
                         (), POINTER(openctm.CTMfloat))
        if exportNormals:
            pnormals = cast((openctm.CTMfloat * len(ctmnormals))
                            (), POINTER(openctm.CTMfloat))
        else:
            pnormals = None
        if exportUVs and ctmtexcoords:
            ptexcoords = cast((openctm.CTMfloat * len(ctmtexcoords))
                              (), POINTER(openctm.CTMfloat))

        for i in range(len(ctmindices)):
            pindices[i] = openctm.CTMuint(ctmindices[i])
        for i in range(len(ctmvertices)):
            pvertices[i] = openctm.CTMfloat(ctmvertices[i])
            if exportNormals:
                pnormals[i] = openctm.CTMfloat(ctmnormals[i])
        if exportUVs and ctmtexcoords:
            for i in range(len(ctmtexcoords)):
                ptexcoords[i] = openctm.CTMfloat(ctmtexcoords[i])

        context = openctm.ctmNewContext(openctm.CTM_EXPORT)
        # openctm.ctmCompressionMethod(context, openctm.CTM_METHOD_MG2)
        # Select compression level (0-9) - default 1
        # openctm.ctmCompressionLevel(context, 9)
        comment = "Exported with OpenCTM exporter using Maya"
        openctm.ctmFileComment(context, c_char_p(comment))
        openctm.ctmDefineMesh(context, pvertices, openctm.CTMuint(
            len(ctmvertices) / 3), pindices,
            openctm.CTMuint(len(ctmindices) / 3), pnormals)
        if exportUVs:
            if ctmtexcoords:
                openctm.ctmAddUVMap(context, ptexcoords,
                                    c_char_p("map1"), c_char_p())
            else:
                print("[Warning] Export UVs selected but "
                      "there are no available UVs to export")

        openctm.ctmSave(context, c_char_p(fileName))
        openctm.ctmFreeContext(context)
        e = openctm.ctmGetError(context)

        if e != 0:
            s = openctm.ctmErrorString(e)
            print(s)
            raise Exception(s)

    def _importCtm(self, fileName, importOptions):
        verbose = importOptions.get('verbose', False)
        context = openctm.ctmNewContext(openctm.CTM_IMPORT)

        # Extract file
        openctm.ctmLoad(context, fileName)
        e = openctm.ctmGetError(context)
        if e != 0:
            s = openctm.ctmErrorString(e)
            print(s)
            openctm.ctmFreeContext(context)
            raise Exception(s)

        # Extract indices
        triCount = openctm.ctmGetInteger(context, openctm.CTM_TRIANGLE_COUNT)
        # ctmIndices = openctm.ctmGetIntegerArray(context, openctm.CTM_INDICES)
        polyCount = [3] * triCount

        # Extract vertices
        vertCount = openctm.ctmGetInteger(context, openctm.CTM_VERTEX_COUNT)
        ctmVertices = openctm.ctmGetFloatArray(context, openctm.CTM_VERTICES)
        vertices = OpenMaya.MFloatPointArray()
        vertices.setLength(vertCount)

        # Extract Normals
        ctmVertNormals = None
        vertNormals = OpenMaya.MFloatPointArray()
        hasNormals = openctm.ctmGetInteger(
            context, openctm.CTM_HAS_NORMALS) == openctm.CTM_TRUE
        if hasNormals:
            ctmVertNormals = openctm.ctmGetFloatArray(
                context, openctm.CTM_NORMALS)
            vertNormals.setLength(vertCount)

        # Extract UVs
        hasUVs = openctm.ctmGetInteger(context, openctm.CTM_UV_MAP_COUNT) > 0
        ctmTexCoords = None
        uCoords = OpenMaya.MFloatArray()
        vCoords = OpenMaya.MFloatArray()

        if hasUVs:
            ctmTexCoords = openctm.ctmGetFloatArray(
                context, openctm.CTM_UV_MAP_1)
            uCoords.setLength(vertCount)
            vCoords.setLength(vertCount)
            textureFilename = openctm.ctmGetUVMapString(
                context, openctm.CTM_UV_MAP_1, openctm.CTM_FILE_NAME)
            # TODO: Load texture file
            if textureFilename:
                pass

        # Extract colors
        colorAttrib = openctm.ctmGetNamedAttribMap(context, "Color")
        hasVertexColors = colorAttrib != openctm.CTM_NONE
        ctmColors = None
        vertexColors = OpenMaya.MColorArray()
        if hasVertexColors:
            ctmColors = openctm.ctmGetFloatArray(context, colorAttrib)
            vertexColors.setLength(vertCount)

        pointToIndex = {}
        nrSkippedVertices = 0
        indices = []
        for i in range(vertCount):
            ctmVertIndex = i * 3
            p = (
                float(ctmVertices[ctmVertIndex]),
                float(ctmVertices[ctmVertIndex + 1]),
                float(ctmVertices[ctmVertIndex + 2])
            )
            if p not in pointToIndex:
                index = i - nrSkippedVertices
                pointToIndex[p] = index
                vertices[index].x = p[0]
                vertices[index].y = p[1]
                vertices[index].z = p[2]
                indices.append(index)

                if hasNormals:
                    vertNormals[index].x = float(ctmVertNormals[ctmVertIndex])
                    vertNormals[index].y = float(
                        ctmVertNormals[ctmVertIndex + 1])
                    vertNormals[index].z = float(
                        ctmVertNormals[ctmVertIndex + 2])

                if hasUVs:
                    ctmUVIndex = i * 2
                    uCoords[index] = float(ctmTexCoords[ctmUVIndex])
                    vCoords[index] = float(ctmTexCoords[ctmUVIndex + 1])

                if hasVertexColors:
                    ctmColIndex = i * 4
                    vertexColors[index].r = float(ctmColors[ctmColIndex])
                    vertexColors[index].g = float(ctmColors[ctmColIndex + 1])
                    vertexColors[index].b = float(ctmColors[ctmColIndex + 2])
                    vertexColors[index].a = float(ctmColors[ctmColIndex + 3])
            else:
                indices.append(pointToIndex[p])
                nrSkippedVertices += 1

        uniqVertCount = len(pointToIndex)
        vertices.setLength(uniqVertCount)
        vertNormals.setLength(uniqVertCount)

        if hasUVs:
            uCoords.setLength(uniqVertCount)
            vCoords.setLength(uniqVertCount)

        if hasVertexColors:
            vertexColors.setLength(uniqVertCount)

        if verbose:
            method = openctm.ctmGetInteger(
                context, openctm.CTM_COMPRESSION_METHOD)
            if method == openctm.CTM_METHOD_RAW:
                methodStr = "RAW"
            elif method == openctm.CTM_METHOD_MG1:
                methodStr = "MG1"
            elif method == openctm.CTM_METHOD_MG2:
                methodStr = "MG2"
            else:
                methodStr = "Unknown"
            print("File: %s" % fileName)
            print("Comment: %s" % str(openctm.ctmGetString(
                context, openctm.CTM_FILE_COMMENT)))
            print("Compression Method: %s" % methodStr)
            print("Vertices Count : %d" % vertCount)
            print("Unique Vertices Count : %d" % uniqVertCount)
            print("Triangles Count: %d" % triCount)
            print("Has normals: %r" % hasNormals)
            print("Has UVs: %r" % hasUVs)
            print("Has Vertex Colors: %r" % hasVertexColors)

        fnMesh = OpenMaya.MFnMesh()
        newMesh = fnMesh.create(
            vertices, polyCount, indices, uCoords, vCoords)

        if hasNormals:
            fnMesh.setVertexNormals(vertNormals, range(len(vertices)))

        if hasVertexColors:
            fnMesh.setVertexColors(vertexColors, range(len(vertices)))

        fnMesh.updateSurface()

        # Assign initial shading group
        slist = OpenMaya.MGlobal.getSelectionListByName("initialShadingGroup")
        initialSG = slist.getDependNode(0)

        fnSG = OpenMaya.MFnSet(initialSG)
        if fnSG.restriction() == OpenMaya.MFnSet.kRenderableOnly:
            fnSG.addMember(newMesh)


def translatorCreator():
    return OpenMayaMPx.asMPxPtr(OpenCTMTranslator())


def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(
        mobject, "Autodesk", __version__, "Any")

    try:
        mplugin.registerFileTranslator(
            kPluginTranslatorTypeName,
            None,
            translatorCreator,
            kOptionScript)  # ,
        # kPluginTranslatorDefaultOptions)

    except Exception as e:
        sys.stderr.write("Failed to register command: %s\n" %
                         kPluginTranslatorTypeName)
        raise


def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    print("Plug-in OpenCTM Exporter uninitialized")
    try:
        mplugin.deregisterFileTranslator(kPluginTranslatorTypeName)
    except Exception as e:
        sys.stderr.write("Failed to unregister command: %s\n" %
                         kPluginTranslatorTypeName)
        raise
