/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include "pch.h"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
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
		this->running = true;
		this->scrollHorizontal = false;
		this->mouseMoveMessagesCount = 0;
		this->filled = false;
		this->snapped = false;
		this->logoTexture = NULL;
		this->hasStoredProjectionMatrix = false;
		this->backgroundColor = april::Color::Black;
		this->initialized = false;
		this->Suspending += ref new SuspendingEventHandler(this, &WinRT_XamlApp::OnSuspend);
		this->Resuming += ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnResume);
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

	void WinRT_XamlApp::setCursorVisible(bool value)
	{
		Windows::UI::Xaml::Window::Current->CoreWindow->PointerCursor = (value ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr);
	}
	
	void WinRT_XamlApp::updateViewState()
	{
		bool newFilled = (ApplicationView::Value == ApplicationViewState::Filled);
		if (!this->filled && newFilled)
		{
			hlog::write(april::logTag, "Handling filled view override...");
		}
		this->filled = newFilled;
		bool newSnapped = (ApplicationView::Value == ApplicationViewState::Snapped);
		if (!this->snapped && newSnapped)
		{
			hlog::write(april::logTag, "Handling snapped view override...");
		}
		this->snapped = newSnapped;
	}

	void WinRT_XamlApp::unassignWindow()
	{
		_HL_TRY_DELETE(this->logoTexture);
		this->hasStoredProjectionMatrix = false;
		this->backgroundColor = april::Color::Black;
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
		WinRT::Interface = ref new WinRT_XamlInterface();
		Windows::UI::Xaml::Window::Current->Content = WinRT::Interface;
		Windows::UI::Xaml::Window::Current->Activated += ref new WindowActivatedEventHandler(this, &WinRT_XamlApp::OnWindowActivationChanged);
		Windows::UI::Xaml::Window::Current->Activate();
		// events
		CoreWindow^ window = Windows::UI::Xaml::Window::Current->CoreWindow;
		window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
				this, &WinRT_XamlApp::OnWindowSizeChanged);
		window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
				this, &WinRT_XamlApp::OnVisibilityChanged);
		window->Closed +=
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(
				this, &WinRT_XamlApp::OnWindowClosed);
		window->PointerPressed +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_XamlApp::OnTouchDown);
		window->PointerReleased +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_XamlApp::OnTouchUp);
		window->PointerMoved +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_XamlApp::OnTouchMove);
		window->PointerWheelChanged +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_XamlApp::OnMouseScroll);
		window->KeyDown +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_XamlApp::OnKeyDown);
		window->KeyUp +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_XamlApp::OnKeyUp);
		window->CharacterReceived +=
			ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(
				this, &WinRT_XamlApp::OnCharacterReceived);
		this->setCursorVisible(true);
	}

	void WinRT_XamlApp::OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args)
	{
		if (!this->initialized)
		{
			hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
			hresource::setArchive("");
			(*WinRT::Init)(WinRT::Args);
			CompositionTarget::Rendering += ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender);
			this->initialized = true;
		}
	}

	void WinRT_XamlApp::OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		if (!this->snapped && !this->filled)
		{
			april::window->handleFocusChangeEvent(false);
		}
	}

	void WinRT_XamlApp::OnResume(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->snapped && !this->filled)
		{
			april::window->handleFocusChangeEvent(true);
		}
	}

	void WinRT_XamlApp::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running)
		{
			return;
		}
		// don't repeat initialization when already running
		this->updateViewState();
		if (!this->filled && !this->snapped)
		{
			if (this->hasStoredProjectionMatrix)
			{
				april::rendersys->setProjectionMatrix(this->storedProjectionMatrix);
				this->hasStoredProjectionMatrix = false;
				april::window->handleFocusChangeEvent(true);
			}
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
			}
			if (!this->hasStoredProjectionMatrix)
			{
				this->storedProjectionMatrix = april::rendersys->getProjectionMatrix();
				this->hasStoredProjectionMatrix = true;
				april::window->handleFocusChangeEvent(false);
			}
			this->_tryLoadLogoTexture();
			april::rendersys->clear();
			viewport.setSize((float)width, (float)height);
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
		}
	}

	void WinRT_XamlApp::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		this->updateViewState();
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		this->updateViewState();
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		april::window->handleQuitRequest(false);
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_DOWN, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_DOWN, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_UP, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			else
			{
				this->pointerIds.remove_at(index);
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_UP, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			this->mouseMoveMessagesCount++;
			if (this->mouseMoveMessagesCount >= 10)
			{
				april::window->setInputMode(april::Window::MOUSE);
			}
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_MOVE, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_MOVE, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}
	
	gvec2 WinRT_XamlApp::_translatePosition(float x, float y)
	{
		static int w = 0;
		static int h = 0;
		if (w == 0 || h == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			w = hround(resolution.x);
			h = hround(resolution.y);
		}
		int width = april::window->getWidth();
		int height = april::window->getHeight();
		if (w == width && h == height)
		{
			return gvec2(x, y);
		}
		return gvec2((float)(int)(x * width / w), (float)(int)(y * height / h));
	}

	void WinRT_XamlApp::_tryLoadLogoTexture()
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
