/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "RenderHelperLayered2D.h"
#include "RenderSystem.h"

#ifdef _DEBUG
//#define _DEBUG_TESTING
#endif

namespace april
{
	// optimizations, but they are not thread-safe
	static PlainVertex pv[8];
	static TexturedVertex tv[6];

	RenderHelperLayered2D::RenderHelperLayered2D() : RenderHelper()
	{
	}

	RenderHelperLayered2D::~RenderHelperLayered2D()
	{
	}

	RenderHelperLayered2D::Layer::Layer(grect rect, RenderOperation renderOperation, const harray<ColoredVertex>& coloredVertices)
	{
		this->rects += rect;
		this->renderOperation = renderOperation;
		this->coloredVertices = coloredVertices;
		this->state.viewport = april::rendersys->state->viewport;
		this->state.useTexture = false;
		this->state.useColor = true;
		this->state.blendMode = april::rendersys->state->blendMode;
		this->state.colorMode = april::rendersys->state->colorMode;
		this->state.colorModeFactor = april::rendersys->state->colorModeFactor;
	}

	RenderHelperLayered2D::Layer::Layer(grect rect, RenderOperation renderOperation, const harray<ColoredTexturedVertex>& coloredTexturedVertices)
	{
		this->rects += rect;
		this->renderOperation = renderOperation;
		this->coloredTexturedVertices = coloredTexturedVertices;
		this->state.viewport = april::rendersys->state->viewport;
		this->state.useTexture = true;
		this->state.useColor = true;
		this->state.texture = april::rendersys->state->texture;
		this->state.blendMode = april::rendersys->state->blendMode;
		this->state.colorMode = april::rendersys->state->colorMode;
		this->state.colorModeFactor = april::rendersys->state->colorModeFactor;
	}

	RenderHelperLayered2D::Layer::~Layer()
	{
	}

