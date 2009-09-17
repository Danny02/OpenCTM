//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        dae.cpp
// Description: Implementation of the DAE (Collada) file format
//              importer/exporter.
//-----------------------------------------------------------------------------
// Copyright (c) 2009 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//-----------------------------------------------------------------------------

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <tinyxml.h>
#include "dae.h"

using namespace std;

enum Axis
{
  X,Y,Z,S,T
};

class Source
{
public:
  Source() : stride(0), count(0), offset(0)
  {
  }
  
  Source(const Source& copy) : array(copy.array), stride(copy.stride), count(copy.count), offset(copy.offset), params(copy.params)
  {
  }
  
  vector<float> array;
  size_t stride, count, offset;
  vector<string> params;
  //vector<Axis> axisOrder;
};

enum Semantic
{
  VERTEX,
  NORMAL,
  TEXCOORD,
  POSITIONS,
  UNKNOWN
};

struct Input
{
  string source;
  Semantic semantic;
  size_t offset;
};

Semantic ToSemantic(const string& semantic)
{
  if (semantic == "VERTEX")
    return VERTEX;
  else if (semantic == "NORMAL")
    return NORMAL;
  else if (semantic == "TEXCOORD")
    return TEXCOORD;
  else if (semantic == "POSITIONS")
    return POSITIONS;
  else
    return UNKNOWN;
}

void HandleP(TiXmlElement* p , vector<size_t>& array)
{
  istringstream strStream (p->GetText());
  char val[100];
  size_t value = 0;
  while (!strStream.eof()) {
    strStream >> val;
    value = atoi(val);
    array.push_back(value);
  }
}

Source& GetSource(map<string, Source >& sources, map<string, vector<Input> >& vertices,const string& source)
{
  map<string, Source >::iterator srcIterator = sources.find(source);
  if (srcIterator != sources.end())
    return srcIterator->second;
  map<string, vector<Input> >::iterator vertIterator = vertices.find(source);
  if (vertIterator != vertices.end() ) {
    for (vector<Input>::iterator i = vertIterator->second.begin(); i != vertIterator->second.end() ; ++i) {
      srcIterator = sources.find(i->source);
      if (srcIterator != sources.end())
        return srcIterator->second;
      
    }
  } else {
    throw string("Error");
  }
  
  return srcIterator->second;
}

void InsertVertNormalTexcoord(vector<Vector3>& vertVector,vector<Vector3>& normalVector,vector<Vector2>& texcoordVector, bool hasVerts, bool hasNormals, bool hasTexcoords,const string& vertSource ,const string& normalSource ,const string& texcoordSource ,size_t vertIndex , size_t normalIndex , size_t texcoordIndex, map<string, Source >& sources,map<string, vector<Input> >& vertices)
{
  if (hasVerts) {
    Source& src = GetSource(sources, vertices , vertSource);
    float x = 0, y = 0, z = 0;
    if (src.stride >= 1)
      x = src.array[src.offset + vertIndex*src.stride];
    if (src.stride >= 2)
      y = src.array[src.offset + vertIndex*src.stride + 1];
    if (src.stride >= 3)
      z = src.array[src.offset + vertIndex*src.stride + 2];
    vertVector.push_back(Vector3(x,y,z));
  }
  
  if (hasNormals) {
    Source& src = GetSource(sources, vertices , normalSource);
    float x = 0, y = 0, z = 0;
    if (src.stride >= 1)
      x = src.array[src.offset + normalIndex*src.stride];
    if (src.stride >= 2)
      y = src.array[src.offset + normalIndex*src.stride + 1];
    if (src.stride >= 3)
      z = src.array[src.offset + normalIndex*src.stride + 2];
    normalVector.push_back(Vector3(x,y,z) );
  }
  
  if (hasTexcoords) {
    Source& src = GetSource(sources, vertices , texcoordSource);
    float s = 0, t = 0;
    if (src.stride >= 1)
      s = src.array[src.offset + texcoordIndex*src.stride];
    if (src.stride >= 2)
      t = src.array[src.offset + texcoordIndex*src.stride + 1];
    
    texcoordVector.push_back(Vector2(s,t));
  }
}

