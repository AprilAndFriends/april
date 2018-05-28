/// @file
/// @version 5.0
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

#ifdef _DEBUG
//#define _DEBUG_TESTING 300
//#define _DEBUG_BOUNDING_RECTS
#endif

//#define MAX_LAYERS 10000
//#define MAX_LAYER_CHECKS 100
//#define SIMPLE_ALGORITHM

namespace april
{
	class RenderHelperLayered2D : public RenderHelper
	{
	public:
		RenderHelperLayered2D(const hmap<hstr, hstr>& options);
		~RenderHelperLayered2D();

		bool create();
		bool destroy();
		void clear();
		void flush();

		bool render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		bool render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color);
		bool render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		bool render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color);
		bool render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		bool render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		bool drawRect(cgrectf rect, const Color& color);
		bool drawFilledRect(cgrectf rect, const Color& color);
		bool drawTexturedRect(cgrectf rect, cgrectf src);
		bool drawTexturedRect(cgrectf rect, cgrectf src, const Color& color);

	protected:
		class RenderCall
		{
		public:
			RenderState state;
			RenderOperation renderOperation;
			PlainVertex* plainVertices;
			TexturedVertex* texturedVertices;
			ColoredVertex* coloredVertices;
			ColoredTexturedVertex* coloredTexturedVertices;
			int count;
			Color color;
			bool useTexture;

			RenderCall(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color);
			RenderCall(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color);
			RenderCall(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
			RenderCall(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
			~RenderCall();

		};

		class Layer
		{
		public:
			int index;
			RenderState state;
			RenderOperation renderOperation;
			harray<grectf> rects;
			harray<ColoredVertex> coloredVertices;
			harray<ColoredTexturedVertex> coloredTexturedVertices;
			harray<Layer*> parallelLayers;

			Layer(int index, RenderCall* renderCall, cgrectf rect);
			~Layer();

		};

		int maxLayers;
		bool layerPullUpMerge;
		harray<RenderCall*> renderCalls;
		hmutex renderCallsMutex;
		harray<Layer*> layers;
		hmutex layersMutex;
		hmutex layeringMutex;
		hthread layeringThread;

		void _waitForCalculations();
		bool _tryForcedFlush(RenderOperation renderOperation);
		void _calculateRenderCall(RenderCall* renderCall);
		void _makeBoundingRectColoredVertices();
		void _makeBoundingRectColoredTexturedVertices();
		void _addRenderLayer(RenderCall* renderCall);
		void _addRenderLayerTextured(RenderCall* renderCall);
		Layer* _processIntersection(RenderCall* renderCall, Layer** currentValidLayer, Layer** lastValidLayer, int& intersectionIndex);
		bool _checkCurrentIntersection(Layer* layer);

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
		gvec2f _min;
		gvec2f _max;
		grectf _boundingRect;

		void _updateVertices(RenderCall* renderCall, PlainVertex* vertices, int count, Color color);
		void _updateVertices(RenderCall* renderCall, TexturedVertex* vertices, int count, Color color);
		void _updateVertices(RenderCall* renderCall, ColoredVertex* vertices, int count);
		void _updateVertices(RenderCall* renderCall, ColoredTexturedVertex* vertices, int count);
		void _updateColoredVerticesSize(int count);
		void _updateColoredTexturedVerticesSize(int count);

	};

}
#endif
