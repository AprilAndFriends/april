/// @file
/// @author  Kresimir Spes
/// @author  Domagoj Cerjan
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <iostream>

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
        
    }
    
    void StaticMesh::convertToVertexArray()
    {
        mVertexArray = new april::TexturedVertex[3*mPolygons.size()];
        bool tex=mTextureCoordinates.size() > 0;
        
        for(int i = 0; i < mPolygons.size(); ++i)
        {
            
            mVertexArray[3*i + 0].x = mVertices[mPolygons[i].mVertind[0]].x;
            mVertexArray[3*i + 0].y = mVertices[mPolygons[i].mVertind[0]].y;
            mVertexArray[3*i + 0].z = mVertices[mPolygons[i].mVertind[0]].z;
            if (tex)
            {
                mVertexArray[3*i + 0].u = mTextureCoordinates[mPolygons[i].mTexind[0]].x;
                mVertexArray[3*i + 0].v = mTextureCoordinates[mPolygons[i].mTexind[0]].y;
            }
            mVertexArray[3*i + 1].x = mVertices[mPolygons[i].mVertind[1]].x;
            mVertexArray[3*i + 1].y = mVertices[mPolygons[i].mVertind[1]].y;
            mVertexArray[3*i + 1].z = mVertices[mPolygons[i].mVertind[1]].z;
            if (tex)
            {
                mVertexArray[3*i + 1].u = mTextureCoordinates[mPolygons[i].mTexind[1]].x;
                mVertexArray[3*i + 1].v = mTextureCoordinates[mPolygons[i].mTexind[1]].y;
            }
            mVertexArray[3*i + 2].x = mVertices[mPolygons[i].mVertind[2]].x;
            mVertexArray[3*i + 2].y = mVertices[mPolygons[i].mVertind[2]].y;
            mVertexArray[3*i + 2].z = mVertices[mPolygons[i].mVertind[2]].z;
            if (tex)
            {
                mVertexArray[3*i + 2].u = mTextureCoordinates[mPolygons[i].mTexind[2]].x;
                mVertexArray[3*i + 2].v = mTextureCoordinates[mPolygons[i].mTexind[2]].y;
            }
        }
        
        mNumVertices = 3*mPolygons.size();
    }
    
    void StaticMesh::loadFromFile(chstr meshPath)
    {
        FILE *fp;
        char buffer[256];
        hstr command;
        
        std::cerr << "[aprilutil] loading mesh '" << meshPath << "'" << std::endl;
        
        fp=fopen(meshPath.c_str(), "rb");
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
                    mVertices.push_back(vert);
                    //std::cerr << "Vertice (" << msh->mVertices.size() << ")" << "[" << vert.x << "," << vert.y << "," << vert.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if(command == "vn")
            {
                gtypes::Vector3 nor;
                if( fscanf(fp, "%f %f %f", &nor.x, &nor.y, &nor.z) != EOF )
                {
                    mNormals.push_back(nor);
                    //std::cerr << "Normal (" << msh->mNormals.size() << ")" << "[" << nor.x << "," << nor.y << "," << nor.z << "]" << std::endl;
                }
                else throw faulty_obj_file(meshPath);
            }
            else if(command == "vt")
            {
                gtypes::Vector2 uv;
                if( fscanf(fp, "%f %f", &uv.x, &uv.y) != EOF )
                {
                    mTextureCoordinates.push_back(uv);
                    //std::cerr << "TexCoord (" << msh->mTextureCoordinates.size() << ")" << "[" << uv.x << "," << uv.y << "]" << std::endl;
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
                    poly.mVertind[0] = ((int)first - 1);
                    poly.mVertind[1] = ((int)second - 1);
                    poly.mVertind[2] = ((int)third - 1);
                }
                
                else if(first.count("//") == 1)
                {
                    hstr tmp1, tmp2;
                    first.split("//", tmp1, tmp2);
                    poly.mVertind[0] = ((int)tmp1 - 1);
                    poly.mNorind[0]  = ((int)tmp2 - 1);
                    
                    second.split("//", tmp1, tmp2);
                    poly.mVertind[1] = ((int)tmp1 - 1);
                    poly.mNorind[1]  = ((int)tmp2 - 1);
                    
                    third.split("//", tmp1, tmp2);
                    poly.mVertind[2] = ((int)tmp1 - 1);
                    poly.mNorind[2]  = ((int)tmp2 - 1);
                }
                
                else if(first.count("/") == 2)
                {
                    
                    hstr tmp1,tmp2,tmp3, tmp;
                    first.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.mVertind[0] = ((int)tmp1 - 1);
                    poly.mTexind[0]  = ((int)tmp2 - 1);
                    poly.mNorind[0]  = ((int)tmp3 - 1);
                    
                    second.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.mVertind[1] = ((int)tmp1 - 1);
                    poly.mTexind[1]  = ((int)tmp2 - 1);
                    poly.mNorind[1]  = ((int)tmp3 - 1);
                    
                    third.split('/', tmp1, tmp);
                    tmp.split('/', tmp2, tmp3);
                    
                    //std::cerr << tmp1 << " " << tmp2 << " " << tmp3 << std::endl;
                    
                    poly.mVertind[2] = ((int)tmp1 - 1);
                    poly.mTexind[2]  = ((int)tmp2 - 1);
                    poly.mNorind[2]  = ((int)tmp3 - 1);
                }
                
                mPolygons.push_back(poly);
                
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
    
    void StaticMesh::setMeshName(chstr meshName)
    {
        mMeshName = meshName;
    }
    
    void StaticMesh::draw(april::RenderOp renderOp)
    {
        rendersys->render(renderOp, mVertexArray, mNumVertices);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    _faulty_obj_file::_faulty_obj_file(chstr filename, const char* source_file, int line) :
	    exception("'" + filename + "' is faulty wavefront file!", source_file, line)
	{
	}
}
