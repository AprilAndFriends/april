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

#include <april/april.h>
#include <hltypes/hresource.h>

#include "StaticMesh.h"

namespace april
{
    StaticMesh::StaticMesh()
    {

    }
    
    StaticMesh::StaticMesh(chstr meshPath)
    {
        loadFromFile(meshPath);
        convertToVertexArray();
    }
    
    StaticMesh::~StaticMesh()
    {
		if (this->vertexArray != NULL)
		{
			delete [] this->vertexArray;
		}
    }
    
    void StaticMesh::convertToVertexArray()
    {
        this->vertexArray = new april::TexturedVertex[3 * polygons.size()];
        bool tex = (this->textureCoordinates.size() > 0);
        
        for (int i = 0; i < this->polygons.size(); ++i)
        {
            
            this->vertexArray[3 * i + 0].x = this->vertexes[this->polygons[i].vertind[0]].x;
            this->vertexArray[3 * i + 0].y = this->vertexes[this->polygons[i].vertind[0]].y;
            this->vertexArray[3 * i + 0].z = this->vertexes[this->polygons[i].vertind[0]].z;
            if (tex)
            {
                this->vertexArray[3 * i + 0].u = this->textureCoordinates[this->polygons[i].texind[0]].x;
                this->vertexArray[3 * i + 0].v = this->textureCoordinates[this->polygons[i].texind[0]].y;
            }
            this->vertexArray[3 * i + 1].x = this->vertexes[this->polygons[i].vertind[1]].x;
            this->vertexArray[3 * i + 1].y = this->vertexes[this->polygons[i].vertind[1]].y;
            this->vertexArray[3 * i + 1].z = this->vertexes[this->polygons[i].vertind[1]].z;
            if (tex)
            {
                this->vertexArray[3 * i + 1].u = this->textureCoordinates[this->polygons[i].texind[1]].x;
                this->vertexArray[3 * i + 1].v = this->textureCoordinates[this->polygons[i].texind[1]].y;
            }
            this->vertexArray[3 * i + 2].x = this->vertexes[this->polygons[i].vertind[2]].x;
            this->vertexArray[3 * i + 2].y = this->vertexes[this->polygons[i].vertind[2]].y;
            this->vertexArray[3 * i + 2].z = this->vertexes[this->polygons[i].vertind[2]].z;
            if (tex)
            {
                this->vertexArray[3 * i + 2].u = this->textureCoordinates[this->polygons[i].texind[2]].x;
                this->vertexArray[3 * i + 2].v = this->textureCoordinates[this->polygons[i].texind[2]].y;
            }
        }
        
        this->numVertexes = 3 * this->polygons.size();
    }
    
    void StaticMesh::loadFromFile(chstr meshPath)
    {
        FILE *fp;
        char buffer[256];
        hstr command;
        
        april::log("[aprilutil] loading mesh '" + meshPath + "'", "[aprilutil] ");
        
		//hresource file(meshPath);
        fp = fopen(meshPath.c_str(), "rb");
        if (fp == NULL)
		{
            throw file_not_found(meshPath);
		}
            
        while (fscanf(fp, "%s", buffer) > 0)
        {
            command = hstr(buffer);
            if (command == "v")
            {
                gtypes::Vector3 vert;
                if( fscanf(fp, "%f %f %f", &vert.x, &vert.y, &vert.z) != EOF )
                {
                    this->vertexes.push_back(vert);
                    //std::cerr << "Vertice (" << msh->vertexes.size() << ")" << "[" << vert.x << "," << vert.y << "," << vert.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if (command == "vn")
            {
                gtypes::Vector3 nor;
                if (fscanf(fp, "%f %f %f", &nor.x, &nor.y, &nor.z) != EOF)
                {
                    this->normals.push_back(nor);
                    //std::cerr << "Normal (" << msh->mNormals.size() << ")" << "[" << nor.x << "," << nor.y << "," << nor.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if (command == "vt")
            {
                gtypes::Vector2 uv;
                if (fscanf(fp, "%f %f", &uv.x, &uv.y) != EOF)
                {
                    this->textureCoordinates.push_back(uv);
                    //std::cerr << "TexCoord (" << msh->textureCoordinates.size() << ")" << "[" << uv.x << "," << uv.y << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if (command == "f")
            {
                Polygon poly;
                char buff1[64];
                char buff2[64];
                char buff3[64];
                hstr first;
				hstr second;
				hstr third;
                
                if (fscanf(fp, " %s %s %s", buff1, buff2, buff3) == EOF)
				{
                    throw faulty_obj_file(meshPath);
				}
                    
                first  = hstr(buff1);
                second = hstr(buff2);
                third  = hstr(buff3);
                
                if(first.count("/") == 0)
                {
                    poly.vertind[0] = ((int)first - 1);
                    poly.vertind[1] = ((int)second - 1);
                    poly.vertind[2] = ((int)third - 1);
                }
                
                else if(first.count("//") == 1)
                {
                    hstr tmp1, tmp2;
                    first.split("//", tmp1, tmp2);
                    poly.vertind[0] = ((int)tmp1 - 1);
                    poly.norind[0]  = ((int)tmp2 - 1);
                    
                    second.split("//", tmp1, tmp2);
                    poly.vertind[1] = ((int)tmp1 - 1);
                    poly.norind[1]  = ((int)tmp2 - 1);
                    
                    third.split("//", tmp1, tmp2);
                    poly.vertind[2] = ((int)tmp1 - 1);
                    poly.norind[2]  = ((int)tmp2 - 1);
                }
                
                else if(first.count("/") == 2)
                {
                    
                    hstr tmp1;
					hstr tmp2;
					hstr tmp3;
					hstr tmp;
                    first.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.vertind[0] = ((int)tmp1 - 1);
                    poly.texind[0]  = ((int)tmp2 - 1);
                    poly.norind[0]  = ((int)tmp3 - 1);
                    
                    second.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.vertind[1] = ((int)tmp1 - 1);
                    poly.texind[1]  = ((int)tmp2 - 1);
                    poly.norind[1]  = ((int)tmp3 - 1);
                    
                    third.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.vertind[2] = ((int)tmp1 - 1);
                    poly.texind[2]  = ((int)tmp2 - 1);
                    poly.norind[2]  = ((int)tmp3 - 1);
                }
                
                this->polygons.push_back(poly);
                
            }
            else if (command.starts_with("#"))
            {
                
            }
            else if (command.starts_with("mtllib"))
            {
                
            }
            else if (command.starts_with("usemtl"))
            {
                
            }
            else if (command == "s")
            {
                
            }
            else if (command == "o")
            {
                
            }
            else if (command == "g")
            {
                
            }
        }
    }
    
    void StaticMesh::setMeshName(chstr meshName)
    {
        this->meshName = meshName;
    }
    
    void StaticMesh::draw(april::RenderOp renderOp)
    {
        rendersys->render(renderOp, vertexArray, numVertexes);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    _faulty_obj_file::_faulty_obj_file(chstr filename, const char* source_file, int line) :
	    exception("'" + filename + "' is faulty wavefront file!", source_file, line)
	{
	}

}
