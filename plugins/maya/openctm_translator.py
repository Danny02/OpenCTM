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

kPluginTranslatorTypeName = "OpenCTM Exporter"


class OpenCTMExporter(OpenMayaMPx.MPxFileTranslator):
    def __init__(self):
        OpenMayaMPx.MPxFileTranslator.__init__(self)

    def maya_useNewAPI():
        pass

    def haveWriteMethod(self):
        return True

    def haveReadMethod(self):
        return False

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
        raise NotImplementedError()


def translatorCreator():
    return OpenMayaMPx.asMPxPtr(OpenCTMExporter())


def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject, "Autodesk", "10.0", "Any")

    try:
        mplugin.registerFileTranslator(
            kPluginTranslatorTypeName, None, translatorCreator)
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
