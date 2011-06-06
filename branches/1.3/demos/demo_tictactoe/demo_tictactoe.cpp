/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>

#include <april/RenderSystem.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hstring.h>

april::Texture* background;
april::Texture* x_symbol;
april::Texture* o_symbol;
april::Texture* line_horz;
april::Texture* line_vert;
april::Texture* line45;
april::Texture* line315;
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
int positions[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
int victory = 0;
bool player = 0;
april::TexturedVertex v[4];

void draw_symbol(int x, int y, chstr symbol)
{
	int x1, x2, x3, x4, y1, y2, y3, y4;
	
	x1 = x * 250 - 250 + (x - 1) * 25;
	x2 = x * 250 + (x - 1) * 25;
	x3 = x * 250 - 250 + (x - 1) * 25;
	x4 = x * 250 + (x - 1) * 25;
	y1 = y * 185 - 185 + (y - 1) * 25;
	y2 = y * 185 - 185 + (y - 1) * 25;
	y3 = y * 185 + (y - 1) * 25;
	y4 = y * 185 + (y - 1) * 25;
	
	if (symbol == "x_symbol")
	{
		april::rendersys->setTexture(x_symbol);
	}
	else if (symbol == "o_symbol")
	{
		april::rendersys->setTexture(o_symbol);
	}
	else
	{
		printf("Wrong parameter 'symbol'!");
	}
	
	v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
	v[1].x = x2; v[1].y = y2; v[1].z = 0; v[1].u = 1; v[1].v = 0;
	v[2].x = x3; v[2].y = y3; v[2].z = 0; v[2].u = 0; v[2].v = 1;
	v[3].x = x4; v[3].y = y4; v[3].z = 0; v[3].u = 1; v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
}


void draw_line(int x_start, int y_start, int x_end, int y_end, std::string symbol)
{
	int x1, x2, x3, x4, y1, y2, y3, y4;
	
	x1 = x_start * 250 + (x_start + 1) * 25;
	x2 = x_end * 250 + 250 + (x_start - 1) * 25;
	x3 = x_start * 250 + (x_start + 1) * 25;
	x4 = x_end * 250 + 250 + (x_start - 1) * 25;
	y1 = y_start * 185 + (y_start + 1) * 25;
	y2 = y_start * 185 + (y_start + 1) * 25;
	y3 = y_end * 185 + 185 + (y_start - 1) * 25;
	y4 = y_end * 185 + 185 + (y_start - 1) * 25;
	
	if (symbol == "line_horz")
	{
		april::rendersys->setTexture(line_horz);
	}
	else if (symbol == "line_vert")
	{
		april::rendersys->setTexture(line_vert);
	}
	else if (symbol == "line45")
	{
		april::rendersys->setTexture(line45);
	}
	else if (symbol == "line315")
	{
		april::rendersys->setTexture(line315);
	}
	else
	{
		printf("Wrong parameter 'symbol'!");
	}
	
	v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
	v[1].x = x2; v[1].y = y2; v[1].z = 0; v[1].u = 1; v[1].v = 0;
	v[2].x = x3; v[2].y = y3; v[2].z = 0; v[2].u = 0; v[2].v = 1;
	v[3].x = x4; v[3].y = y4; v[3].z = 0; v[3].u = 1; v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
}

bool update(float k)
{	
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	
	april::rendersys->setTexture(background);
	v[0].x = 0;   v[0].y = 0;   v[0].z = 0; v[0].u = 0; v[0].v = 0;
	v[1].x = 800; v[1].y = 0;   v[1].z = 0; v[1].u = 1; v[1].v = 0;
	v[2].x = 0;   v[2].y = 600; v[2].z = 0; v[2].u = 0; v[2].v = 1;
	v[3].x = 800; v[3].y = 600; v[3].z = 0; v[3].u = 1; v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
	
	april::rendersys->setTexture(0);
	april::rendersys->drawColoredQuad(grect(250, 0, 25, 600), april::Color::MANGENTA);
	april::rendersys->drawColoredQuad(grect(525, 0, 25, 600), april::Color::MANGENTA);
	april::rendersys->drawColoredQuad(grect(0, 185, 800, 25), april::Color::MANGENTA);
	april::rendersys->drawColoredQuad(grect(0, 390, 800, 25), april::Color::MANGENTA);
	
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			if (positions[i][j] == 1)
			{
				draw_symbol(i + 1, j + 1, "x_symbol");
			}
			else if (positions[i][j] == 2)
			{
				draw_symbol(i + 1, j + 1, "o_symbol");
			}
		}
	}
		
	switch (victory)
	{
	case 1:
		draw_line(0, 0, 0, 2, "line_horz");
		break;
	case 2:
		draw_line(1, 0, 1, 2, "line_horz");
		break;
	case 3:
		draw_line(2, 0, 2, 2, "line_horz");
		break;
	case 4:
		draw_line(0, 0, 2, 0, "line_vert");
		break;
	case 5:
		draw_line(0, 1, 2, 1, "line_vert");
		break;
	case 6:
		draw_line(0, 2, 2, 2, "line_vert");
		break;
	case 7:
		draw_line(0, 0, 2, 2, "line45");
		break;
	case 8:
		draw_line(0, 0, 2, 2, "line315");
		break;
	}
	
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
	april::init("renderer", drawRect.w, drawRect.h, false, "demo_tictactoe");
	april::rendersys->getWindow()->setUpdateCallback(update);
	april::rendersys->getWindow()->setMouseCallbacks(NULL, OnMouseUp, NULL);
	background = april::rendersys->loadTexture("../media/texture.jpg");
	x_symbol = april::rendersys->loadTexture("../media/x.png");
	o_symbol = april::rendersys->loadTexture("../media/o.png");
	line_horz = april::rendersys->loadTexture("../media/line_horz.png");
	line_vert = april::rendersys->loadTexture("../media/line_vert.png");
	line45 = april::rendersys->loadTexture("../media/line45.png");
	line315 = april::rendersys->loadTexture("../media/line315.png");
	april::rendersys->getWindow()->enterMainLoop();
	april::destroy();
	return 0;
}
