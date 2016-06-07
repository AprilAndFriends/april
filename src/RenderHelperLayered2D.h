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
#include <hltypes/hstring.h>

#include "aprilUtil.h"
#include "RenderHelper.h"
#include "RenderState.h"

#define MAX_LAYER_RECTS 10

namespace april
{
	class RenderHelperLayered2D : public RenderHelper
	{
	public:
		RenderHelperLayered2D();
		~RenderHelperLayered2D();

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
		
		void flush();
		void clear();

	protected:
		class Layer
		{
		public:
			RenderState state;
			RenderOperation renderOperation;
			harray<grect> rects;
			harray<ColoredVertex> coloredVertices;
			harray<ColoredTexturedVertex> coloredTexturedVertices;

			Layer(grect rect, RenderOperation renderOperation, ColoredVertex* vertices, int count);
			Layer(grect rect, RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count);
			~Layer();

		};

		harray<Layer*> layers;

		bool _tryForcedFlush(RenderOperation renderOperation);
		bool _renderColoredVertices(RenderOperation renderOperation);
		bool _renderColoredTexturedVertices(RenderOperation renderOperation);

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

		void _updateVertices(PlainVertex* vertices, int count, april::Color color);
		void _updateVertices(TexturedVertex* vertices, int count, april::Color color);
		void _updateVertices(ColoredVertex* vertices, int count);
		void _updateVertices(ColoredTexturedVertex* vertices, int count);
		void _updateColoredVerticesSize(int count);
		void _updateColoredTexturedVerticesSize(int count);

	};

}
#endif
