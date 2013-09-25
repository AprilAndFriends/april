/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT_WINDOW) && !defined(_WINP8)
#include "pch.h"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "IWinRT.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_BaseApp.h"
#include "WinRT_XamlApp.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Media;

#define MANIFEST_FILENAME "AppxManifest.xml"
#define SNAP_VIEW_WIDTH 320 // as defined by Microsoft

namespace april
{
	WinRT_XamlApp::WinRT_XamlApp() : Application()
	{
		this->app = ref new WinRT_BaseApp();
		this->running = true;
		this->filled = false;
		this->snapped = false;
		this->logoTexture = NULL;
		this->hasStoredProjection = false;
		this->backgroundColor = april::Color::Black;
		this->initialized = false;
		this->Suspending += ref new SuspendingEventHandler(this->app, &WinRT_BaseApp::OnSuspend);
		this->Resuming += ref new EventHandler<Object^>(this->app, &WinRT_BaseApp::OnResume);
#ifdef _DEBUG
		this->UnhandledException += ref new UnhandledExceptionEventHandler([](Object^ sender, UnhandledExceptionEventArgs^ args)
		{
			hlog::error("FATAL", _HL_PSTR_TO_HSTR(args->Message));
		});
#endif
	}

	WinRT_XamlApp::~WinRT_XamlApp()
	{
		_HL_TRY_DELETE(this->logoTexture);
	}

	void WinRT_XamlApp::unassignWindow()
	{
		_HL_TRY_DELETE(this->logoTexture);
		this->hasStoredProjection = false;
		this->backgroundColor = april::Color::Black;
	}

	void WinRT_XamlApp::setCursorVisible(bool value)
	{
		Windows::UI::Xaml::Window::Current->CoreWindow->PointerCursor = (value ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr);
	}
	
	bool WinRT_XamlApp::canSuspendResume()
	{
		return (!this->snapped && !this->filled);
	}

	void WinRT_XamlApp::updateViewState()
	{
		bool newFilled = (ApplicationView::Value == ApplicationViewState::Filled);
		bool newSnapped = (ApplicationView::Value == ApplicationViewState::Snapped);
		if (this->filled != newFilled || this->snapped != newSnapped)
		{
			hlog::write(april::logTag, "Handling view change...");
			if (!newFilled && !newSnapped)
			{
				if (this->hasStoredProjection)
				{
					april::rendersys->setOrthoProjection(this->storedOrthoProjection);
					april::rendersys->setProjectionMatrix(this->storedProjectionMatrix);
					this->hasStoredProjection = false;
					april::window->handleFocusChangeEvent(true);
				}
			}
			else if (!this->hasStoredProjection)
			{
				this->storedOrthoProjection = april::rendersys->getOrthoProjection();
				this->storedProjectionMatrix = april::rendersys->getProjectionMatrix();
				this->hasStoredProjection = true;
				april::window->handleFocusChangeEvent(false);
			}
			this->snapped = newSnapped;
			this->filled = newFilled;
		}
	}

	void WinRT_XamlApp::checkEvents()
	{
	}

	void WinRT_XamlApp::showKeyboard()
	{
		WinRT::XamlOverlay->showKeyboard();
	}

	void WinRT_XamlApp::hideKeyboard()
	{
		WinRT::XamlOverlay->hideKeyboard();
	}

	void WinRT_XamlApp::Connect(int connectionId, Object^ target)
	{
	}

	void WinRT_XamlApp::OnLaunched(LaunchActivatedEventArgs^ args)
	{
		// don't repeat app initialization when already running
		if (args->PreviousExecutionState == ApplicationExecutionState::Running)
		{
			Windows::UI::Xaml::Window::Current->Activate();
			return;
		}
		WinRT::XamlOverlay = ref new WinRT_XamlOverlay();
		Windows::UI::Xaml::Window::Current->Content = WinRT::XamlOverlay;
		Windows::UI::Xaml::Window::Current->Activated += ref new WindowActivatedEventHandler(this, &WinRT_XamlApp::OnWindowActivationChanged);
		Windows::UI::Xaml::Window::Current->Activate();
		this->app->assignEvents(Windows::UI::Core::CoreWindow::GetForCurrentThread());
		this->setCursorVisible(true);
	}

