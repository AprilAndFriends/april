/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include "april/RenderSystem.h"

April::Texture* background;
April::Texture* x_symbol;
April::Texture* o_symbol;
April::Texture* line_hor;
April::Texture* line_vert;
April::Texture* line45;
April::Texture* line315;
int positions[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
int victory = 0;
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


void draw_line(int x_start, int y_start, int x_end, int y_end, std::string symbol)
{
	int x1, x2, x3, x4, y1, y2, y3, y4;
	
	x1 = x_start*250 + (x_start+1)*25;
	x2 = x_end*250 + 250 + (x_start+1)*25;
	x3 = x_start*250 + (x_start+1)*25;
	x4 = x_end*250 + 250 + (x_start-1)*25;
	y1 = y_start*185 + (y_start+1)*25;
	y2 = y_start*185 + (y_start+1)*25;
	y3 = y_end*185 + 185 + (y_start+1)*25;
	y4 = y_end*185 + 185 + (y_start+1)*25;
	
	
	if(symbol == "line_hor")
		rendersys->setTexture(line_hor);
	else if(symbol == "line_vert")
		rendersys->setTexture(line_vert);
	else if(symbol == "line45")
		rendersys->setTexture(line45);
	else if(symbol == "line315")
		rendersys->setTexture(line315);
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
	
		
	if (positions[0][0] == 1)
		draw_symbol(1,1,"x_symbol");
	if (positions[0][1] == 1)
		draw_symbol(1,2,"x_symbol");
	if (positions[0][2] == 1)
		draw_symbol(1,3,"x_symbol");
	if (positions[1][0] == 1)
		draw_symbol(2,1,"x_symbol");
	if (positions[1][1] == 1)
		draw_symbol(2,2,"x_symbol");
	if (positions[1][2] == 1)
		draw_symbol(2,3,"x_symbol");
	if (positions[2][0] == 1)
		draw_symbol(3,1,"x_symbol");
	if (positions[2][1] == 1)
		draw_symbol(3,2,"x_symbol");
	if (positions[2][2] == 1)
		draw_symbol(3,3,"x_symbol");
		
	if (positions[0][0] == 2)
		draw_symbol(1,1,"o_symbol");
	if (positions[0][1] == 2)
		draw_symbol(1,2,"o_symbol");
	if (positions[0][2] == 2)
		draw_symbol(1,3,"o_symbol");
	if (positions[1][0] == 2)
		draw_symbol(2,1,"o_symbol");
	if (positions[1][1] == 2)
		draw_symbol(2,2,"o_symbol");
	if (positions[1][2] == 2)
		draw_symbol(2,3,"o_symbol");
	if (positions[2][0] == 2)
		draw_symbol(3,1,"o_symbol");
	if (positions[2][1] == 2)
		draw_symbol(3,2,"o_symbol");
	if (positions[2][2] == 2)
		draw_symbol(3,3,"o_symbol");
		
		
	if (victory == 1)
		draw_line(0,0,0,2,"line_hor");
	if (victory == 2)
		draw_line(1,0,1,2,"line_hor");
	if (victory == 3)
		draw_line(2,0,2,2,"line_hor");
	if (victory == 4)
		draw_line(0,0,2,0,"line_vert");
	if (victory == 5)
		draw_line(0,1,2,1,"line_vert");
	if (victory == 6)
		draw_line(0,2,2,2,"line_vert");
	if (victory == 7)
		draw_line(0,0,2,2,"line45");
	if (victory == 8)
		draw_line(0,0,2,2,"line315");
	
	return true;
}



void OnMouseUp(float x,float y,int button)
{
	if (x >= 0 && x <= 250 && y >= 0 && y <= 185 && !player && positions[0][0] == 0)
	{
		positions[0][0] = 1;
		player = !player;
	}
	if (x >= 0 && x <= 250 && y >= 210 && y <= 390 && !player && positions[0][1] == 0)
	{
		positions[0][1] = 1;
		player = !player;
	}
	if (x >= 0 && x <= 250 && y >= 415 && y <= 600 && !player && positions[0][2] == 0)
	{
		positions[0][2] = 1;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 0 && y <= 185 && !player && positions[1][0] == 0)
	{
		positions[1][0] = 1;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 210 && y <= 390 && !player && positions[1][1] == 0)
	{
		positions[1][1] = 1;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 415 && y <= 600 && !player && positions[1][2] == 0)
	{
		positions[1][2] = 1;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 0 && y <= 185 && !player && positions[2][0] == 0)
	{
		positions[2][0] = 1;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 210 && y <= 390 && !player && positions[2][1] == 0)
	{
		positions[2][1] = 1;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 415 && y <= 600 && !player && positions[2][2] == 0)
	{
		positions[2][2] = 1;
		player = !player;
	}
	
	
	if (x >= 0 && x <= 250 && y >= 0 && y <= 185 && player && positions[0][0] == 0)
	{
		positions[0][0] = 2;
		player = !player;
	}
	if (x >= 0 && x <= 250 && y >= 210 && y <= 390 && player && positions[0][1] == 0)
	{
		positions[0][1] = 2;
		player = !player;
	}
	if (x >= 0 && x <= 250 && y >= 415 && y <= 600 && player && positions[0][2] == 0)
	{
		positions[0][2] = 2;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 0 && y <= 185 && player && positions[1][0] == 0)
	{
		positions[1][0] = 2;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 210 && y <= 390 && player && positions[1][1] == 0)
	{
		positions[1][1] = 2;
		player = !player;
	}
	if (x >= 275 && x <= 525 && y >= 415 && y <= 600 && player && positions[1][2] == 0)
	{
		positions[1][2] = 2;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 0 && y <= 185 && player && positions[2][0] == 0)
	{
		positions[2][0] = 2;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 210 && y <= 390 && player && positions[2][1] == 0)
	{
		positions[2][1] = 2;
		player = !player;
	}
	if (x >= 550 && x <= 800 && y >= 415 && y <= 600 && player && positions[2][2] == 0)
	{
		positions[2][2] = 2;	
		player = !player;
	}
	
	
	if (positions[0][0] == 1 && positions[0][1] == 1 && positions[0][2] == 1)
		victory = 1;
	if (positions[1][0] == 1 && positions[1][1] == 1 && positions[1][2] == 1)
		victory = 2;
	if (positions[2][0] == 1 && positions[2][1] == 1 && positions[2][2] == 1)
		victory = 3;
	if (positions[0][0] == 1 && positions[1][0] == 1 && positions[2][0] == 1)
		victory = 4;
	if (positions[0][1] == 1 && positions[1][1] == 1 && positions[2][1] == 1)
		victory = 5;
	if (positions[0][2] == 1 && positions[1][2] == 1 && positions[2][2] == 1)
		victory = 6;
	if (positions[0][0] == 1 && positions[1][1] == 1 && positions[2][2] == 1)
		victory = 7;
	if (positions[0][2] == 1 && positions[1][1] == 1 && positions[2][0] == 1)
		victory = 8;
		
	if (positions[0][0] == 2 && positions[0][1] == 2 && positions[0][2] == 2)
		victory = 1;
	if (positions[1][0] == 2 && positions[1][1] == 2 && positions[1][2] == 2)
		victory = 2;
	if (positions[2][0] == 2 && positions[2][1] == 2 && positions[2][2] == 2)
		victory = 3;
	if (positions[0][0] == 2 && positions[1][0] == 2 && positions[2][0] == 2)
		victory = 4;
	if (positions[0][1] == 2 && positions[1][1] == 2 && positions[2][1] == 2)
		victory = 5;
	if (positions[0][2] == 2 && positions[1][2] == 2 && positions[2][2] == 2)
		victory = 6;
	if (positions[0][0] == 2 && positions[1][1] == 2 && positions[2][2] == 2)
		victory = 7;
	if (positions[0][2] == 2 && positions[1][1] == 2 && positions[2][0] == 2)
		victory = 8;
	
	
	/*
	if (positions[0][0] == 1 && positions[0][1] == 1 && positions[0][2] == 1 ||
	positions[1][0] == 1 && positions[1][1] == 1 && positions[1][2] == 1 ||
	positions[2][0] == 1 && positions[2][1] == 1 && positions[2][2] == 1 ||
	positions[0][0] == 1 && positions[1][0] == 1 && positions[2][0] == 1 ||
	positions[0][1] == 1 && positions[1][1] == 1 && positions[2][1] == 1 ||
	positions[0][2] == 1 && positions[1][2] == 1 && positions[2][2] == 1 ||
	positions[0][0] == 1 && positions[1][1] == 1 && positions[2][2] == 1 ||
	positions[0][2] == 1 && positions[1][1] == 1 && positions[2][0] == 1)	
	{
		positions[0][0] = 1;
		positions[0][1] = 1;
		positions[0][2] = 1;
		positions[1][0] = 1;
		positions[1][1] = 1;
		positions[1][2] = 1;
		positions[2][0] = 1;
		positions[2][1] = 1;
		positions[2][2] = 1;
	}
	
	if (positions[0][0] == 2 && positions[0][1] == 2 && positions[0][2] == 2 ||
	positions[1][0] == 2 && positions[1][1] == 2 && positions[1][2] == 2 ||
	positions[2][0] == 2 && positions[2][1] == 2 && positions[2][2] == 2 ||
	positions[0][0] == 2 && positions[1][0] == 2 && positions[2][0] == 2 ||
	positions[0][1] == 2 && positions[1][1] == 2 && positions[2][1] == 2 ||
	positions[0][2] == 2 && positions[1][2] == 2 && positions[2][2] == 2 ||
	positions[0][0] == 2 && positions[1][1] == 2 && positions[2][2] == 2 ||
	positions[0][2] == 2 && positions[1][1] == 2 && positions[2][0] == 2)
	{
		positions[0][0] = 2;
		positions[0][1] = 2;
		positions[0][2] = 2;
		positions[1][0] = 2;
		positions[1][1] = 2;
		positions[1][2] = 2;
		positions[2][0] = 2;
		positions[2][1] = 2;
		positions[2][2] = 2;
	}
	*/
}

int main()
{
	April::init("OpenGL",800,600,0,"demo_tictactoe");
	rendersys->registerUpdateCallback(render);
	rendersys->registerMouseCallbacks(0,OnMouseUp,0);
	background=rendersys->loadTexture("../media/texture.jpg");
	x_symbol=rendersys->loadTexture("../media/x.png");
	o_symbol=rendersys->loadTexture("../media/o.png");
	line_hor=rendersys->loadTexture("../media/line_hor.png");
	line_vert=rendersys->loadTexture("../media/line_vert.png");
	line45=rendersys->loadTexture("../media/line45.png");
	line315=rendersys->loadTexture("../media/line315.png");

	rendersys->enterMainLoop();
	April::destroy();
	return 0;
}