	bool RenderHelperLayered2D::_tryForcedFlush(RenderOperation renderOperation)
	{
		if (renderOperation != RO_TRIANGLE_LIST && renderOperation != RO_LINE_LIST ||
			april::rendersys->state->depthBuffer || april::rendersys->state->depthBufferWrite)
		{
			this->flush();
			return true;
		}
		return false;
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, PlainVertex* vertices, int count)
	{
		return this->render(renderOperation, vertices, count, april::Color::White);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, PlainVertex* vertices, int count, Color color)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		harray<ColoredVertex> v(ColoredVertex(april::rendersys->getNativeColorUInt(color)), count);
		for_iter (i, 0, count)
		{
			v[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation, transformationMatrix, v);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, TexturedVertex* vertices, int count)
	{
		return this->render(renderOperation, vertices, count, april::Color::White);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, TexturedVertex* vertices, int count, Color color)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		harray<ColoredTexturedVertex> v(ColoredTexturedVertex(april::rendersys->getNativeColorUInt(color)), count);
		for_iter (i, 0, count)
		{
			v[i].set(transformationMatrix * vertices[i].toGvec3());
			v[i].u = vertices[i].u;
			v[i].v = vertices[i].v;
		}
		return this->_renderColoredTexturedVertices(renderOperation, transformationMatrix, v);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		harray<ColoredVertex> v(vertices, count);
		for_iter (i, 0, count)
		{
			v[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation, transformationMatrix, v);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		harray<ColoredTexturedVertex> v(vertices, count);
		for_iter (i, 0, count)
		{
			v[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredTexturedVertices(renderOperation, transformationMatrix, v);
	}

	bool RenderHelperLayered2D::drawRect(grect rect, Color color)
	{
		pv[0].x = pv[2].x = pv[4].x = pv[5].x = rect.x;
		pv[0].y = pv[1].y = pv[4].y = pv[6].y = rect.y;
		pv[1].x = pv[3].x = pv[6].x = pv[7].x = rect.x + rect.w;
		pv[2].y = pv[3].y = pv[5].y = pv[7].y = rect.y + rect.h;
		return this->render(RO_LINE_LIST, pv, 8, color);
	}

	bool RenderHelperLayered2D::drawFilledRect(grect rect, Color color)
	{
		pv[0].x = pv[2].x = pv[4].x = rect.x;
		pv[0].y = pv[1].y = pv[3].y = rect.y;
		pv[1].x = pv[3].x = pv[5].x = rect.x + rect.w;
		pv[2].y = pv[4].y = pv[5].y = rect.y + rect.h;
		return this->render(RO_TRIANGLE_LIST, pv, 6, color);
	}

	bool RenderHelperLayered2D::drawTexturedRect(grect rect, grect src)
	{
		tv[0].x = tv[2].x = tv[4].x = rect.x;
		tv[0].y = tv[1].y = tv[3].y = rect.y;
		tv[0].u = tv[2].u = tv[4].u = src.x;
		tv[0].v = tv[1].v = tv[3].v = src.y;
		tv[1].x = tv[3].x = tv[5].x = rect.x + rect.w;
		tv[2].y = tv[4].y = tv[5].y = rect.y + rect.h;
		tv[1].u = tv[3].u = tv[5].u = src.x + src.w;
		tv[2].v = tv[4].v = tv[5].v = src.y + src.h;
		return this->render(RO_TRIANGLE_LIST, tv, 6);
	}

	bool RenderHelperLayered2D::drawTexturedRect(grect rect, grect src, Color color)
	{
		tv[0].x = tv[2].x = tv[4].x = rect.x;
		tv[0].y = tv[1].y = tv[3].y = rect.y;
		tv[0].u = tv[2].u = tv[4].u = src.x;
		tv[0].v = tv[1].v = tv[3].v = src.y;
		tv[1].x = tv[3].x = tv[5].x = rect.x + rect.w;
		tv[2].y = tv[4].y = tv[5].y = rect.y + rect.h;
		tv[1].u = tv[3].u = tv[5].u = src.x + src.w;
		tv[2].v = tv[4].v = tv[5].v = src.y + src.h;
		return this->render(RO_TRIANGLE_LIST, tv, 6, color);
	}

	bool RenderHelperLayered2D::_renderColoredVertices(RenderOperation renderOperation, const gmat4& transformationMatrix, const harray<ColoredVertex>& vertices)
	{
		// find bounding rect
		gvec2 min(vertices[0].x, vertices[0].y);
		gvec2 max = min;
		for_iter (i, 1, vertices.size())
		{
			min.x = hmin(min.x, vertices[i].x);
			min.y = hmin(min.y, vertices[i].y);
			max.x = hmax(max.x, vertices[i].x);
			max.y = hmax(max.y, vertices[i].y);
		}
		grect boundingRect(min, max - min);
		if (april::rendersys->state->viewport.x != 0.0f || april::rendersys->state->viewport.y != 0.0f)
		{
			gvec3 offset = transformationMatrix * gvec3(april::rendersys->state->viewport.x, april::rendersys->state->viewport.y, 0.0f);
			boundingRect += gvec2(offset.x, offset.y);
		}
		// check existing rects
		bool intersection = false;
		Layer* validLayer = NULL;
		foreach_r (Layer*, it, this->layers)
		{
			if ((*it)->coloredVertices.size() > 0 && (*it)->renderOperation == renderOperation && (*it)->state.blendMode == april::rendersys->state->blendMode &&
				(*it)->state.colorMode == april::rendersys->state->colorMode && (*it)->state.colorModeFactor == april::rendersys->state->colorModeFactor &&
				(*it)->state.texture == april::rendersys->state->texture && (*it)->state.viewport == april::rendersys->state->viewport)
			{
				validLayer = (*it);
			}
			intersection = false;
			foreach (grect, it2, (*it)->rects)
			{
				if (boundingRect.intersects(*it2))
				{
					intersection = true;
					break;
				}
			}
			if (intersection)
			{
				break;
			}
		}
		if (validLayer == NULL)
		{
			this->layers += new Layer(boundingRect, renderOperation, vertices);
		}
		else
		{
			validLayer->coloredVertices += vertices;
			validLayer->rects += boundingRect;
		}
		return true;
	}

	bool RenderHelperLayered2D::_renderColoredTexturedVertices(RenderOperation renderOperation, const gmat4& transformationMatrix, const harray<ColoredTexturedVertex>& vertices)
	{
		// find bounding rect
		gvec2 min(vertices[0].x, vertices[0].y);
		gvec2 max = min;
		for_iter (i, 1, vertices.size())
		{
			min.x = hmin(min.x, vertices[i].x);
			min.y = hmin(min.y, vertices[i].y);
			max.x = hmax(max.x, vertices[i].x);
			max.y = hmax(max.y, vertices[i].y);
		}
		grect boundingRect(min, max - min);
		if (april::rendersys->state->viewport.x != 0.0f || april::rendersys->state->viewport.y != 0.0f)
		{
			gvec3 offset = transformationMatrix * gvec3(april::rendersys->state->viewport.x, april::rendersys->state->viewport.y, 0.0f);
			boundingRect += gvec2(offset.x, offset.y);
		}
		// check existing rects
		bool intersection = false;
		Layer* validLayer = NULL;
		foreach_r (Layer*, it, this->layers)
		{
			if ((*it)->coloredTexturedVertices.size() > 0 && (*it)->renderOperation == renderOperation && (*it)->state.blendMode == april::rendersys->state->blendMode &&
				(*it)->state.colorMode == april::rendersys->state->colorMode && (*it)->state.colorModeFactor == april::rendersys->state->colorModeFactor &&
				(*it)->state.texture == april::rendersys->state->texture && (*it)->state.viewport == april::rendersys->state->viewport)
			{
				validLayer = (*it);
			}
			intersection = false;
			foreach (grect, it2, (*it)->rects)
			{
				if (boundingRect.intersects(*it2))
				{
					intersection = true;
					break;
				}
			}
			if (intersection)
			{
				break;
			}
		}
		if (validLayer == NULL)
		{
			this->layers += new Layer(boundingRect, renderOperation, vertices);
		}
		else
		{
			validLayer->coloredTexturedVertices += vertices;
			validLayer->rects += boundingRect;
		}
		return true;
	}

	void RenderHelperLayered2D::flush()
	{
		if (this->layers.size() == 0)
		{
			return;
		}
#ifdef _DEBUG_TESTING
		hlog::debug("OK", this->layers.size());
#endif
		this->layers.last()->state.modelviewMatrixChanged = true;
		this->layers.last()->state.projectionMatrixChanged = true;
		RenderState* state = april::rendersys->state;
		bool anyViewportChanged = false;
#ifdef _DEBUG_TESTING
		int count = 15;
#endif
		foreach (Layer*, it, this->layers)
		{
#ifdef _DEBUG_TESTING
			if (count <= 0)
			{
				delete (*it);
				continue;
			}
			--count;
#endif
			april::rendersys->state = &(*it)->state;
			if (april::rendersys->state->viewport != april::rendersys->deviceState->viewport)
			{
				anyViewportChanged = april::rendersys->state->viewportChanged = true;
			}
			if ((*it)->coloredVertices.size() > 0)
			{
				april::rendersys->_renderInternal((*it)->renderOperation, (april::ColoredVertex*)(*it)->coloredVertices, (*it)->coloredVertices.size());
			}
			else if ((*it)->coloredTexturedVertices.size() > 0)
			{
				april::rendersys->_renderInternal((*it)->renderOperation, (april::ColoredTexturedVertex*)(*it)->coloredTexturedVertices, (*it)->coloredTexturedVertices.size());
			}
#ifdef _DEBUG_TESTING
			foreach (grect, it2, (*it)->rects)
			{
				april::rendersys->_drawRectInternal((*it2), april::Color::Cyan);
			}
#endif
			delete (*it);
		}
		this->layers.clear();
		april::rendersys->state = state;
		if (anyViewportChanged)
		{
			april::rendersys->state->viewportChanged = true;
		}
		april::rendersys->state->modelviewMatrixChanged = true;
		april::rendersys->state->projectionMatrixChanged = true;
	}

	void RenderHelperLayered2D::clear()
	{
		foreach (Layer*, it, this->layers)
		{
			delete (*it);
		}
		this->layers.clear();
	}

}
