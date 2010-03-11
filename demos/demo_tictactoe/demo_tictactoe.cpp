/************************************************************************************
This source file is part of the Awesome Portable Rendering Interface Library
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
#include <stdio.h>
#include "april/RenderSystem.h"

April::Texture* background;
April::Texture* x_symbol;
April::Texture* o_symbol;
int temp = 0;
bool player = 0;


void draw_symbol(int x, int y, std::string symbol)
{
	int x1, x2, x3, x4, y1, y2, y3, y4;
	
	x1 = x*250 - 250 + (x-1)*25;
	x2 = x*250 + (x-1)*25;
	x3 = x*250 - 250 + (x-1)*25;
	x4 = x*250 + (x-1)*25;
	y1 = y*185 - 185 + (y-1)*25;
	y2 = y*185 - 185 + (y-1)*25;
	y3 = y*185 + (y-1)*25;
	y4 = y*185 + (y-1)*25;
	
	/*switch (x)
	{
		case 1:
			x1 = 0;
			x2 = 250;
			x3 = 0;
			x4 = 250;
		break;
		
		case 2:
			x1 = 275;
			x2 = 525;
			x3 = 275;
			x4 = 525;
		break;
		
		case 3:
			x1 = 550;
			x2 = 800;
			x3 = 550;
			x4 = 800;
		break;
		
		default:
			printf("Wrong parameter 'x'!");
		break;
	}
	
	switch (y)
	{
		case 1:
			y1 = 0;
			y2 = 0;
			y3 = 185;
			y4 = 185;
		break;
		
		case 2:
			y1 = 210;
			y2 = 210;
			y3 = 390;
			y4 = 390;
		break;
		
		case 3:
			y1 = 415;
			y2 = 415;
			y3 = 600;
			y4 = 600;
		break;
		
		default:
			printf("Wrong parameter 'y'!");
		break;
	}*/
	
	if(symbol == "x_symbol")
		rendersys->setTexture(x_symbol);
	else if(symbol == "o_symbol")
		rendersys->setTexture(o_symbol);
	else
		printf("Wrong parameter 'symbol'!");

	
	April::TexturedVertex v[4];
	v[0].x=x1; v[0].y=y1; v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=x2; v[1].y=y2; v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=x3; v[2].y=y3; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=x4; v[3].y=y4; v[3].z=0; v[3].u=1; v[3].v=1;
	rendersys->render(April::TriangleStrip,v,4);
}

bool render(float time_increase)
{	
	rendersys->setViewport(800,600);
	
	rendersys->setTexture(background);
	April::TexturedVertex v[4];
	v[0].x=0;   v[0].y=0;   v[0].z=0; v[0].u=0; v[0].v=0;
	v[1].x=800; v[1].y=0;   v[1].z=0; v[1].u=1; v[1].v=0;
	v[2].x=0;   v[2].y=600; v[2].z=0; v[2].u=0; v[2].v=1;
	v[3].x=800; v[3].y=600; v[3].z=0; v[3].u=1; v[3].v=1;
	rendersys->render(April::TriangleStrip,v,4);
	
	rendersys->setTexture(0);
	rendersys->drawColoredQuad(250,0,25,600,1,1,0,1);
	rendersys->drawColoredQuad(525,0,25,600,1,1,0,1);
	rendersys->drawColoredQuad(0,185,800,25,1,1,0,1);
	rendersys->drawColoredQuad(0,390,800,25,1,1,0,1);
	
	
	if (temp == 110)
		draw_symbol(1,1,"o_symbol");
	if (temp == 120)
		draw_symbol(1,2,"o_symbol");
	if (temp == 130)
		draw_symbol(1,3,"o_symbol");
	if (temp == 210)
		draw_symbol(2,1,"o_symbol");
	if (temp == 220)
		draw_symbol(2,2,"o_symbol");
	if (temp == 230)
		draw_symbol(2,3,"o_symbol");
	if (temp == 310)
		draw_symbol(3,1,"o_symbol");
	if (temp == 320)
		draw_symbol(3,2,"o_symbol");
	if (temp == 330)
		draw_symbol(3,3,"o_symbol");
		
	if (temp == 111)
		draw_symbol(1,1,"x_symbol");
	if (temp == 121)
		draw_symbol(1,2,"x_symbol");
	if (temp == 131)
		draw_symbol(1,3,"x_symbol");
	if (temp == 211)
		draw_symbol(2,1,"x_symbol");
	if (temp == 221)
		draw_symbol(2,2,"x_symbol");
	if (temp == 231)
		draw_symbol(2,3,"x_symbol");
	if (temp == 311)
		draw_symbol(3,1,"x_symbol");
	if (temp == 321)
		draw_symbol(3,2,"x_symbol");
	if (temp == 331)
		draw_symbol(3,3,"x_symbol");
	
	return true;
}

void OnMouseUp(float x,float y,int button)
{
	if (x >= 0 && x <= 250 && y >= 0 && y <= 185 && !player)
	temp = 110;
	if (x >= 0 && x <= 250 && y >= 210 && y <= 390 && !player)
	temp = 120;
	if (x >= 0 && x <= 250 && y >= 415 && y <= 600 && !player)
	temp = 130;
	if (x >= 275 && x <= 525 && y >= 0 && y <= 185 && !player)
	temp = 210;
	if (x >= 275 && x <= 525 && y >= 210 && y <= 390 && !player)
	temp = 220;
	if (x >= 275 && x <= 525 && y >= 415 && y <= 600 && !player)
	temp = 230;
	if (x >= 550 && x <= 800 && y >= 0 && y <= 185 && !player)
	temp = 310;
	if (x >= 550 && x <= 800 && y >= 210 && y <= 390 && !player)
	temp = 320;
	if (x >= 550 && x <= 800 && y >= 415 && y <= 600 && !player)
	temp = 330;
	
	if (x >= 0 && x <= 250 && y >= 0 && y <= 185 && player)
	temp = 111;
	if (x >= 0 && x <= 250 && y >= 210 && y <= 390 && player)
	temp = 121;
	if (x >= 0 && x <= 250 && y >= 415 && y <= 600 && player)
	temp = 131;
	if (x >= 275 && x <= 525 && y >= 0 && y <= 185 && player)
	temp = 211;
	if (x >= 275 && x <= 525 && y >= 210 && y <= 390 && player)
	temp = 221;
	if (x >= 275 && x <= 525 && y >= 415 && y <= 600 && player)
	temp = 231;
	if (x >= 550 && x <= 800 && y >= 0 && y <= 185 && player)
	temp = 311;
	if (x >= 550 && x <= 800 && y >= 210 && y <= 390 && player)
	temp = 321;
	if (x >= 550 && x <= 800 && y >= 415 && y <= 600 && player)
	temp = 331;	
	
	player = !player;
}

int main()
{
	April::init("OpenGL",800,600,0,"demo_tictactoe");
	rendersys->registerUpdateCallback(render);
	rendersys->registerMouseCallbacks(0,OnMouseUp,0);
	background=rendersys->loadTexture("../media/texture.jpg");
	x_symbol=rendersys->loadTexture("../media/x.png");
	o_symbol=rendersys->loadTexture("../media/o.png");

	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
