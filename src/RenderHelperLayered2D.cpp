/// @file
/// @version 4.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "RenderHelperLayered2D.h"
#include "RenderSystem.h"

#define LINE_VERTEX_POOL_SIZE 8
#define TRIANGLE_VERTEX_POOL_SIZE 6

namespace april
{
	// optimizations, but they are not thread-safe
	static PlainVertex pv[LINE_VERTEX_POOL_SIZE];
	static TexturedVertex tv[LINE_VERTEX_POOL_SIZE];
	static const grect screenRect(-1.0f, -1.0f, 2.0f, 2.0f);

	RenderHelperLayered2D::RenderCall::RenderCall(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(false)
	{
		this->renderOperation = renderOperation;
		this->plainVertices = new PlainVertex[count];
		memcpy(this->plainVertices, vertices, count * sizeof(PlainVertex));
		this->count = count;
		this->color = color;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(true)
	{
		this->renderOperation = renderOperation;
		this->texturedVertices = new TexturedVertex[count];
		memcpy(this->texturedVertices, vertices, count * sizeof(TexturedVertex));
		this->count = count;
		this->color = color;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(false)
	{
		this->renderOperation = renderOperation;
		this->coloredVertices = new ColoredVertex[count];
		memcpy(this->coloredVertices, vertices, count * sizeof(ColoredVertex));
		this->count = count;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(true)
	{
		this->renderOperation = renderOperation;
		this->coloredTexturedVertices = new ColoredTexturedVertex[count];
		memcpy(this->coloredTexturedVertices, vertices, count * sizeof(ColoredTexturedVertex));
		this->count = count;
	}

	RenderHelperLayered2D::RenderCall::~RenderCall()
	{
		if (this->plainVertices != NULL)
		{
			delete[] this->plainVertices;
		}
		if (this->texturedVertices != NULL)
		{
			delete[] this->texturedVertices;
		}
		if (this->coloredVertices != NULL)
		{
			delete[] this->coloredVertices;
		}
		if (this->coloredTexturedVertices != NULL)
		{
			delete[] this->coloredTexturedVertices;
		}
	}

	RenderHelperLayered2D::Layer::Layer(int index, RenderCall* renderCall, cgrect rect)
	{
		this->index = index;
		this->rects += rect;
		this->renderOperation = renderCall->renderOperation;
		this->state.modelviewMatrixChanged = false;
		this->state.projectionMatrixChanged = false;
		this->state.viewportChanged = false;
		this->state.useTexture = renderCall->useTexture;
		this->state.useColor = true;
		this->state.viewport = renderCall->state.viewport;
		this->state.texture = renderCall->state.texture;
		this->state.blendMode = renderCall->state.blendMode;
		this->state.colorMode = renderCall->state.colorMode;
		this->state.colorModeFactor = renderCall->state.colorModeFactor;
		this->state.systemColor = Color::White;
	}

	RenderHelperLayered2D::Layer::~Layer()
	{
	}

	RenderHelperLayered2D::RenderHelperLayered2D(const hmap<hstr, hstr>& options) : RenderHelper(options), layeringThread(&_threadUpdate, "APRIL layered 2D renderer"),
		_coloredVertices(NULL), _coloredVerticesCount(0), _coloredVerticesCapacity(0), _coloredTexturedVertices(NULL), _coloredTexturedVerticesCount(0),
		_coloredTexturedVerticesCapacity(0), _nativeColor(0), _potCount(0)
	{
		this->maxLayers = (int)options.tryGet("max_layers", 0);
		this->layerPullUpMerge = (bool)options.tryGet("layer_pull_up_merge", true);
	}

	RenderHelperLayered2D::~RenderHelperLayered2D()
	{
		this->destroy();
		if (this->_coloredVertices != NULL)
		{
			delete[] this->_coloredVertices;
		}
		if (this->_coloredTexturedVertices != NULL)
		{
			delete[] this->_coloredTexturedVertices;
		}
	}

	bool RenderHelperLayered2D::create()
	{
		if (!RenderHelper::create())
		{
			return false;
		}
		this->layeringThread.start();
		return true;
	}

	bool RenderHelperLayered2D::destroy()
	{
		if (this->created)
		{
			this->layeringThread.join();
		}
		return RenderHelper::destroy();
	}

	void RenderHelperLayered2D::clear()
	{
		hmutex::ScopeLock layersLock(&this->layersMutex);
		hmutex::ScopeLock renderCallsLock(&this->renderCallsMutex);
		foreach (Layer*, it, this->layers)
		{
			delete (*it);
		}
		this->layers.clear();
		foreach (RenderCall*, it, this->renderCalls)
		{
			delete (*it);
		}
		this->renderCalls.clear();
	}

	void RenderHelperLayered2D::flush()
	{
		this->_waitForCalculations();
		hmutex::ScopeLock lock(&this->layersMutex);
		if (this->layers.size() == 0)
		{
			return;
		}
		harray<Layer*> layers = this->layers;
		this->layers.clear();
		lock.release();
		layers.first()->state.modelviewMatrixChanged = true;
		layers.first()->state.projectionMatrixChanged = true;
		RenderState* state = april::rendersys->state;
		bool anyViewportChanged = false;
		foreach (Layer*, it, layers)
		{
			april::rendersys->state = &(*it)->state;
			if (april::rendersys->state->viewport != april::rendersys->deviceState->viewport)
			{
				anyViewportChanged = april::rendersys->state->viewportChanged = true;
			}
			if ((*it)->coloredVertices.size() > 0)
			{
				april::rendersys->_renderInternal((*it)->renderOperation, (ColoredVertex*)(*it)->coloredVertices, (*it)->coloredVertices.size());
			}
			else if ((*it)->coloredTexturedVertices.size() > 0)
			{
				april::rendersys->_renderInternal((*it)->renderOperation, (ColoredTexturedVertex*)(*it)->coloredTexturedVertices, (*it)->coloredTexturedVertices.size());
			}
			delete (*it);
		}
		april::rendersys->state = state;
		if (anyViewportChanged)
		{
			april::rendersys->state->viewportChanged = true;
		}
		april::rendersys->state->modelviewMatrixChanged = true;
		april::rendersys->state->projectionMatrixChanged = true;
	}

	void RenderHelperLayered2D::_threadUpdate(hthread* thread)
	{
		RenderHelperLayered2D* self = dynamic_cast<RenderHelperLayered2D*>(april::rendersys->renderHelper);
		hmutex::ScopeLock layeringLock;
		hmutex::ScopeLock renderCallslock;
		RenderCall* renderCall = NULL;
		while (thread->isRunning())
		{
			layeringLock.acquire(&self->layeringMutex);
			renderCallslock.acquire(&self->renderCallsMutex);
			if (self->renderCalls.size() == 0)
			{
				renderCallslock.release();
				layeringLock.release();
				hthread::sleep(0.01f);
				continue;
			}
			renderCall = self->renderCalls.removeFirst();
			renderCallslock.release();
			self->_calculateRenderCall(renderCall);
			layeringLock.release();
			delete renderCall;
		}
	}

	void RenderHelperLayered2D::_waitForCalculations()
	{
		hmutex::ScopeLock lock(&this->layeringMutex);
		while (this->renderCalls.size() > 0)
		{
			lock.release();
			hthread::sleep(0.01f);
			lock.acquire(&this->layeringMutex);
		}
	}

	bool RenderHelperLayered2D::_tryForcedFlush(RenderOperation renderOperation)
	{
		if ((renderOperation != RenderOperation::TriangleList && renderOperation != RenderOperation::LineList) ||
			april::rendersys->state->depthBuffer || april::rendersys->state->depthBufferWrite)
		{
			this->flush();
			return true;
		}
		if (april::rendersys->state->viewportChanged)
		{
			this->flush();
			april::rendersys->state->viewportChanged = false;
		}
		return false;
	}

	void RenderHelperLayered2D::_calculateRenderCall(RenderCall* renderCall)
	{
		if (!renderCall->useTexture)
		{
			if (renderCall->plainVertices != NULL)
			{
				this->_updateVertices(renderCall, renderCall->plainVertices, renderCall->count, renderCall->color);
			}
			else
			{
				this->_updateVertices(renderCall, renderCall->coloredVertices, renderCall->count);
			}
			this->_addRenderLayer(renderCall);
		}
		else
		{
			if (renderCall->texturedVertices != NULL)
			{
				this->_updateVertices(renderCall, renderCall->texturedVertices, renderCall->count, renderCall->color);
			}
			else
			{
				this->_updateVertices(renderCall, renderCall->coloredTexturedVertices, renderCall->count);
			}
			this->_addRenderLayerTextured(renderCall);
		}
	}

	void RenderHelperLayered2D::_makeBoundingRectColoredVertices()
	{
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
	}

	void RenderHelperLayered2D::_makeBoundingRectColoredTexturedVertices()
	{
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
	}

	void RenderHelperLayered2D::_addRenderLayer(RenderCall* renderCall)
	{
		this->_makeBoundingRectColoredVertices();
		if (!this->_boundingRect.intersects(screenRect))
		{
			return;
		}
		// check existing rects
		Layer* currentValidLayer = NULL;
		Layer* lastValidLayer = NULL;
		int intersectionIndex = -1;
		hmutex::ScopeLock lock(&this->layersMutex);
		Layer* layer = this->_processIntersection(renderCall, &currentValidLayer, &lastValidLayer, intersectionIndex);
		layer->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
	}

	void RenderHelperLayered2D::_addRenderLayerTextured(RenderCall* renderCall)
	{
		this->_makeBoundingRectColoredTexturedVertices();
		if (!this->_boundingRect.intersects(screenRect))
		{
			return;
		}
		// check existing rects
		Layer* currentValidLayer = NULL;
		Layer* lastValidLayer = NULL;
		int intersectionIndex = -1;
		hmutex::ScopeLock lock(&this->layersMutex);
		Layer* layer = this->_processIntersection(renderCall, &currentValidLayer, &lastValidLayer, intersectionIndex);
		layer->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
	}

	bool RenderHelperLayered2D::_checkCurrentIntersection(Layer* layer)
	{
		foreach (grect, it, layer->rects)
		{
			if (this->_boundingRect.intersects(*it))
			{
				return true;
			}
		}
		return false;
	}

	RenderHelperLayered2D::Layer* RenderHelperLayered2D::_processIntersection(RenderCall* renderCall, Layer** currentValidLayer, Layer** lastValidLayer, int& intersectionIndex)
	{
		HL_LAMBDA_CLASS(_sortLayers, bool, ((RenderHelperLayered2D::Layer* const& a, RenderHelperLayered2D::Layer* const& b) { return (a->index < b->index); }));
		// find first intersection
		int intersectedIndex = -1;
		Layer* intersectedLayer = NULL;
		Layer* layer = NULL;
		for_iter_r (i, this->layers.size(), 0)
		{
			if (this->_checkCurrentIntersection(this->layers[i]))
			{
				intersectedIndex = i;
				intersectedLayer = this->layers[i];
				break;
			}
		}
		if (intersectedIndex >= 0)
		{
			// find all matching layers from the parallel layers of the intersected layer
			harray<Layer*> belowLayers;
			harray<Layer*> aboveLayers;
			foreach (Layer*, it, intersectedLayer->parallelLayers)
			{
				if ((*it)->state.useTexture == renderCall->useTexture &&
					(*it)->renderOperation == renderCall->renderOperation &&
					(*it)->state.blendMode == renderCall->state.blendMode &&
					(*it)->state.colorMode == renderCall->state.colorMode &&
					(*it)->state.colorModeFactor == renderCall->state.colorModeFactor &&
					(*it)->state.texture == renderCall->state.texture &&
					!this->_checkCurrentIntersection(*it))
				{
					if ((*it)->index < intersectedIndex)
					{
						if (this->layerPullUpMerge)
						{
							belowLayers += (*it);
						}
					}
					else
					{
						aboveLayers += (*it);
					}
				}
			}
			if (belowLayers.size() > 0)
			{
				belowLayers.sort(&_sortLayers::lambda);
				// check below matching layers for a matching layer
				bool valid = false;
				foreach_r (Layer*, it, belowLayers)
				{
					valid = true;
					for_iter (i, (*it)->index + 1, intersectedIndex)
					{
						if (!(*it)->parallelLayers.has(this->layers[i]) || !intersectedLayer->parallelLayers.has(this->layers[i]) || this->_checkCurrentIntersection(this->layers[i]))
						{
							valid = false;
							break;
						}
					}
					if (valid)
					{
						(*it)->rects += this->_boundingRect;
						intersectedLayer->parallelLayers -= (*it);
						(*it)->parallelLayers -= intersectedLayer;
						harray<Layer*> parallelLayers = (*it)->parallelLayers;
						foreach (Layer*, it2, parallelLayers)
						{
							if ((*it2)->index < (*it)->index && this->_checkCurrentIntersection(*it2))
							{
								(*it2)->parallelLayers -= (*it);
								(*it)->parallelLayers -= (*it2);
							}
						}
						this->layers.removeAt((*it)->index);
						this->layers.insertAt(intersectedIndex, (*it));
						for_iter (i, (*it)->index, intersectedIndex + 1)
						{
							this->layers[i]->index = i;
						}
						return (*it);
					}
				}
			}
			if (aboveLayers.size() > 0)
			{
				layer = aboveLayers.min(&_sortLayers::lambda);
				layer->rects += this->_boundingRect;
				harray<Layer*> parallelLayers = layer->parallelLayers;
				foreach (Layer*, it, parallelLayers)
				{
					if ((*it)->index < layer->index && this->_checkCurrentIntersection(*it))
					{
						(*it)->parallelLayers -= layer;
						layer->parallelLayers -= (*it);
					}
				}
				return layer;
			}
		}
		layer = new Layer(intersectedIndex + 1, renderCall, this->_boundingRect);
		if (hbetweenIE(intersectedIndex, 0, this->layers.size()))
		{
			if (this->layerPullUpMerge)
			{
				for_iter (i, 0, intersectedIndex)
				{
					if (!this->_checkCurrentIntersection(this->layers[i]))
					{
						layer->parallelLayers += this->layers[i];
					}
				}
			}
			if (intersectedIndex < this->layers.size() - 1)
			{
				layer->parallelLayers += this->layers(intersectedIndex + 1, this->layers.size() - intersectedIndex - 1);
			}
			this->layers.insertAt(intersectedIndex + 1, layer);
			for_iter (i, intersectedIndex + 2, this->layers.size())
			{
				this->layers[i]->index = i;
			}
		}
		else
		{
			layer->index = this->layers.size();
			layer->parallelLayers = this->layers;
			this->layers += layer;
		}
		foreach (Layer*, it, layer->parallelLayers)
		{
			(*it)->parallelLayers += layer;
		}
		return layer;
	}

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, PlainVertex* vertices, int count, Color color)
	{
		this->_updateColoredVerticesSize(count);
		this->_nativeColor = april::rendersys->getNativeColorUInt(color);
		this->_transformationMatrix = renderCall->state.projectionMatrix * renderCall->state.modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
			this->_coloredVertices[i].color = this->_nativeColor;
		}
	}

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, TexturedVertex* vertices, int count, Color color)
	{
		this->_updateColoredTexturedVerticesSize(count);
		this->_nativeColor = april::rendersys->getNativeColorUInt(color);
		this->_transformationMatrix = renderCall->state.projectionMatrix * renderCall->state.modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
			this->_coloredTexturedVertices[i].u = vertices[i].u;
			this->_coloredTexturedVertices[i].v = vertices[i].v;
			this->_coloredTexturedVertices[i].color = this->_nativeColor;
		}
	}

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, ColoredVertex* vertices, int count)
	{
		this->_updateColoredVerticesSize(count);
		memcpy(this->_coloredVertices, vertices, count * sizeof(ColoredVertex));
		this->_transformationMatrix = renderCall->state.projectionMatrix * renderCall->state.modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
		}
	}

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, ColoredTexturedVertex* vertices, int count)
	{
		this->_updateColoredTexturedVerticesSize(count);
		memcpy(this->_coloredTexturedVertices, vertices, count * sizeof(ColoredTexturedVertex));
		this->_transformationMatrix = renderCall->state.projectionMatrix * renderCall->state.modelviewMatrix;
		for_iter (i, 0, count)
		{
			this->_coloredTexturedVertices[i].set(this->_transformationMatrix * vertices[i].toGvec3());
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

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		return this->render(renderOperation, vertices, count, Color::White);
	}

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->maxLayers > 0 && this->layers.size() > this->maxLayers)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count, color);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		return this->render(renderOperation, vertices, count, Color::White);
	}

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->maxLayers > 0 && this->layers.size() > this->maxLayers)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count, color);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->maxLayers > 0 && this->layers.size() > this->maxLayers)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->maxLayers > 0 && this->layers.size() > this->maxLayers)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::drawRect(cgrect rect, const Color& color)
	{
		pv[0].x = pv[2].x = pv[4].x = pv[5].x = rect.x;
		pv[0].y = pv[1].y = pv[4].y = pv[6].y = rect.y;
		pv[1].x = pv[3].x = pv[6].x = pv[7].x = rect.x + rect.w;
		pv[2].y = pv[3].y = pv[5].y = pv[7].y = rect.y + rect.h;
		return this->render(RenderOperation::LineList, pv, LINE_VERTEX_POOL_SIZE, color);
	}

	bool RenderHelperLayered2D::drawFilledRect(cgrect rect, const Color& color)
	{
		pv[0].x = pv[2].x = pv[4].x = rect.x;
		pv[0].y = pv[1].y = pv[3].y = rect.y;
		pv[1].x = pv[3].x = pv[5].x = rect.x + rect.w;
		pv[2].y = pv[4].y = pv[5].y = rect.y + rect.h;
		return this->render(RenderOperation::TriangleList, pv, TRIANGLE_VERTEX_POOL_SIZE, color);
	}

	bool RenderHelperLayered2D::drawTexturedRect(cgrect rect, cgrect src)
	{
		return this->drawTexturedRect(rect, src, Color::White);
	}

	bool RenderHelperLayered2D::drawTexturedRect(cgrect rect, cgrect src, const Color& color)
	{
		tv[0].x = tv[2].x = tv[4].x = rect.x;
		tv[0].y = tv[1].y = tv[3].y = rect.y;
		tv[0].u = tv[2].u = tv[4].u = src.x;
		tv[0].v = tv[1].v = tv[3].v = src.y;
		tv[1].x = tv[3].x = tv[5].x = rect.x + rect.w;
		tv[2].y = tv[4].y = tv[5].y = rect.y + rect.h;
		tv[1].u = tv[3].u = tv[5].u = src.x + src.w;
		tv[2].v = tv[4].v = tv[5].v = src.y + src.h;
		return this->render(RenderOperation::TriangleList, tv, TRIANGLE_VERTEX_POOL_SIZE, color);
	}

}
