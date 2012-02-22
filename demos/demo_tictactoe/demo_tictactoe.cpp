/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#define RESOURCE_PATH "./"
#else
#define RESOURCE_PATH "../media/"
#endif

#include <stdio.h>

#include <april/main.h>
#include <april/RenderSystem.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <hltypes/hstring.h>

april::Texture* background = NULL;
april::Texture* x_symbol = NULL;
april::Texture* o_symbol = NULL;
april::Texture* line_horz = NULL;
april::Texture* line_vert = NULL;
april::Texture* line45 = NULL;
april::Texture* line315 = NULL;
int positions[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
int victory = 0;
bool player = 0;
april::TexturedVertex v[4];

#ifndef _ANDROID
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 size = drawRect.getSize() * 5 / 16;

void draw_symbol(int x, int y, chstr symbol)
{
	float x1, x2, x3, x4, y1, y2, y3, y4;
	
	x1 = x * size.x - size.x + (x - 1) * 10;
	x2 = x * size.x + (x - 1) * 10;
	x3 = x * size.x - size.x + (x - 1) * 10;
	x4 = x * size.x + (x - 1) * 10;
	y1 = y * size.y - size.y + (y - 1) * 10;
	y2 = y * size.y - size.y + (y - 1) * 10;
	y3 = y * size.y + (y - 1) * 10;
	y4 = y * size.y + (y - 1) * 10;
	
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
		printf("Wrong parameter 'symbol'!\n");
	}
	
	v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
	v[1].x = x2; v[1].y = y2; v[1].z = 0; v[1].u = 1; v[1].v = 0;
	v[2].x = x3; v[2].y = y3; v[2].z = 0; v[2].u = 0; v[2].v = 1;
	v[3].x = x4; v[3].y = y4; v[3].z = 0; v[3].u = 1; v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
}


void draw_line(int x_start, int y_start, int x_end, int y_end, std::string symbol)
{
	float x1 = x_start * (size.x + 10) + 10;
	float x2 = (x_end + 1) * (size.x + 10) + 10;
	float y1 = y_start * (size.y + 10) + 10;
	float y2 = (y_end + 1) * (size.y + 10);
	
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
		printf("Wrong parameter 'symbol'!\n");
	}
	
	v[0].x = x1; v[0].y = y1; v[0].z = 0; v[0].u = 0; v[0].v = 0;
	v[1].x = x2; v[1].y = y1; v[1].z = 0; v[1].u = 1; v[1].v = 0;
	v[2].x = x1; v[2].y = y2; v[2].z = 0; v[2].u = 0; v[2].v = 1;
	v[3].x = x2; v[3].y = y2; v[3].z = 0; v[3].u = 1; v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
}

