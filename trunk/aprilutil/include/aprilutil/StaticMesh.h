/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                  Domagoj Cerjan (domagoj.cerjan@gmail.com)                         *
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
        int m_vertind[3];
        int m_norind[3];
        int m_texind[3];
    };
    
    
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class AprilUtilExport StaticMesh
	{
    
    public:
        hltypes::Array<gtypes::Vector3> m_vertices;
        hltypes::Array<Polygon> m_polygons;
        hltypes::Array<gtypes::Vector3> m_normals;
        hltypes::Array<gtypes::Vector2> m_textureCoordinates;
        
        hstr m_meshName;
        
        April::TexturedVertex *m_vertexArray;
        int m_numVertices;
        
        
        April::Texture *m_texture;
        
    public:
		StaticMesh();
        StaticMesh(hstr meshPath);
		~StaticMesh();

		//void recalculateNormals();
        //void setTexture(April::Texture *texture);
        void setMashName(hstr meshName);
        
        void convertToVertexArray();
        void loadFromFile(hstr meshPath);
        
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
