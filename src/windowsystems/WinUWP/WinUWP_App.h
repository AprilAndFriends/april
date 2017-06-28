﻿/// @file
/// @version 4.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinUWP App.

#ifdef _WINUWP_WINDOW
#ifndef APRIL_WINUWP_APP_H
#define APRIL_WINUWP_APP_H

//#include <gtypes/Matrix4.h>
//#include <gtypes/Vector2.h>
//#include <hltypes/harray.h>

//#include "Color.h"
//#include "pch.h"
//#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
/*
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;
#ifdef _WINP8
using namespace Windows::Phone::UI::Input;
#endif
*/

namespace april
{
	class Texture;
	
	ref class WinUWP_App sealed : public Core::IFrameworkView
	{
	public:
		WinUWP_App();
		//virtual void Connect(int connectionId, Object^ target);

		// IFrameworkView methods.
		virtual void Initialize(Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

		void refreshCursor();
		
		/*
		void OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);
		void OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);
		*/

		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);

	protected:
		// Application lifecycle events
		void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);
		// CoreWindow events
		void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args);
		void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args);
		// DisplayInformation events
		void OnDpiChanged(DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(DisplayInformation^ sender, Platform::Object^ args);

		void _updateWindowSize(float width, float height);

	private:
		bool running;
		/*
		Texture* splashTexture;
		*/
		CoreCursor^ defaultCursor;
		/*
		Color backgroundColor;
		bool launched;
		bool activated;
		*/
		// TODOa - special temporary hack required for input blocking the first frame after focus has been returned / the window was reactivated
		bool firstFrameAfterActivateHack;
		bool scrollHorizontal;
		harray<unsigned int> pointerIds;
		int64_t startTime;
		april::Key currentButton;

		void _handleFocusChange(bool focused);
		gvec2 _transformPosition(float x, float y);
		void _resetTouches();
		/*
		void _tryRenderSplashTexture(int count = 1);
		april::Texture* _tryLoadTexture(chstr nodeName, chstr attributeName);
		void _tryLoadSplashTexture();
		void _tryLoadBackgroundColor();
		bool _findVisualElements(chstr nodeName, hstr& data, int& index);
		*/

	};

	ref class FrameworkViewSource sealed : IFrameworkViewSource
	{
	public:
		virtual IFrameworkView^ CreateView();

	};

}
#endif
#endif