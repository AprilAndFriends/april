/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
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
	rendersys->clear(true, true);
	static float angle=0;
	angle+=time_increase*90;
    
	rendersys->setPerspective(60,800/600.,0.1f,100.0f);
	rendersys->setTexture(tex);
	
	rendersys->lookAt(gtypes::Vector3(2,2,-5),gtypes::Vector3(0,0,0),gtypes::Vector3(0,1,0));
	rendersys->rotate(angle,0,1,0);
	
    mMesh->draw(April::TriangleList);
    
	return true;
}

int main()
{
	April::init("April",800,600,0,"April: 3D Demo");
	rendersys->registerUpdateCallback(render);

    mMesh = new April::StaticMesh("../media/testobject.obj");
	tex = rendersys->loadTexture("../media/texture.jpg");
    
	rendersys->enterMainLoop();
    
    delete mMesh;
    
	April::destroy();
	return 0;
}
