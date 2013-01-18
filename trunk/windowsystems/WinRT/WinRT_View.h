/// @file
/// @author  Boris Mikic
/// @version 2.52
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT View.

#ifdef _WIN32
#ifndef APRIL_WINRT_VIEW_H
#define APRIL_WINRT_VIEW_H
#include <hltypes/hplatform.h>
#if _HL_WINRT
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include <windows.h>
#include <agile.h>

#include "Window.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

namespace april
{
	ref class WinRT_View : public IFrameworkView
	{
	public:
		virtual void Initialize(_In_ CoreApplicationView^ applicationView);
		virtual void Uninitialize();
		virtual void SetWindow(_In_ CoreWindow^ window);
		virtual void Load(_In_ Platform::String^ entryPoint);
		virtual void Run();
		
		bool isFilled() { return this->filled; }
		bool isSnapped() { return this->snapped; }
		CoreWindow^ getCoreWindow() { return this->window.Get(); }
		void setCursorVisible(bool value);
		
		void OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args);
		void OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args);
		void OnSuspend(_In_ Platform::Object^ sender, _In_ SuspendingEventArgs^ args);
		void OnResume(_In_ Platform::Object^ sender, _In_ Platform::Object^ args);
		void OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args);
		
		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);
		
		void checkEvents();
		
	private: // has to be private
		Platform::Agile<CoreWindow> window;
		bool scrollHorizontal;
		harray<unsigned int> pointerIds;
		bool filled;
		bool snapped;
		int mouseMoveMessagesCount;
		
		void _updateViewState();
		
	};
	
	class WinRT
	{
	public:
		~WinRT() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static WinRT_View^ View;
		
	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
#endif