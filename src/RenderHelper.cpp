/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "RenderHelper.h"

namespace april
{
	RenderHelper::RenderHelper(const hmap<hstr, hstr>& options) : created(false)
	{
	}
	
	RenderHelper::~RenderHelper()
	{
	}

	bool RenderHelper::create()
	{
		if (!this->created)
		{
			this->created = true;
			return true;
		}
		return false;
	}

	bool RenderHelper::destroy()
	{
		if (this->created)
		{
			this->created = false;
			this->flush();
			this->clear();
			return true;
		}
		return false;
	}

	void RenderHelper::clear()
	{
	}

	void RenderHelper::flush()
	{
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const PlainVertex* vertices, int count, Color color)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count, Color color)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::render(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::drawRect(cgrect rect, const Color& color)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::drawFilledRect(cgrect rect, const Color& color)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::drawTexturedRect(cgrect rect, cgrect src)
	{
		this->flush();
		return false;
	}

	bool RenderHelper::drawTexturedRect(cgrect rect, cgrect src, const Color& color)
	{
		this->flush();
		return false;
	}

}
