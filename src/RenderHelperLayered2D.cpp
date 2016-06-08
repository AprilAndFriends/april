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

namespace april
{
	// optimizations, but they are not thread-safe
	static PlainVertex pv[8];
	static TexturedVertex tv[6];

	RenderHelperLayered2D::RenderCall::RenderCall(RenderOperation renderOperation, PlainVertex* vertices, int count, april::Color color) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(false)
	{
		this->renderOperation = renderOperation;
		this->plainVertices = new PlainVertex[count];
		memcpy(this->plainVertices, vertices, count * sizeof(PlainVertex));
		this->count = count;
		this->color = color;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(RenderOperation renderOperation, TexturedVertex* vertices, int count, april::Color color) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(true)
	{
		this->renderOperation = renderOperation;
		this->texturedVertices = new TexturedVertex[count];
		memcpy(this->texturedVertices, vertices, count * sizeof(TexturedVertex));
		this->count = count;
		this->color = color;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(RenderOperation renderOperation, ColoredVertex* vertices, int count) :
		state(*april::rendersys->state), plainVertices(NULL), texturedVertices(NULL), coloredVertices(NULL), coloredTexturedVertices(NULL), useTexture(false)
	{
		this->renderOperation = renderOperation;
		this->coloredVertices = new ColoredVertex[count];
		memcpy(this->coloredVertices, vertices, count * sizeof(ColoredVertex));
		this->count = count;
	}

	RenderHelperLayered2D::RenderCall::RenderCall(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count) :
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

	RenderHelperLayered2D::Layer::Layer(RenderCall* renderCall, const grect& rect)
	{
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
	}

	RenderHelperLayered2D::Layer::~Layer()
	{
	}

	RenderHelperLayered2D::RenderHelperLayered2D() : RenderHelper(), layeringThread(&_threadUpdate, "APRIL layered 2D renderer"), _coloredVertices(NULL),
		_coloredVerticesCount(0), _coloredVerticesCapacity(0), _coloredTexturedVertices(NULL), _coloredTexturedVerticesCount(0),
		_coloredTexturedVerticesCapacity(0), _nativeColor(0), _potCount(0)
	{
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
#ifdef _DEBUG_TESTING
		//hlog::debug("OK", this->layers.size());
#endif
		harray<Layer*> layers = this->layers;
		this->layers.clear();
		lock.release();
		layers.first()->state.modelviewMatrixChanged = true;
		layers.first()->state.projectionMatrixChanged = true;
		RenderState* state = april::rendersys->state;
		bool anyViewportChanged = false;
#ifdef _DEBUG_TESTING
		int count = 5;
#endif
		foreach (Layer*, it, layers)
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
			for_iter (i, 0, (*it)->rects.size())
			{
				april::rendersys->_drawRectInternal((*it)->rects[i] - (*it)->offsets[i], april::Color::Cyan);
			}
#endif
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
		if (renderOperation != RO_TRIANGLE_LIST && renderOperation != RO_LINE_LIST ||
			april::rendersys->state->depthBuffer || april::rendersys->state->depthBufferWrite)
		{
			this->flush();
			return true;
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
			this->_addRenderLayerNonTextured(renderCall);
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

	void RenderHelperLayered2D::_addRenderLayerNonTextured(RenderCall* renderCall)
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
		gvec2 viewportOffset;
		//if (renderCall->state.viewport.x != 0.0f || renderCall->state.viewport.y != 0.0f)
		{
			gvec3 offset = renderCall->state.projectionMatrix * gvec3(renderCall->state.viewport.x, renderCall->state.viewport.y, 0.0f);
			viewportOffset.set(offset.x, offset.y);
			this->_boundingRect += viewportOffset;
		}
		/*
		this->_boundingRect.clip(grect(0.0f, 0.0f, renderCall->state.viewport.getSize()));
		if (this->_boundingRect.w <= 0.0f || this->_boundingRect.h <= 0.0f)
		{
			return;
		}
		*/
		// check existing rects
		Layer* currentValidLayer = NULL;
		Layer* lastValidLayer = NULL;
		int intersectionIndex = -1;
		hmutex::ScopeLock lock(&this->layersMutex);
		Layer* layer = this->_processIntersection(renderCall, &currentValidLayer, &lastValidLayer, intersectionIndex);
#ifndef SIMPLE_ALGORITHM
		layer->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
#ifdef _DEBUG_TESTING
		layer->offsets += viewportOffset;
#endif
#else
		if (lastValidLayer != NULL)
		{
			lastValidLayer->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
			lastValidLayer->rects += this->_boundingRect;
#ifdef _DEBUG_TESTING
			lastValidLayer->offsets += viewportOffset;
#endif
		}
		else if (intersectionIndex >= 0 && currentValidLayer == this->layers[intersectionIndex])
		{
			this->layers.insertAt(intersectionIndex + 1, new Layer(renderCall, this->_boundingRect));
			this->layers[intersectionIndex + 1]->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
#ifdef _DEBUG_TESTING
			this->layers[intersectionIndex + 1]->offsets += viewportOffset;
#endif
		}
		else
		{
			this->layers += new Layer(renderCall, this->_boundingRect);
			this->layers.last()->coloredVertices.add(this->_coloredVertices, this->_coloredVerticesCount);
#ifdef _DEBUG_TESTING
			this->layers.last()->offsets += viewportOffset;
#endif
		}
#endif
	}

	void RenderHelperLayered2D::_addRenderLayerTextured(RenderCall* renderCall)
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
		gvec2 viewportOffset;
		//if (renderCall->state.viewport.x != 0.0f || renderCall->state.viewport.y != 0.0f)
		{
			gvec3 offset = renderCall->state.projectionMatrix * gvec3(renderCall->state.viewport.x, renderCall->state.viewport.y, 0.0f);
			viewportOffset.set(offset.x, offset.y);
			this->_boundingRect += viewportOffset;
		}
		/*
		this->_boundingRect.clip(grect(0.0f, 0.0f, renderCall->state.viewport.getSize()));
		if (this->_boundingRect.w <= 0.0f || this->_boundingRect.h <= 0.0f)
		{
			return;
		}
		*/
		// check existing rects
		Layer* currentValidLayer = NULL;
		Layer* lastValidLayer = NULL;
		int intersectionIndex = -1;
		hmutex::ScopeLock lock(&this->layersMutex);
		Layer* layer = this->_processIntersection(renderCall, &currentValidLayer, &lastValidLayer, intersectionIndex);
#ifndef SIMPLE_ALGORITHM
		layer->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
#ifdef _DEBUG_TESTING
		layer->offsets += viewportOffset;
#endif
#else
		if (lastValidLayer != NULL)
		{
			lastValidLayer->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
			lastValidLayer->rects += this->_boundingRect;
#ifdef _DEBUG_TESTING
			lastValidLayer->offsets += viewportOffset;
#endif
		}
		else if (intersectionIndex >= 0 && currentValidLayer == this->layers[intersectionIndex])
		{
			this->layers.insertAt(intersectionIndex + 1, new Layer(renderCall, this->_boundingRect));
			this->layers[intersectionIndex + 1]->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
#ifdef _DEBUG_TESTING
			this->layers[intersectionIndex + 1]->offsets += viewportOffset;
#endif
		}
		else
		{
			this->layers += new Layer(renderCall, this->_boundingRect);
			this->layers.last()->coloredTexturedVertices.add(this->_coloredTexturedVertices, this->_coloredTexturedVerticesCount);
#ifdef _DEBUG_TESTING
			this->layers.last()->offsets += viewportOffset;
#endif
		}
#endif
	}

	RenderHelperLayered2D::Layer* RenderHelperLayered2D::_processIntersection(RenderCall* renderCall, Layer** currentValidLayer, Layer** lastValidLayer, int& intersectionIndex)
	{
#ifndef SIMPLE_ALGORITHM
		int intersectedIndex = -1;
		for_iter_r (i, this->layers.size(), 0)
		{
			foreach (grect, it, this->layers[i]->rects)
			{
				if (this->_boundingRect.intersects(*it))
				{
					intersectedIndex = i;
					break;
				}
			}
		}
		if (intersectedIndex >= 0)
		{
			Layer* validLayer = NULL;
			Layer* parallelLayer = NULL;
			int validIndex = NULL;
			foreach (Layer*, it, this->layers[intersectedIndex]->parallelLayers)
			{
				if ((*it)->state.useTexture == renderCall->useTexture && (*it)->renderOperation == renderCall->renderOperation &&
					(*it)->state.blendMode == renderCall->state.blendMode && (*it)->state.colorMode == renderCall->state.colorMode &&
					(*it)->state.colorModeFactor == renderCall->state.colorModeFactor &&
					(*it)->state.texture == renderCall->state.texture && (*it)->state.viewport == renderCall->state.viewport)
				{
					validLayer = (*it);
					validIndex = this->layers.indexOf(validLayer);
					parallelLayer = this->layers[intersectedIndex];
					this->layers[intersectedIndex]->parallelLayers -= validLayer;
					break;
				}
			}
			if (validLayer == NULL)
			{
				for_iter (i, intersectedIndex + 1, this->layers.size())
				{
					if (this->layers[i]->state.useTexture == renderCall->useTexture && this->layers[i]->renderOperation == renderCall->renderOperation &&
						this->layers[i]->state.blendMode == renderCall->state.blendMode && this->layers[i]->state.colorMode == renderCall->state.colorMode &&
						this->layers[i]->state.colorModeFactor == renderCall->state.colorModeFactor &&
						this->layers[i]->state.texture == renderCall->state.texture && this->layers[i]->state.viewport == renderCall->state.viewport)
					{
						validLayer = this->layers[i];
						validIndex = i;
						break;
					}
				}
			}
			if (validLayer != NULL)
			{
				harray<Layer*> removed;
				validLayer->rects += this->_boundingRect;
				if (parallelLayer != NULL)
				{
					validLayer->parallelLayers -= parallelLayer;
				}
				foreach (Layer*, it, validLayer->parallelLayers)
				{
					foreach (grect, it2, (*it)->rects)
					{
						if (this->_boundingRect.intersects(*it2))
						{
							removed += (*it);
							(*it)->parallelLayers -= validLayer;
							break;
						}
					}
				}
				if (removed.size() > 0 || parallelLayer != NULL)
				{
					validLayer->parallelLayers -= removed;
					if (parallelLayer != NULL)
					{
						removed += parallelLayer;
					}
					harray<int> removedIndexes;
					foreach (Layer*, it, removed)
					{
						removedIndexes += this->layers.indexOf(*it);
					}
					int maxIndex = removedIndexes.max();
					if (validIndex < maxIndex)
					{
						int currentIndex = validIndex;
						for_iter (i, 0, removedIndexes.size())
						{
							if (validIndex < removedIndexes[i])
							{
								if (removedIndexes[i] - currentIndex > 1)
								{
									this->layers.removeAt(validIndex);
									this->layers.insertAt(currentIndex, validLayer);
									break;
								}
								currentIndex = removedIndexes[i];
							}
						}
					}
				}
				return validLayer;
			}
		}
		Layer* layer = new Layer(renderCall, this->_boundingRect);
		if (intersectedIndex >= 0)
		{
			layer->parallelLayers = this->layers(intersectedIndex + 1, this->layers.size() - intersectedIndex - 1);
			this->layers.insertAt(intersectedIndex + 1, layer);
		}
		else
		{
			layer->parallelLayers = this->layers;
			this->layers += layer;
		}
		foreach (Layer*, it, layer->parallelLayers)
		{
			(*it)->parallelLayers += layer;
		}
		return layer;
#else
		for_iter_r (i, this->layers.size(), 0)
		{
			if (*currentValidLayer != NULL)
			{
				*lastValidLayer = *currentValidLayer;
			}
			*currentValidLayer = NULL;
			if (this->layers[i]->state.useTexture == renderCall->useTexture && this->layers[i]->renderOperation == renderCall->renderOperation &&
				this->layers[i]->state.blendMode == renderCall->state.blendMode && this->layers[i]->state.colorMode == renderCall->state.colorMode &&
				this->layers[i]->state.colorModeFactor == renderCall->state.colorModeFactor &&
				this->layers[i]->state.texture == renderCall->state.texture && this->layers[i]->state.viewport == renderCall->state.viewport)
			{
				*currentValidLayer = this->layers[i];
			}
			foreach (grect, it, this->layers[i]->rects)
			{
				if (this->_boundingRect.intersects(*it))
				{
					intersectionIndex = i;
					return NULL;
				}
			}
		}
		return NULL;
#endif
	}

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, PlainVertex* vertices, int count, april::Color color)
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

	void RenderHelperLayered2D::_updateVertices(RenderCall* renderCall, TexturedVertex* vertices, int count, april::Color color)
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
		if (this->layers.size() > MAX_LAYERS)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count, color);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
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
		if (this->layers.size() > MAX_LAYERS)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count, color);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->layers.size() > MAX_LAYERS)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
	}

	bool RenderHelperLayered2D::render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count)
	{
		if (this->_tryForcedFlush(renderOperation) || count == 0)
		{
			return false;
		}
		if (this->layers.size() > MAX_LAYERS)
		{
			this->flush();
		}
		RenderCall* renderCall = new RenderCall(renderOperation, vertices, count);
		hmutex::ScopeLock lock(&this->renderCallsMutex);
		this->renderCalls += renderCall;
		return true;
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
		return this->drawTexturedRect(rect, src, april::Color::White);
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

}
