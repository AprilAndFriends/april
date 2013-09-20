/// @file
/// @author  Kresimir Spes
/// @author  Domagoj Cerjan
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a static mesh.

#ifndef APRILUTIL_MESH_LOADER_H
#define APRILUTIL_MESH_LOADER_H

#include <stdio.h>

#include <april/RenderSystem.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/exception.h>

#include "aprilutilExport.h"

namespace april
{

    class aprilutilExport Polygon
    {
    public:
        int vertind[3];
        int norind[3];
        int texind[3];
    };
    
    
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class aprilutilExport StaticMesh
	{
    
    public:
        hltypes::Array<gvec3> vertexes;
        hltypes::Array<Polygon> polygons;
        hltypes::Array<gvec3> normals;
        hltypes::Array<gvec2> textureCoordinates;
        
        hstr meshName;
        
        april::TexturedVertex* vertexArray;
        int numVertexes;
        
        
        april::Texture* texture;
        
    public:
		StaticMesh();
        StaticMesh(chstr meshPath);
		~StaticMesh();

		//void recalculateNormals();
        //void setTexture(april::Texture *texture);
        void setMeshName(chstr meshName);
        
        void convertToVertexArray();
        void loadFromFile(chstr meshPath);
        
        void draw(april::RenderOp renderOp);
        

    };

    
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class aprilutilExport _faulty_obj_file : public hltypes::exception
	{
	public:
		_faulty_obj_file(chstr filename, const char* source_file, int line);
	};
	#define faulty_obj_file(filename) hltypes::exception(filename, __FILE__, __LINE__)

}

#endif