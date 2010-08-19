/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include "StaticMesh.h"

#include <iostream>

namespace April
{
    
    StaticMesh::StaticMesh()
    {

    }
    
    StaticMesh::StaticMesh(hstr meshPath)
    {
        loadFromFile(meshPath);
        convertToVertexArray();
    }
    
    StaticMesh::~StaticMesh()
    {
        
    }
    
    void StaticMesh::convertToVertexArray()
    {
        m_vertexArray = new April::TexturedVertex[3*m_polygons.size()];
        
        for(int i = 0; i < m_polygons.size(); ++i)
        {
            
            m_vertexArray[3*i + 0].x = m_vertices[m_polygons[i].m_vertind[0]].x;
            m_vertexArray[3*i + 0].y = m_vertices[m_polygons[i].m_vertind[0]].y;
            m_vertexArray[3*i + 0].z = m_vertices[m_polygons[i].m_vertind[0]].z;
            m_vertexArray[3*i + 0].u = m_textureCoordinates[m_polygons[i].m_texind[0]].x;
            m_vertexArray[3*i + 0].v = m_textureCoordinates[m_polygons[i].m_texind[0]].y;
            
            m_vertexArray[3*i + 1].x = m_vertices[m_polygons[i].m_vertind[1]].x;
            m_vertexArray[3*i + 1].y = m_vertices[m_polygons[i].m_vertind[1]].y;
            m_vertexArray[3*i + 1].z = m_vertices[m_polygons[i].m_vertind[1]].z;
            m_vertexArray[3*i + 1].u = m_textureCoordinates[m_polygons[i].m_texind[1]].x;
            m_vertexArray[3*i + 1].v = m_textureCoordinates[m_polygons[i].m_texind[1]].y;
            
            m_vertexArray[3*i + 2].x = m_vertices[m_polygons[i].m_vertind[2]].x;
            m_vertexArray[3*i + 2].y = m_vertices[m_polygons[i].m_vertind[2]].y;
            m_vertexArray[3*i + 2].z = m_vertices[m_polygons[i].m_vertind[2]].z;
            m_vertexArray[3*i + 2].u = m_textureCoordinates[m_polygons[i].m_texind[2]].x;
            m_vertexArray[3*i + 2].v = m_textureCoordinates[m_polygons[i].m_texind[2]].y;
            
        }
        
        m_numVertices = 3*m_polygons.size();
    }
    
    void StaticMesh::loadFromFile(hstr meshPath)
    {
        FILE *fp;
        char buffer[256];
        hstr command;
        
        std::cerr << "[aprilutil] loading mesh '" << meshPath << "'" << std::endl;
        
        fp=fopen(meshPath.c_str(), "r");
        if( fp == NULL )
            throw file_not_found(meshPath);
            
        while( fscanf(fp, "%s", buffer) > 0 )
        {
            command = hstr(buffer);
            if(command == "v")
            {
                gtypes::Vector3 vert;
                if( fscanf(fp, "%f %f %f", &vert.x, &vert.y, &vert.z) != EOF )
                {
                    m_vertices.push_back(vert);
                    //std::cerr << "Vertice (" << msh->m_vertices.size() << ")" << "[" << vert.x << "," << vert.y << "," << vert.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if(command == "vn")
            {
                gtypes::Vector3 nor;
                if( fscanf(fp, "%f %f %f", &nor.x, &nor.y, &nor.z) != EOF )
                {
                    m_normals.push_back(nor);
                    //std::cerr << "Normal (" << msh->m_normals.size() << ")" << "[" << nor.x << "," << nor.y << "," << nor.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if(command == "vt")
            {
                gtypes::Vector2 uv;
                if( fscanf(fp, "%f %f", &uv.x, &uv.y) != EOF )
                {
                    m_textureCoordinates.push_back(uv);
                    //std::cerr << "TexCoord (" << msh->m_textureCoordinates.size() << ")" << "[" << uv.x << "," << uv.y << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if(command == "f")
            {
                Polygon poly;
                char buff1[64];
                char buff2[64];
                char buff3[64];
                hstr first, second, third;
                
                if(fscanf(fp, " %s %s %s", buff1, buff2, buff3) == EOF)
                    throw faulty_obj_file(meshPath);
                    
                first  = hstr(buff1);
                second = hstr(buff2);
                third  = hstr(buff3);
                
                if(first.count("/") == 0)
                {
                    poly.m_vertind[0] = ((int)first - 1);
                    poly.m_vertind[1] = ((int)second - 1);
                    poly.m_vertind[2] = ((int)third - 1);
                }
                
                else if(first.count("//") == 1)
                {
                    hstr tmp1, tmp2;
                    first.split("//", tmp1, tmp2);
                    poly.m_vertind[0] = ((int)tmp1 - 1);
                    poly.m_norind[0]  = ((int)tmp2 - 1);
                    
                    second.split("//", tmp1, tmp2);
                    poly.m_vertind[1] = ((int)tmp1 - 1);
                    poly.m_norind[1]  = ((int)tmp2 - 1);
                    
                    third.split("//", tmp1, tmp2);
                    poly.m_vertind[2] = ((int)tmp1 - 1);
                    poly.m_norind[2]  = ((int)tmp2 - 1);
                }
                
                else if(first.count("/") == 2)
                {
                    
                    hstr tmp1,tmp2,tmp3, tmp;
                    first.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.m_vertind[0] = ((int)tmp1 - 1);
                    poly.m_texind[0]  = ((int)tmp2 - 1);
                    poly.m_norind[0]  = ((int)tmp3 - 1);
                    
                    second.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.m_vertind[1] = ((int)tmp1 - 1);
                    poly.m_texind[1]  = ((int)tmp2 - 1);
                    poly.m_norind[1]  = ((int)tmp3 - 1);
                    
                    third.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.m_vertind[2] = ((int)tmp1 - 1);
                    poly.m_texind[2]  = ((int)tmp2 - 1);
                    poly.m_norind[2]  = ((int)tmp3 - 1);
                }
                
                m_polygons.push_back(poly);
                
            }
            else if(command.starts_with("#"))
            {
                
            }
            else if(command.starts_with("mtllib"))
            {
                
            }
            else if(command.starts_with("usemtl"))
            {
                
            }
            else if(command == "s")
            {
                
            }
            else if(command == "o")
            {
                
            }
            else if(command == "g")
            {
                
            }
        }
    }
    
    void StaticMesh::setMashName(hstr meshName)
    {
        m_meshName = meshName;
    }
    
    void StaticMesh::draw(April::RenderOp renderOp)
    {
        rendersys->render(renderOp, m_vertexArray, m_numVertices);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    _faulty_obj_file::_faulty_obj_file(chstr filename, const char* source_file, int line) :
	    exception("'" + filename + "' is faulty wavefront file!", source_file, line)
	{
	}
}