	void WinRT_XamlApp::OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args)
	{
		if (!this->initialized)
		{
			hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
			hresource::setArchive("");
			(*WinRT::Init)(WinRT::Args);
			this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
			this->initialized = true;
		}
	}

	void WinRT_XamlApp::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running || april::window == NULL)
		{
			return;
		}
		// don't repeat initialization when already running
		this->updateViewState();
		if (!this->filled && !this->snapped)
		{
			this->running = april::window->updateOneFrame();
		}
		else
		{
			static grect drawRect(0.0f, 0.0f, 1.0f, 1.0f);
			static grect srcRect(0.0f, 0.0f, 1.0f, 1.0f);
			static grect viewport(0.0f, 0.0f, 1.0f, 1.0f);
			static int width = 0;
			static int height = 0;
			if (width == 0 || height == 0)
			{
				gvec2 resolution = april::getSystemInfo().displayResolution;
				width = hround(resolution.x);
				height = hround(resolution.y);
				viewport.setSize((float)width, (float)height);
			}
			this->_tryLoadLogoTexture();
			april::rendersys->clear();
			april::rendersys->setOrthoProjection(viewport);
			april::rendersys->drawFilledRect(viewport, this->backgroundColor);
			if (this->logoTexture != NULL)
			{
				drawRect.set(0.0f, (float)((height - this->logoTexture->getHeight()) / 2),
					(float)this->logoTexture->getWidth(), (float)this->logoTexture->getHeight());
				april::rendersys->setTexture(this->logoTexture);
				if (this->filled)
				{
					// render texture in center
					drawRect.x = (float)((width - SNAP_VIEW_WIDTH - this->logoTexture->getWidth()) / 2);
				}
				if (this->snapped)
				{
					// render texture twice on each side of the snapped view
					drawRect.x = (float)((SNAP_VIEW_WIDTH - this->logoTexture->getWidth()) / 2);
				}
				april::rendersys->drawTexturedRect(drawRect, srcRect);
			}
		}
		april::rendersys->presentFrame();
		if (!this->running)
		{
			(*WinRT::Destroy)();
			CompositionTarget::Rendering::remove(this->eventToken);
			hlog::warn(april::logTag, "Manual closing in WinRT apps should not be used!");
		}
	}

	void WinRT_XamlApp::_tryLoadLogoTexture()
	{
		if (april::rendersys == NULL)
		{
			return;
		}
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
		data = data(index, -1);
		index = data.find("<Application ");
		if (index < 0)
		{
			return;
		}
		data = data(index, -1);
		index = data.find("<VisualElements ");
		if (index < 0)
		{
			return;
		}
		// finding the logo entry in XML
		data = data(index, -1);
		int logoIndex = data.find("Logo=\"");
		if (logoIndex >= 0)
		{
			index = logoIndex + hstr("Logo=\"").size();
			hstr logoFilename = data(index, -1);
			index = logoFilename.find("\"");
			if (index >= 0)
			{
				logoFilename = logoFilename(0, index);
				harray<hstr> filenames;
				filenames += logoFilename(0, index);
				// adding that ".scale-100" thing here, because my prayers went unanswered and Microsoft decided to change the format after all
				index = logoFilename.rfind('.');
				filenames += logoFilename(0, index) + ".scale-100" + logoFilename(index, -1);
				foreach (hstr, it, filenames)
				{
					// loading the logo file
					this->logoTexture = april::rendersys->createTexture(logoFilename, false);
					if (this->logoTexture != NULL)
					{
						try
						{
							this->logoTexture->load();
							break;
						}
						catch (hltypes::exception&)
						{
							delete this->logoTexture;
							this->logoTexture = NULL;
						}
					}
				}
			}
		}
		// finding the color entry in XML
		int colorIndex = data.find("BackgroundColor=\"");
		if (colorIndex >= 0)
		{
			index = colorIndex + hstr("BackgroundColor=\"").size();
			hstr colorString = data(index, -1);
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
