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
	static gvec3 corners[4];

	RenderHelperLayered2D::RenderHelperLayered2D() : RenderHelper(), _coloredVertices(NULL), _coloredVerticesCount(0),
		_coloredVerticesCapacity(0), _coloredTexturedVertices(NULL), _coloredTexturedVerticesCount(0), _coloredTexturedVerticesCapacity(0),
		_nativeColor(0), _potCount(0)
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
		this->_nativeColor = april::rendersys->getNativeColorUInt(color);
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].color = this->_nativeColor;
		}
	}

	void RenderHelperLayered2D::_updateVertices(TexturedVertex* vertices, int count, april::Color color)
	{
		this->_updateColoredTexturedVerticesSize(count);
		this->_nativeColor = april::rendersys->getNativeColorUInt(color);
		if (vertices != NULL)
		{
			for_iter (i, 0, count)
			{
				this->_coloredTexturedVertices[i].u = vertices[i].u;
				this->_coloredTexturedVertices[i].v = vertices[i].v;
				this->_coloredTexturedVertices[i].color = this->_nativeColor;
			}
		}
		else
		{
			for_iter (i, 0, count)
			{
				this->_coloredTexturedVertices[i].color = this->_nativeColor;
			}
		}
	}

	void RenderHelperLayered2D::_updateVertices(ColoredVertex* vertices, int count)
	{
		this->_updateColoredVerticesSize(count);
		if (vertices != NULL)
		{
			memcpy(this->_coloredVertices, vertices, count * sizeof(ColoredVertex));
		}
	}

	void RenderHelperLayered2D::_updateVertices(ColoredTexturedVertex* vertices, int count)
	{
		this->_updateColoredTexturedVerticesSize(count);
		if (vertices != NULL)
		{
			memcpy(this->_coloredTexturedVertices, vertices, count * sizeof(ColoredTexturedVertex));
		}
	}

	void RenderHelperLayered2D::_updateColoredVerticesSize(int count)
	{
		this->_coloredVerticesCount = count;
		this->_potCount = hpotCeil(count);
		if (this->_coloredVertices == NULL)
		{
			this->_coloredVerticesCapacity = this->_potCount;
			this->_coloredVertices = new ColoredVertex[this->_coloredVerticesCapacity];
		}
		else if (this->_coloredVerticesCapacity < this->_potCount)
		{
			delete[] this->_coloredVertices;
			this->_coloredVerticesCapacity = this->_potCount;
			this->_coloredVertices = new ColoredVertex[this->_coloredVerticesCapacity];
		}
	}

	void RenderHelperLayered2D::_updateColoredTexturedVerticesSize(int count)
	{
		this->_coloredTexturedVerticesCount = count;
		this->_potCount = hpotCeil(count);
		if (this->_coloredTexturedVertices == NULL)
		{
			this->_coloredTexturedVerticesCapacity = this->_potCount;
			this->_coloredTexturedVertices = new ColoredTexturedVertex[this->_coloredTexturedVerticesCapacity];
		}
		else if (this->_coloredTexturedVerticesCapacity < this->_potCount)
		{
			delete[] this->_coloredTexturedVertices;
			this->_coloredTexturedVerticesCapacity = this->_potCount;
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
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation);
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
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredTexturedVertices(renderOperation);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		this->_updateVertices(vertices, count);
		// transform all positions
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredVertices(renderOperation);
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		this->_updateVertices(vertices, count);
		// transform all positions
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
		}
		return this->_renderColoredTexturedVertices(renderOperation);
	}

	bool RenderHelperLayered2D::drawRect(grect rect, Color color)
	{
		this->_updateVertices((PlainVertex*)NULL, 8, color);
		corners[0].x = corners[2].x = rect.x;
		corners[0].y = corners[1].y = rect.y;
		corners[1].x = corners[3].x = rect.x + rect.w;
		corners[2].y = corners[3].y = rect.y + rect.h;
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, 4)
		{
			corners[i] = this->_transformationMatrix * corners[i];
		}
		this->_coloredVertices[0].set(corners[0]);
		this->_coloredVertices[1].set(corners[1]);
		this->_coloredVertices[2].set(corners[2]);
		this->_coloredVertices[3].set(corners[3]);
		this->_coloredVertices[4].set(corners[0]);
		this->_coloredVertices[5].set(corners[2]);
		this->_coloredVertices[6].set(corners[1]);
		this->_coloredVertices[7].set(corners[3]);
		return this->_renderColoredVertices(RO_LINE_LIST);
	}

	bool RenderHelperLayered2D::drawFilledRect(grect rect, Color color)
	{
		this->_updateVertices((PlainVertex*)NULL, 6, color);
		corners[0].x = corners[2].x = rect.x;
		corners[0].y = corners[1].y = rect.y;
		corners[1].x = corners[3].x = rect.x + rect.w;
		corners[2].y = corners[3].y = rect.y + rect.h;
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, 4)
		{
			corners[i] = this->_transformationMatrix * corners[i];
		}
		this->_coloredVertices[0].set(corners[0]);
		this->_coloredVertices[1].set(corners[1]);
		this->_coloredVertices[2].set(corners[2]);
		this->_coloredVertices[3].set(corners[1]);
		this->_coloredVertices[4].set(corners[2]);
		this->_coloredVertices[5].set(corners[3]);
		return this->_renderColoredVertices(RO_TRIANGLE_LIST);
	}

	bool RenderHelperLayered2D::drawTexturedRect(grect rect, grect src)
	{
		return this->drawTexturedRect(rect, src, april::Color::White);
	}

	bool RenderHelperLayered2D::drawTexturedRect(grect rect, grect src, Color color)
	{
		this->_updateVertices((TexturedVertex*)NULL, 6, color);
		corners[0].x = corners[2].x = rect.x;
		corners[0].y = corners[1].y = rect.y;
		corners[1].x = corners[3].x = rect.x + rect.w;
		corners[2].y = corners[3].y = rect.y + rect.h;
		this->_transformationMatrix = april::rendersys->state->projectionMatrix * april::rendersys->state->modelviewMatrix;
		for_iter (i, 0, 4)
		{
			corners[i] = this->_transformationMatrix * corners[i];
		}
		this->_coloredTexturedVertices[0].set(corners[0]);
		this->_coloredTexturedVertices[1].set(corners[1]);
		this->_coloredTexturedVertices[2].set(corners[2]);
		this->_coloredTexturedVertices[3].set(corners[1]);
		this->_coloredTexturedVertices[4].set(corners[2]);
		this->_coloredTexturedVertices[5].set(corners[3]);
		this->_coloredTexturedVertices[0].u = this->_coloredTexturedVertices[2].u = this->_coloredTexturedVertices[4].u = src.x;
		this->_coloredTexturedVertices[0].v = this->_coloredTexturedVertices[1].v = this->_coloredTexturedVertices[3].v = src.y;
		this->_coloredTexturedVertices[1].u = this->_coloredTexturedVertices[3].u = this->_coloredTexturedVertices[5].u = src.x + src.w;
		this->_coloredTexturedVertices[2].v = this->_coloredTexturedVertices[4].v = this->_coloredTexturedVertices[5].v = src.y + src.h;
		return this->_renderColoredTexturedVertices(RO_TRIANGLE_LIST);
	}

	bool RenderHelperLayered2D::_renderColoredVertices(RenderOperation renderOperation)
	{
		// find bounding rect
		this->_min.set(this->_coloredVertices[0].x, this->_coloredVertices[0].y);
		this->_max = this->_min;
		for_iter (i, 1, this->_coloredVerticesCount)
		{
			this->_min.x = hmin(this->_min.x, this->_coloredVertices[i].x);
			this->_min.y = hmin(this->_min.y, this->_coloredVertices[i].y);
			this->_max.x = hmax(this->_max.x, this->_coloredVertices[i].x);
			this->_max.y = hmax(this->_max.y, this->_coloredVertices[i].y);
		}
		this->_boundingRect.set(this->_min, this->_max - this->_min);
		if (april::rendersys->state->viewport.x != 0.0f || april::rendersys->state->viewport.y != 0.0f)
		{
			gvec3 offset = this->_transformationMatrix * gvec3(april::rendersys->state->viewport.x, april::rendersys->state->viewport.y, 0.0f);
			this->_boundingRect += gvec2(offset.x, offset.y);
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
				if (this->_boundingRect.intersects(*it2))
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
			this->layers += new Layer(this->_boundingRect, renderOperation, this->_coloredVertices, this->_coloredVerticesCount);
		}
		else
		{
			validLayer->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
			validLayer->rects += this->_boundingRect;
		}
		return true;
	}

	bool RenderHelperLayered2D::_renderColoredTexturedVertices(RenderOperation renderOperation)
	{
		// find bounding rect
		this->_min.set(this->_coloredTexturedVertices[0].x, this->_coloredTexturedVertices[0].y);
		this->_max = this->_min;
		for_iter (i, 1, this->_coloredTexturedVerticesCount)
		{
			this->_min.x = hmin(this->_min.x, this->_coloredTexturedVertices[i].x);
			this->_min.y = hmin(this->_min.y, this->_coloredTexturedVertices[i].y);
			this->_max.x = hmax(this->_max.x, this->_coloredTexturedVertices[i].x);
			this->_max.y = hmax(this->_max.y, this->_coloredTexturedVertices[i].y);
		}
		this->_boundingRect.set(this->_min, this->_max - this->_min);
		if (april::rendersys->state->viewport.x != 0.0f || april::rendersys->state->viewport.y != 0.0f)
		{
			gvec3 offset = this->_transformationMatrix * gvec3(april::rendersys->state->viewport.x, april::rendersys->state->viewport.y, 0.0f);
			this->_boundingRect += gvec2(offset.x, offset.y);
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
				if (this->_boundingRect.intersects(*it2))
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
			this->layers += new Layer(this->_boundingRect, renderOperation, this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
		}
		else
		{
			validLayer->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
			validLayer->rects += this->_boundingRect;
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
