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

	RenderHelperLayered2D::RenderHelperLayered2D() : RenderHelper(), _coloredVertices(NULL), _coloredVerticesCount(0),
		_coloredVerticesCapacity(0), _coloredTexturedVertices(NULL), _coloredTexturedVerticesCount(0), _coloredTexturedVerticesCapacity(0)
	{
	}

	RenderHelperLayered2D::~RenderHelperLayered2D()
	{
		if (this->_coloredVertices != NULL)
		{
			delete[] this->_coloredVertices;
		}
		if (this->_coloredTexturedVertices != NULL)
		{
			delete[] this->_coloredTexturedVertices;
		}
	}

	RenderHelperLayered2D::Layer::Layer(grect rect, RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		this->rects += rect;
		this->renderOperation = renderOperation;
		this->coloredVertices.add(vertices, count);
		this->state.viewport = april::rendersys->state->viewport;
		this->state.useTexture = false;
		this->state.useColor = true;
		this->state.blendMode = april::rendersys->state->blendMode;
		this->state.colorMode = april::rendersys->state->colorMode;
		this->state.colorModeFactor = april::rendersys->state->colorModeFactor;
	}

	RenderHelperLayered2D::Layer::Layer(grect rect, RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		this->rects += rect;
		this->renderOperation = renderOperation;
		this->coloredTexturedVertices.add(vertices, count);
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

	void RenderHelperLayered2D::_updateVertices(PlainVertex* vertices, int count, april::Color color)
	{
		this->_updateColoredVerticesSize(count);
		unsigned int nativeColor = april::rendersys->getNativeColorUInt(color);
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].color = nativeColor;
		}
	}

	void RenderHelperLayered2D::_updateVertices(TexturedVertex* vertices, int count, april::Color color)
	{
		this->_updateColoredTexturedVerticesSize(count);
		unsigned int nativeColor = april::rendersys->getNativeColorUInt(color);
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].u = vertices[i].u;
			this->_coloredTexturedVertices[i].v = vertices[i].v;
			this->_coloredTexturedVertices[i].color = nativeColor;
		}
	}

	void RenderHelperLayered2D::_updateVertices(ColoredVertex* vertices, int count)
	{
		this->_updateColoredVerticesSize(count);
		memcpy(this->_coloredVertices, vertices, count * sizeof(ColoredVertex));
	}

	void RenderHelperLayered2D::_updateVertices(ColoredTexturedVertex* vertices, int count)
	{
		this->_updateColoredTexturedVerticesSize(count);
		memcpy(this->_coloredTexturedVertices, vertices, count * sizeof(ColoredTexturedVertex));
	}

	void RenderHelperLayered2D::_updateColoredVerticesSize(int count)
	{
		this->_coloredVerticesCount = count;
		int potCount = hpotCeil(count);
		if (this->_coloredVertices == NULL)
		{
			this->_coloredVerticesCapacity = potCount;
			this->_coloredVertices = new ColoredVertex[this->_coloredVerticesCapacity];
		}
		else if (this->_coloredVerticesCapacity < potCount)
		{
			delete[] this->_coloredVertices;
			this->_coloredVerticesCapacity = potCount;
			this->_coloredVertices = new ColoredVertex[this->_coloredVerticesCapacity];
		}
	}

	void RenderHelperLayered2D::_updateColoredTexturedVerticesSize(int count)
	{
		this->_coloredTexturedVerticesCount = count;
		int potCount = hpotCeil(count);
		if (this->_coloredTexturedVertices == NULL)
		{
			this->_coloredTexturedVerticesCapacity = potCount;
			this->_coloredTexturedVertices = new ColoredTexturedVertex[this->_coloredTexturedVerticesCapacity];
		}
		else if (this->_coloredTexturedVerticesCapacity < potCount)
		{
			delete[] this->_coloredTexturedVertices;
			this->_coloredTexturedVerticesCapacity = potCount;
			this->_coloredTexturedVertices = new ColoredTexturedVertex[this->_coloredTexturedVerticesCapacity];
		}
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
		this->_updateVertices(vertices, count, color);
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation, transformationMatrix);
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
		this->_updateVertices(vertices, count, color);
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredTexturedVertices(renderOperation, transformationMatrix);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		this->_updateVertices(vertices, count);
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation, transformationMatrix);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		this->_updateVertices(vertices, count);
		// transform all positions
		gmat4 transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredTexturedVertices(renderOperation, transformationMatrix);
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

	bool RenderHelperLayered2D::_renderColoredVertices(RenderOperation renderOperation, const gmat4& transformationMatrix)
	{
		// find bounding rect
		gvec2 min(this->_coloredVertices[0].x, this->_coloredVertices[0].y);
		gvec2 max = min;
		for_iter (i, 1, this->_coloredVerticesCount)
		{
			min.x = hmin(min.x, this->_coloredVertices[i].x);
			min.y = hmin(min.y, this->_coloredVertices[i].y);
			max.x = hmax(max.x, this->_coloredVertices[i].x);
			max.y = hmax(max.y, this->_coloredVertices[i].y);
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
			this->layers += new Layer(boundingRect, renderOperation, this->_coloredVertices, this->_coloredVerticesCount);
		}
		else
		{
			validLayer->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
			validLayer->rects += boundingRect;
		}
		return true;
	}

	bool RenderHelperLayered2D::_renderColoredTexturedVertices(RenderOperation renderOperation, const gmat4& transformationMatrix)
	{
		// find bounding rect
		gvec2 min(this->_coloredTexturedVertices[0].x, this->_coloredTexturedVertices[0].y);
		gvec2 max = min;
		for_iter (i, 1, this->_coloredTexturedVerticesCount)
		{
			min.x = hmin(min.x, this->_coloredTexturedVertices[i].x);
			min.y = hmin(min.y, this->_coloredTexturedVertices[i].y);
			max.x = hmax(max.x, this->_coloredTexturedVertices[i].x);
			max.y = hmax(max.y, this->_coloredTexturedVertices[i].y);
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
			this->layers += new Layer(boundingRect, renderOperation, this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
		}
		else
		{
			validLayer->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
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
		int count = 150;
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
