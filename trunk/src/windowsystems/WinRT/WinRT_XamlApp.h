/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT XAML App.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_XAML_APP_H
#define APRIL_WINRT_XAML_APP_H

#include <gtypes/Matrix4.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>

#include "Color.h"
#include "pch.h"
#include "WinRT_XamlOverlay.xaml.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;

namespace april
{
	class Texture;
	
	[Windows::Foundation::Metadata::WebHostHidden]
	ref class WinRT_XamlApp sealed : public Application, public IComponentConnector
	{
	public:
		WinRT_XamlApp();
		virtual void Connect(int connectionId, Object^ target);
		
		void refreshCursor();
		
		property WinRT_XamlOverlay^ Overlay { WinRT_XamlOverlay^ get() { return this->overlay; } }

		void OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args);
		void OnResume(_In_ Object^ sender, _In_ Object^ args);
		void OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args);
		void OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args);
		void OnOrientationChanged(_In_ DisplayInformation^ sender, _In_ Object^ args);
		void OnDpiChanged(_In_ DisplayInformation^ sender, _In_ Object^ args);
		void OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);
		void OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args);

		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);

	internal:
		void OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args);
		void OnRender(_In_ Object^ sender, _In_ Object^ args);
		
	protected:
		virtual void OnLaunched(_In_ LaunchActivatedEventArgs^ args) override;
		
	private:
		bool running;
		Texture* splashTexture;
		WinRT_XamlOverlay^ overlay;
		CoreCursor^ defaultCursor;
		Color backgroundColor;
		bool launched;
		bool activated;
		Windows::Foundation::EventRegistrationToken eventToken;
		bool scrollHorizontal;
		harray<unsigned int> pointerIds;
		unsigned int startTime;
		april::Key currentButton;

		~WinRT_XamlApp();
		
		void _handleFocusChange(bool focused);
		void _refreshCursor();
		gvec2 _transformPosition(float x, float y);
		void _resetTouches();
		void _tryRenderSplashTexture();
		april::Texture* _tryLoadTexture(chstr nodeName, chstr attributeName);
		void _tryLoadSplashTexture();
		void _tryLoadBackgroundColor();
		bool _findVisualElements(chstr nodeName, hstr& data, int& index);

	};

}
#endif
#endif