bool update(float k)
{	
	april::rendersys->clear();
	april::rendersys->setOrthoProjection(drawRect);
	
	april::rendersys->setTexture(background);
	v[0].x = 0;				v[0].y = 0;				v[0].z = 0;	v[0].u = 0;	v[0].v = 0;
	v[1].x = drawRect.w;	v[1].y = 0;				v[1].z = 0;	v[1].u = 1;	v[1].v = 0;
	v[2].x = 0;				v[2].y = drawRect.h;	v[2].z = 0;	v[2].u = 0;	v[2].v = 1;
	v[3].x = drawRect.w;	v[3].y = drawRect.h;	v[3].z = 0;	v[3].u = 1;	v[3].v = 1;
	april::rendersys->render(april::TriangleStrip, v, 4);
	
	april::rendersys->setTexture(NULL);
	april::rendersys->drawColoredQuad(grect(size.x, 0, 10, drawRect.h), APRIL_COLOR_MANGENTA);
	april::rendersys->drawColoredQuad(grect(size.x * 2 + 10, 0, 10, drawRect.h), APRIL_COLOR_MANGENTA);
	april::rendersys->drawColoredQuad(grect(0, size.y, drawRect.w, 10), APRIL_COLOR_MANGENTA);
	april::rendersys->drawColoredQuad(grect(0, size.y * 2 + 10, drawRect.w, 10), APRIL_COLOR_MANGENTA);
	
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

void OnMouseUp(float x, float y, int button)
{
	if (!player)
	{
		if (x >= 0 && x <= size.x && y >= 0 && y <= size.y && positions[0][0] == 0)
		{
			positions[0][0] = 1;
			player = !player;
		}
		if (x >= 0 && x <= size.x && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[0][1] == 0)
		{
			positions[0][1] = 1;
			player = !player;
		}
		if (x >= 0 && x <= size.x && y >= size.y * 2 + 20 && y <= drawRect.h && positions[0][2] == 0)
		{
			positions[0][2] = 1;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= 0 && y <= size.y && positions[1][0] == 0)
		{
			positions[1][0] = 1;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[1][1] == 0)
		{
			positions[1][1] = 1;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= size.y * 2 + 20 && y <= drawRect.h && positions[1][2] == 0)
		{
			positions[1][2] = 1;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= 0 && y <= size.y && positions[2][0] == 0)
		{
			positions[2][0] = 1;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[2][1] == 0)
		{
			positions[2][1] = 1;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= size.y * 2 + 20 && y <= drawRect.h && positions[2][2] == 0)
		{
			positions[2][2] = 1;
			player = !player;
		}
	}
	else
	{
		if (x >= 0 && x <= size.x && y >= 0 && y <= size.y && positions[0][0] == 0)
		{
			positions[0][0] = 2;
			player = !player;
		}
		if (x >= 0 && x <= size.x && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[0][1] == 0)
		{
			positions[0][1] = 2;
			player = !player;
		}
		if (x >= 0 && x <= size.x && y >= size.y * 2 + 20 && y <= drawRect.h && positions[0][2] == 0)
		{
			positions[0][2] = 2;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= 0 && y <= size.y && positions[1][0] == 0)
		{
			positions[1][0] = 2;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[1][1] == 0)
		{
			positions[1][1] = 2;
			player = !player;
		}
		if (x >= size.x + 10 && x <= size.x * 2 + 10 && y >= size.y * 2 + 20 && y <= drawRect.h && positions[1][2] == 0)
		{
			positions[1][2] = 2;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= 0 && y <= size.y && positions[2][0] == 0)
		{
			positions[2][0] = 2;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= size.y + 10 && y <= size.y * 2 + 10 && positions[2][1] == 0)
		{
			positions[2][1] = 2;
			player = !player;
		}
		if (x >= size.x * 2 + 20 && x <= drawRect.w && y >= size.y * 2 + 20 && y <= drawRect.h && positions[2][2] == 0)
		{
			positions[2][2] = 2;	
			player = !player;
		}
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
	
}

void april_init(const harray<hstr>& args)
{
	april::init();
	april::createRenderSystem("");
	april::createRenderTarget((int)drawRect.w, (int)drawRect.h, false, "Demo Tic Tac Toe");
	april::rendersys->getWindow()->setUpdateCallback(update);
	april::rendersys->getWindow()->setMouseCallbacks(NULL, OnMouseUp, NULL);
	background = april::rendersys->loadTexture(RESOURCE_PATH "texture");
	x_symbol = april::rendersys->loadTexture(RESOURCE_PATH "x");
	o_symbol = april::rendersys->loadTexture(RESOURCE_PATH "o");
	line_horz = april::rendersys->loadTexture(RESOURCE_PATH "line_horz");
	line_vert = april::rendersys->loadTexture(RESOURCE_PATH "line_vert");
	line45 = april::rendersys->loadTexture(RESOURCE_PATH "line45");
	line315 = april::rendersys->loadTexture(RESOURCE_PATH "line315");
}

void april_destroy()
{
	delete background;
	delete x_symbol;
	delete o_symbol;
	delete line_horz;
	delete line_vert;
	delete line45;
	delete line315;
	april::destroy();
}
