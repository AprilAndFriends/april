/// @file
/// @author  Boris Mikic
/// @version 2.52
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"
#include "WinRT_View.h"
#include "WinRT_ViewSource.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel::Core;

#define MANIFEST_FILENAME "AppxManifest.xml"
#define SNAP_VIEW_WIDTH 320 // as defined by Microsfot

namespace april
{
	WinRT_Window::WinRT_Window() : Window()
	{
		this->name = APRIL_WS_WINRT;
		this->width = 0;
		this->height = 0;
		this->touchEnabled = false;
		this->multiTouchActive = false;
		this->hasStoredProjectionMatrix = false;
		this->backgroundColor = april::Color::Black;
		this->logoTexture = NULL;
	}

	WinRT_Window::~WinRT_Window()
	{
		this->destroy();
	}

	bool WinRT_Window::create(int w, int h, bool fullscreen, chstr title)
	{
		if (!Window::create(w, h, fullscreen, title))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->multiTouchActive = false;
		this->hasStoredProjectionMatrix = false;
		this->backgroundColor = april::Color::Black;
		this->logoTexture = NULL;
		this->setCursorVisible(true);
		return true;
	}
	
	bool WinRT_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		this->hasStoredProjectionMatrix = false;
		_HL_TRY_DELETE(this->logoTexture);
		return true;
	}

	/*
	void WinRT_Window::setTitle(chstr title)
	{
	}
	*/
	
	void WinRT_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		WinRT::View->setCursorVisible(value);
	}
	
	void* WinRT_Window::getBackendId()
	{
		// TODO ?
		return 0;
	}

	/*
	void WinRT_Window::_setResolution(int w, int h)
	{
	}
	*/
	
	bool WinRT_Window::updateOneFrame()
	{
		static grect drawRect(0.0f, 0.0f, 1.0f, 1.0f);
		static grect srcRect(0.0f, 0.0f, 1.0f, 1.0f);
		static grect viewport(0.0f, 0.0f, 1.0f, 1.0f);
		this->checkEvents();
		if (WinRT::View->isFilled() || WinRT::View->isSnapped())
		{
			if (!this->hasStoredProjectionMatrix)
			{
				this->storedProjectionMatrix = april::rendersys->getProjectionMatrix();
				this->hasStoredProjectionMatrix = true;
			}
			this->_tryLoadLogoTexture();
			april::rendersys->clear();
			viewport.setSize((float)this->width, (float)this->height);
			april::rendersys->setOrthoProjection(viewport);
			april::rendersys->drawFilledRect(viewport, this->backgroundColor);
			if (this->logoTexture != NULL)
			{
				drawRect.set(0.0f, (float)((this->height - this->logoTexture->getHeight()) / 2),
					(float)this->logoTexture->getWidth(), (float)this->logoTexture->getHeight());
				april::rendersys->setTexture(this->logoTexture);
				if (WinRT::View->isFilled())
				{
					// render texture in center
					drawRect.x = (float)((this->width - SNAP_VIEW_WIDTH - this->logoTexture->getWidth()) / 2);
				}
				if (WinRT::View->isSnapped())
				{
					// render texture twice on each side of the snapped view
					drawRect.x = (float)((SNAP_VIEW_WIDTH - this->logoTexture->getWidth()) / 2);
				}
				april::rendersys->drawTexturedRect(drawRect, srcRect);
			}
			april::rendersys->presentFrame();
			return this->running;
		}
		if (this->hasStoredProjectionMatrix)
		{
			april::rendersys->setProjectionMatrix(this->storedProjectionMatrix);
			this->hasStoredProjectionMatrix = false;
		}
		return Window::updateOneFrame();
	}
	
	void WinRT_Window::presentFrame()
	{
	}
	
	void WinRT_Window::checkEvents()
	{
		april::WinRT::View->checkEvents();
		while (this->mouseEvents.size() > 0)
		{
			MouseInputEvent e = this->mouseEvents.pop_first();
			if (e.type != AMOUSEEVT_SCROLL)
			{
				this->cursorPosition = e.position;
			}
			Window::handleMouseEvent(e.type, e.position, e.button);
		}
		while (this->keyEvents.size() > 0)
		{
			KeyInputEvent e = this->keyEvents.pop_first();
			Window::handleKeyEvent(e.type, e.keyCode, e.charCode);
		}
		while (this->touchEvents.size() > 0)
		{
			TouchInputEvent e = this->touchEvents.pop_first();
			Window::handleTouchEvent(e.touches);
		}
	}
	
	void WinRT_Window::handleTouchEvent(MouseEventType type, gvec2 position, int index)
	{
		int previousTouchesSize = this->touches.size();
		switch (type)
		{
		case AMOUSEEVT_DOWN:
			if (index < this->touches.size()) // DOWN event of an already indexed touch, never happened so far
			{
				return;
			}
			this->touches += position;
			break;
		case AMOUSEEVT_UP:
			if (index >= this->touches.size()) // redundant UP event, can happen
			{
				return;
			}
			this->touches.remove_at(index);
			break;
		case AMOUSEEVT_MOVE:
			if (index >= this->touches.size()) // MOVE event of an unindexed touch, never happened so far
			{
				return;
			}
			this->touches[index] = position;
			break;
		}
		if (this->multiTouchActive || this->touches.size() > 1)
		{
			if (!this->multiTouchActive && previousTouchesSize == 1)
			{
				// cancel (notify the app) the previously called mousedown event so we can begin the multi touch event properly
				this->handleMouseEvent(AMOUSEEVT_UP, gvec2(-10000.0f, -10000.0f), AMOUSEBTN_LEFT);
			}
			this->multiTouchActive = (this->touches.size() > 0);
		}
		else
		{
			this->handleMouseEvent(type, position, AMOUSEBTN_LEFT);
		}
		this->touchEvents.clear();
		this->touchEvents += TouchInputEvent(this->touches);
	}
	
	void WinRT_Window::handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button)
	{
		this->mouseEvents += MouseInputEvent(type, position, button);
	}
	
	void WinRT_Window::handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode)
	{
		this->keyEvents += KeyInputEvent(type, keyCode, charCode);
	}
	
	void WinRT_Window::_tryLoadLogoTexture()
	{
		if (this->logoTexture != NULL)
		{
			return;
		}
		if (!hfile::exists(MANIFEST_FILENAME))
		{
			return;
		}
		hstr data = hfile::hread(MANIFEST_FILENAME); // lets hope Microsoft does not change the format of these
		// locating the right entry in XML
		int index = data.find("<Applications>");
		if (index < 0)
		{
			return;
		}
		data = data(index, data.size() - index);
		index = data.find("<Application ");
		if (index < 0)
		{
			return;
		}
		data = data(index, data.size() - index);
		index = data.find("<VisualElements ");
		if (index < 0)
		{
			return;
		}
		// finding the logo entry in XML
		data = data(index, data.size() - index);
		int logoIndex = data.find("Logo=\"");
		if (logoIndex >= 0)
		{
			index = logoIndex + hstr("Logo=\"").size();
			hstr logoFilename = data(index, data.size() - index);
			index = logoFilename.find("\"");
			if (index >= 0)
			{
				// loading the logo file
				logoFilename = logoFilename(0, index);
				this->logoTexture = april::rendersys->loadTexture(logoFilename, true);
				if (this->logoTexture != NULL)
				{
					try
					{
						this->logoTexture->load();
					}
					catch (hltypes::exception&)
					{
						delete this->logoTexture;
						this->logoTexture = NULL;
					}
				}
			}
		}
		// finding the color entry in XML
		int colorIndex = data.find("BackgroundColor=\"");
		if (colorIndex >= 0)
		{
			index = colorIndex + hstr("BackgroundColor=\"").size();
			hstr colorString = data(index, data.size() - index);
			index = colorString.find("\"");
			if (index >= 0)
			{
				// loading the color string
				colorString = colorString(0, index).ltrim('#');
				if (colorString.size() >= 6)
				{
					if (colorString.size() > 6)
					{
						colorString = colorString(0, 6);
					}
					this->backgroundColor.set(colorString);
				}
			}
		}
	}

}
#endif
