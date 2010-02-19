/************************************************************************************
This source file is part of the Advanced Text Rendering System
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#include "april/RenderSystem.h"

April::Texture* tex;

bool render(float time_increase)
{
	rendersys->setViewport(800,600);
	rendersys->setTexture(tex);
	
	April::TexturedVertex v[4];
	
	v[0].x=0;   v[0].y=0;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=800; v[1].y=0;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=0;   v[2].y=600; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=800; v[3].y=600; v[3].z=0; v[3].u=1; v[3].v=1;
	
	rendersys->render(TRIANGLE_STRIP,v,4);
	return true;
}

int main()
{
	April::init("OpenGL",800,600,0,"demo_simple");
	rendersys->registerUpdateCallback(render);

	tex=rendersys->loadTexture("../media/texture.jpg");

	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
