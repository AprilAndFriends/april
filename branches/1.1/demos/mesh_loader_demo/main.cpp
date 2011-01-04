/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "april/RenderSystem.h"
#include "april/Timer.h"
#include "aprilutil/StaticMesh.h"

#include <iostream>

April::Texture* tex;
April::StaticMesh *mMesh;

bool render(float time_increase)
{
	April::rendersys->clear(true, true);
	static float angle=0;
	angle+=time_increase*90;
    
	April::rendersys->setPerspective(60,800/600.,0.1f,100.0f);
	April::rendersys->setTexture(tex);
	
	April::rendersys->lookAt(gtypes::Vector3(2,2,-5),gtypes::Vector3(0,0,0),gtypes::Vector3(0,1,0));
	April::rendersys->rotate(angle,0,1,0);
	
    mMesh->draw(April::TriangleList);
    
	return true;
}

int main()
{
	April::init("April",800,600,0,"April: 3D Demo");
	April::rendersys->registerUpdateCallback(render);

    mMesh = new April::StaticMesh("../media/testobject.obj");
	tex = April::rendersys->loadTexture("../media/texture.jpg");
    
	April::rendersys->enterMainLoop();
    
    delete mMesh;
    
	April::destroy();
	return 0;
}
