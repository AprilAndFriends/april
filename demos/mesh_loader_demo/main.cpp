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
#include "april/Window.h"
#include "april/Timer.h"
#include "aprilutil/StaticMesh.h"

#include <iostream>

april::Texture* tex;
april::StaticMesh *mMesh;

bool render(float time_increase)
{
	april::rendersys->clear(true, true);
	static float angle=0;
	angle+=time_increase*90;
    
	april::rendersys->setPerspective(60,800/600.,0.1f,100.0f);
	april::rendersys->setTexture(tex);
	
	april::rendersys->lookAt(gtypes::Vector3(2,2,-5),gtypes::Vector3(0,0,0),gtypes::Vector3(0,1,0));
	april::rendersys->rotate(angle,0,1,0);
	
    mMesh->draw(april::TriangleList);
    
	return true;
}

int main()
{
	april::init("april",800,600,0,"april: 3D Demo");
	april::rendersys->getWindow()->setUpdateCallback(render);

    mMesh = new april::StaticMesh("../media/testobject.obj");
	tex = april::rendersys->loadTexture("../media/texture.jpg");
    
	april::rendersys->getWindow()->enterMainLoop();
    
    delete mMesh;
    
	april::destroy();
	return 0;
}
