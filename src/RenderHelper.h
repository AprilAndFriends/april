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
/// Defines a generic render helper.

#ifndef APRIL_RENDER_HELPER_H
#define APRIL_RENDER_HELPER_H

#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "aprilUtil.h"
#include "Color.h"

namespace april
{
	class aprilExport RenderHelper
	{
	public:
		RenderHelper(const hmap<hstr, hstr>& options);
		virtual ~RenderHelper();

		virtual bool create();
		virtual bool destroy();
		virtual void clear();
		virtual void flush();

		virtual bool render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		virtual bool render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color);
		virtual bool render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		virtual bool render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color);
		virtual bool render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		virtual bool render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		virtual bool drawRect(cgrect rect, const Color& color);
		virtual bool drawFilledRect(cgrect rect, const Color& color);
		virtual bool drawTexturedRect(cgrect rect, cgrect src);
		virtual bool drawTexturedRect(cgrect rect, cgrect src, const Color& color);

	protected:
		bool created;

	};

}
#endif
