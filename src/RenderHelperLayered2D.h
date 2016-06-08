/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a render helper for layered 2D rendering.

#ifndef APRIL_RENDER_HELPER_LAYERED_2D_H
#define APRIL_RENDER_HELPER_LAYERED_2D_H

#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "aprilUtil.h"
#include "RenderHelper.h"
#include "RenderState.h"

#define MAX_VERTEX_POOL 32768

#ifdef _DEBUG
//#define _DEBUG_TESTING
#endif

namespace april
{
	class RenderHelperLayered2D : public RenderHelper
	{
	public:
		RenderHelperLayered2D();
		~RenderHelperLayered2D();

		bool create();
		bool destroy();
		void clear();
		void flush();

		bool render(RenderOperation renderOperation, PlainVertex* vertices, int count);
		bool render(RenderOperation renderOperation, PlainVertex* vertices, int count, Color color);
		bool render(RenderOperation renderOperation, TexturedVertex* vertices, int count);
		bool render(RenderOperation renderOperation, TexturedVertex* vertices, int count, Color color);
		bool render(RenderOperation renderOperation, ColoredVertex* vertices, int count);
		bool render(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count);
		bool drawRect(grect rect, Color color);
		bool drawFilledRect(grect rect, Color color);
		bool drawTexturedRect(grect rect, grect src);
		bool drawTexturedRect(grect rect, grect src, Color color);
		
	protected:
		class VertexPool
		{
		public:
			VertexPool();
			~VertexPool();

			PlainVertex* allocate(PlainVertex* data, int count);
			TexturedVertex* allocate(TexturedVertex* data, int count);
			ColoredVertex* allocate(ColoredVertex* data, int count);
			ColoredTexturedVertex* allocate(ColoredTexturedVertex* data, int count);
			void release(PlainVertex* data);
			void release(TexturedVertex* data);
			void release(ColoredVertex* data);
			void release(ColoredTexturedVertex* data);

		protected:
			template <typename T>
			class Allocation
			{
			public:
				T* data;
				int index;
				int count;

				Allocation(T* data, int index, int count)
				{
					this->data = data;
					this->index = index;
					this->count = count;
				}

				~Allocation()
				{
					if (this->index < 0) // indicates dynamic allocation
					{
						delete[] this->data;
					}
				}

			};

			PlainVertex plainVertices[MAX_VERTEX_POOL];
			TexturedVertex texturedVertices[MAX_VERTEX_POOL];
			ColoredVertex coloredVertices[MAX_VERTEX_POOL];
			ColoredTexturedVertex coloredTexturedVertices[MAX_VERTEX_POOL];
			harray<Allocation<PlainVertex>* > plainAllocations;
			harray<Allocation<TexturedVertex>* > texturedAllocations;
			harray<Allocation<ColoredVertex>* > coloredAllocations;
			harray<Allocation<ColoredTexturedVertex>* > coloredTexturedAllocations;
			hmutex plainMutex;
			hmutex texturedMutex;
			hmutex coloredMutex;
			hmutex coloredTexturedMutex;

		};

		class RenderCall
		{
		public:
			VertexPool* vertexPool;
			RenderState state;
			RenderOperation renderOperation;
			PlainVertex* plainVertices;
			TexturedVertex* texturedVertices;
			ColoredVertex* coloredVertices;
			ColoredTexturedVertex* coloredTexturedVertices;
			int count;
			Color color;
			bool useTexture;

			RenderCall(VertexPool* vertexPool, RenderOperation renderOperation, PlainVertex* vertices, int count, april::Color color);
			RenderCall(VertexPool* vertexPool, RenderOperation renderOperation, TexturedVertex* vertices, int count, april::Color color);
			RenderCall(VertexPool* vertexPool, RenderOperation renderOperation, ColoredVertex* vertices, int count);
			RenderCall(VertexPool* vertexPool, RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count);
			~RenderCall();

		};

		class Layer
		{
		public:
			RenderState state;
			RenderOperation renderOperation;
			harray<grect> rects;
#ifdef _DEBUG_TESTING
			harray<gvec2> offsets;
#endif
			harray<ColoredVertex> coloredVertices;
			harray<ColoredTexturedVertex> coloredTexturedVertices;

			Layer(RenderCall* renderCall, const grect& rect, ColoredVertex* vertices, int count);
			Layer(RenderCall* renderCall, const grect& rect, ColoredTexturedVertex* vertices, int count);
			~Layer();

		};

		VertexPool vertexPool;
		harray<RenderCall*> renderCalls;
		hmutex renderCallsMutex;
		harray<Layer*> layers;
		hmutex layersMutex;
		hmutex layeringMutex;
		hthread layeringThread;

		void _waitForCalculations();
		bool _tryForcedFlush(RenderOperation renderOperation);
		void _calculateRenderCall(RenderCall* renderCall);
		void _addRenderLayerNonTextured(RenderCall* renderCall);
		void _addRenderLayerTextured(RenderCall* renderCall);
		void _solveIntersection(RenderCall* renderCall, Layer** currentValidLayer, Layer** lastValidLayer, int& intersectionIndex);

		static void _threadUpdate(hthread* thread);

	private:
		ColoredVertex* _coloredVertices;
		int _coloredVerticesCount;
		int _coloredVerticesCapacity;
		ColoredTexturedVertex* _coloredTexturedVertices;
		int _coloredTexturedVerticesCount;
		int _coloredTexturedVerticesCapacity;

		unsigned int _nativeColor;
		int _potCount;
		gmat4 _transformationMatrix;
		gvec2 _min;
		gvec2 _max;
		grect _boundingRect;

		void _updateVertices(RenderCall* renderCall, PlainVertex* vertices, int count, april::Color color);
		void _updateVertices(RenderCall* renderCall, TexturedVertex* vertices, int count, april::Color color);
		void _updateVertices(RenderCall* renderCall, ColoredVertex* vertices, int count);
		void _updateVertices(RenderCall* renderCall, ColoredTexturedVertex* vertices, int count);
		void _updateColoredVerticesSize(int count);
		void _updateColoredTexturedVerticesSize(int count);

	};

}
#endif
