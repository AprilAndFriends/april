/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
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
#include "WinRT_XamlInterface.xaml.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;

namespace april
{
	class Texture;

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class WinRT_XamlApp sealed : public Application, public IComponentConnector
    {
    public:
        WinRT_XamlApp();
        virtual void Connect(int connectionId, Object^ target);

		void setCursorVisible(bool value);

		void updateViewState();
		void unassignWindow();
		
		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);

    internal:
		virtual void OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args);
		virtual void OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args);
		virtual void OnResume(_In_ Object^ sender, _In_ Object^ args);
		virtual void OnRender(_In_ Object^ sender, _In_ Object^ args);
        virtual void OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args);
		virtual void OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args);
		virtual void OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args);

	protected:
        virtual void OnLaunched(_In_ LaunchActivatedEventArgs^ args) override;

	private:
		bool running;
		bool scrollHorizontal;
		int mouseMoveMessagesCount;
		harray<unsigned int> pointerIds;
		bool filled;
		bool snapped;
		Texture* logoTexture;
		bool hasStoredProjectionMatrix;
		gmat4 storedProjectionMatrix;
		Color backgroundColor;
		bool initialized;

		~WinRT_XamlApp();

		gvec2 _translatePosition(float x, float y);
		void _tryLoadLogoTexture();

    };
}
#endif
#endif
