/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Domagoj Cerjan                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include "AprilUtilExport.h"

#include "hltypes/harray.h"
#include "hltypes/hstring.h"
#include "hltypes/exception.h"
#include "gtypes/Vector2.h"
#include "gtypes/Vector3.h"

#include "april/RenderSystem.h"

#include <stdio.h>

namespace April
{

    class AprilUtilExport Polygon
    {
    public:
        int mVertind[3];
        int mNorind[3];
        int mTexind[3];
    };
    
    
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class AprilUtilExport StaticMesh
	{
    
    public:
        hltypes::Array<gtypes::Vector3> mVertices;
        hltypes::Array<Polygon> mPolygons;
        hltypes::Array<gtypes::Vector3> mNormals;
        hltypes::Array<gtypes::Vector2> mTextureCoordinates;
        
        hstr mMeshName;
        
        April::TexturedVertex *mVertexArray;
        int mNumVertices;
        
        
        April::Texture *mTexture;
        
    public:
		StaticMesh();
        StaticMesh(chstr meshPath);
		~StaticMesh();

		//void recalculateNormals();
        //void setTexture(April::Texture *texture);
        void setMeshName(chstr meshName);
        
        void convertToVertexArray();
        void loadFromFile(chstr meshPath);
        
        void draw(April::RenderOp renderOp);
        

    };

    
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class AprilUtilExport _faulty_obj_file : public hltypes::exception
	{
	public:
		_faulty_obj_file(chstr filename, const char* source_file, int line);
	};
	#define faulty_obj_file(filename) hltypes::exception(filename, __FILE__, __LINE__)

}

#endif
