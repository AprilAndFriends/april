/// @file
/// @author  Boris Mikic
/// @version 3.12
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
#include "WinRT_Window.h"
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

namespace april
{
	WinRT_XamlApp::WinRT_XamlApp() : Application()
	{
		this->app = ref new WinRT_BaseApp();
		this->running = true;
		this->filled = false;
		this->snapped = false;
		this->logoTexture = NULL;
		this->hasStoredViewData = false;
		this->storedCursorVisible = false;
		this->backgroundColor = april::Color::Black;
		this->launched = false;
		this->activated = false;
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
		this->hasStoredViewData = false;
		this->storedCursorVisible = false;
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
		bool allowFilledView = (april::window->getParam(WINRT_ALLOW_FILLED_VIEW) != "0");
		bool newFilled = (ApplicationView::Value == ApplicationViewState::Filled && !allowFilledView);
		bool newSnapped = (ApplicationView::Value == ApplicationViewState::Snapped);
		if (this->filled != newFilled || this->snapped != newSnapped)
		{
			hlog::write(april::logTag, "Handling view change...");
			if (!newFilled && !newSnapped)
			{
				if (this->hasStoredViewData)
				{
					april::rendersys->setOrthoProjection(this->storedOrthoProjection);
					april::rendersys->setProjectionMatrix(this->storedProjectionMatrix);
					april::window->setCursorVisible(this->storedCursorVisible);
					this->hasStoredViewData = false;
					Windows::UI::Xaml::Window::Current->Content = WinRT::XamlOverlay;
					april::window->handleFocusChangeEvent(true);
				}
			}
			else if (!this->hasStoredViewData)
			{
				this->storedOrthoProjection = april::rendersys->getOrthoProjection();
				this->storedProjectionMatrix = april::rendersys->getProjectionMatrix();
				this->storedCursorVisible = april::window->isCursorVisible();
				april::window->setCursorVisible(true);
				this->hasStoredViewData = true;
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
		// don't repeat app initialization when already launched
		if (!this->launched)
		{
			this->launched = true;
			this->app->assignEvents(Windows::UI::Core::CoreWindow::GetForCurrentThread());
			this->setCursorVisible(true);
			WinRT::XamlOverlay = ref new WinRT_XamlOverlay();
			Windows::UI::Xaml::Window::Current->Content = WinRT::XamlOverlay;
			Windows::UI::Xaml::Window::Current->Activated += ref new WindowActivatedEventHandler(this, &WinRT_XamlApp::OnWindowActivationChanged);
			hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
			hresource::setArchive("");
			(*WinRT::Init)(WinRT::Args);
		}
		Windows::UI::Xaml::Window::Current->Activate();
	}

	void WinRT_XamlApp::OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args)
	{
		if (!this->activated)
		{
			this->activated = true;
			this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
			if (april::rendersys != NULL)
			{
				april::rendersys->presentFrame();
				april::rendersys->reset();
				april::rendersys->clear();
			}
		}
		else if (args->WindowActivationState == CoreWindowActivationState::Deactivated)
		{
			this->app->handleFocusChange(false);
			CompositionTarget::Rendering::remove(this->eventToken);
			this->eventToken.Value = 0;
		}
		else if (args->WindowActivationState == CoreWindowActivationState::CodeActivated
			|| args->WindowActivationState == CoreWindowActivationState::PointerActivated)
		{
			this->app->handleFocusChange(true);
			if (this->eventToken.Value == 0)
			{
				this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
			}
		}
	}

	void WinRT_XamlApp::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running || april::window == NULL)
		{
			return;
		}
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
			static bool useCustomSnappedView = false;
			static int width = 0;
			static int height = 0;
			useCustomSnappedView = (april::window->getParam(WINRT_USE_CUSTOM_SNAPPED_VIEW) != "0");
			width = april::window->getWidth();
			height = april::window->getHeight();
			viewport.setSize((float)width, (float)height);
			if (!useCustomSnappedView)
			{
				this->_tryLoadLogoTexture();
			}
			april::rendersys->clear();
			april::rendersys->setOrthoProjection(viewport);
			if (!useCustomSnappedView)
			{
				april::rendersys->drawFilledRect(viewport, this->backgroundColor);
				if (this->logoTexture != NULL)
				{
					drawRect.set((float)((width - this->logoTexture->getWidth()) / 2), (float)((height - this->logoTexture->getHeight()) / 2),
						(float)this->logoTexture->getWidth(), (float)this->logoTexture->getHeight());
					april::rendersys->setTexture(this->logoTexture);
					april::rendersys->drawTexturedRect(drawRect, srcRect);
				}
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