/// Import a DAE file from a file.
void Import_DAE(const char * aFileName, Mesh &aMesh)
{
  // Clear the mesh
  aMesh.Clear();
  TiXmlDocument doc(aFileName);
  
  if (doc.LoadFile())
  {
    
    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem = hDoc.FirstChildElement().Element();
    TiXmlHandle hRoot(elem);
    
    map<string, Source > sources;
    size_t indicesOffset = 0, vertexOffset = 0, texcoordOffset = 0, normalOffset = 0;
    
    TiXmlHandle geometry = hRoot.FirstChild( "library_geometries" ).FirstChild("geometry");
    for(elem = geometry.ToElement(); elem; elem=elem->NextSiblingElement())
    {
      TiXmlHandle geometry(elem);
      
      TiXmlElement* meshElem =  geometry.FirstChild("mesh").ToElement();
      
      if(meshElem)
      {
        TiXmlHandle mesh(meshElem);
        
        TiXmlElement* sourceElem;
        for(sourceElem = mesh.FirstChild("source").ToElement(); sourceElem;
            sourceElem = sourceElem->NextSiblingElement())
        {
          if(string(sourceElem->Value()) != "source")
            continue;
          TiXmlHandle source(sourceElem);
          string id = source.ToElement()->Attribute("id");
          TiXmlElement* arr = sourceElem->FirstChild("float_array")->ToElement();
          string str = arr->GetText();
          istringstream strStream (str);
          sources.insert(make_pair(id, Source()));
          
          TiXmlElement* techniqueElem = sourceElem->FirstChild("technique_common")->ToElement();
          TiXmlElement* accessorElem = techniqueElem->FirstChild("accessor")->ToElement();
          
          sources[id].stride = atoi(accessorElem->Attribute("stride"));
          sources[id].count = atoi(accessorElem->Attribute("count"));
          sources[id].offset = atoi(accessorElem->Attribute("offset"));
          char val[100];
          float value = 0;
          while(!strStream.eof())
          {
            strStream >> val;
            value = float(atof(val));
            sources[id].array.push_back(value);
          }
        }
        
        TiXmlElement* verticesElem = mesh.FirstChild("vertices").ToElement();
        map<string, vector<Input> > vertices;
        if (verticesElem) {
          string id = verticesElem->Attribute("id");
          vertices.insert(make_pair(id, vector<Input>()));
          TiXmlElement* inputElem;
          for(inputElem = verticesElem->FirstChild("input")->ToElement();
              inputElem; inputElem = inputElem->NextSiblingElement())
          {
            if(string(inputElem->Value()) != "input")
              continue;
            
            vertices[id].push_back(Input());
            vertices[id].back().source = string(inputElem->Attribute("source")).substr(1);
            vertices[id].back().semantic = ToSemantic(inputElem->Attribute("semantic"));
          }
        }
        
        TiXmlElement* trianglesElem = mesh.FirstChild("triangles").ToElement();
        if(trianglesElem)
        {
          TiXmlHandle triangles(trianglesElem);
          vector<Input> inputs;
          bool hasVerts = false, hasNormals = false, hasTexcoords = false;
          string vertSource = "", normalSource = "", texcoordSource = ""; 
          TiXmlElement* inputElem;
          for(inputElem = triangles.FirstChild( "input" ).ToElement();
              inputElem; inputElem = inputElem->NextSiblingElement())
          {
            if(string(inputElem->Value()) != "input")
              continue;
            //TiXmlHandle input(inputElem);
            inputs.push_back(Input());
            inputs.back().source = string(inputElem->Attribute("source")).substr(1);
            inputs.back().offset = atoi(inputElem->Attribute("offset"));
            inputs.back().semantic = ToSemantic(inputElem->Attribute("semantic"));
            switch(inputs.back().semantic)
            {
              case VERTEX:
                hasVerts = true;
                vertSource = inputs.back().source;
                break;
              case NORMAL:
                hasNormals = true;
                normalSource = inputs.back().source;
                break;
              case TEXCOORD:
                hasTexcoords = true;
                texcoordSource = inputs.back().source;
                break;
              default:
                break;
            }
          }
          
          vector<size_t> pArray;
          TiXmlElement* p = triangles.FirstChild( "p" ).ToElement();
          
          HandleP(p,pArray);
          
          vector<size_t> indexVector;
          vector<Vector3> vertVector, normalVector;
          vector<Vector2> texcoordVector;
          map<size_t, map<size_t, map< size_t, size_t > > > prevIndices;
          size_t index = 0;
          for (size_t i = 0; i < pArray.size() ; i += inputs.size()) {
            size_t vertIndex = 0, normalIndex = 0, texcoordIndex = 0;
            for (vector<Input>::const_iterator j = inputs.begin(); j != inputs.end(); ++j) {
              switch (j->semantic) {
                case VERTEX:
                  vertIndex = pArray[i + j->offset];
                  break;
                case NORMAL:
                  normalIndex = pArray[i + j->offset];
                  break;
                case TEXCOORD:
                  texcoordIndex = pArray[i + j->offset];
                  break;
                default:
                  break;
              }
            }
            map<size_t, map<size_t, map< size_t, size_t > > >::iterator prevIt1 = prevIndices.find(vertIndex);
            
            if(prevIt1 != prevIndices.end())
            {
              map<size_t, map< size_t, size_t > >::iterator prevIt2 = prevIt1->second.find(normalIndex);
              if(prevIt2 != prevIt1->second.end())
              {
                map< size_t, size_t >::iterator prevIt3 = prevIt2->second.find(texcoordIndex);
                if(prevIt3 != prevIt2->second.end())
                {
                  indexVector.push_back(prevIt3->second);
                }
                else
                {
                  indexVector.push_back(index);
                  prevIt2->second.insert(make_pair(texcoordIndex, index));
                  InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
                  ++index;
                }
              }
              else
              {
                indexVector.push_back(index);
                prevIt1->second.insert(make_pair(normalIndex, map< size_t, size_t >()));
                prevIt1->second[normalIndex].insert(make_pair(texcoordIndex, index));
                InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
                ++index;
              }
            }
            else
            {
              indexVector.push_back(index);
              prevIndices.insert(make_pair(vertIndex,map<size_t, map< size_t, size_t > >()));
              prevIndices[vertIndex].insert(make_pair(normalIndex, map< size_t, size_t >()));
              prevIndices[vertIndex][normalIndex].insert(make_pair(texcoordIndex, index));
              InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
              ++index;
            }
            
          }
			
		  //TiXmlElement* polylistElem = mesh.FirstChild("polylist").ToElement();
			
          size_t indicesOff = indicesOffset, vertexOff = vertexOffset, normalOff = normalOffset, texcoordOff = texcoordOffset;
          indicesOffset += indexVector.size();
          vertexOffset += vertVector.size();
          normalOffset += normalVector.size();
          texcoordOffset += texcoordVector.size();
          aMesh.mIndices.resize(indicesOffset );
          aMesh.mVertices.resize(vertexOffset );
          aMesh.mNormals.resize(normalOffset );
          aMesh.mTexCoords.resize(texcoordOffset );
          
          for(size_t i = 0; i < indexVector.size(); ++i)
            aMesh.mIndices[indicesOff + i] = indexVector[i];
          
          for(size_t i = 0; i < vertVector.size(); ++i)
            aMesh.mVertices[vertexOff + i] = vertVector[i];
          
          for(size_t i = 0; i < normalVector.size(); ++i)
            aMesh.mNormals[normalOff + i] = normalVector[i];
          
          for(size_t i = 0; i < texcoordVector.size(); ++i)
            aMesh.mTexCoords[texcoordOff + i] = texcoordVector[i];
        }
      }
    }
  }
  else
    throw runtime_error("Could not open input file.");
}

/// Export a DAE file to a file.
void Export_DAE(const char * aFileName, Mesh &aMesh)
{
  throw runtime_error("Could not open input file.");
}
