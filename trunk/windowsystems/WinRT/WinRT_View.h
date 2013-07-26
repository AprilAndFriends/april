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
/// Defines a WinRT View.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_VIEW_H
#define APRIL_WINRT_VIEW_H

#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include <agile.h>

#include "Window.h"
#include "WinRT_XamlApp.h"
#include "WinRT_XamlInterface.xaml.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

namespace april
{
	/*
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
		
		void checkEvents();
		void updateViewState();
		
	private: // has to be private
		bool filled;
		bool snapped;
		
	};
	*/
	
	class WinRT
	{
	public:
		~WinRT() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static WinRT_XamlApp^ App;
		static WinRT_XamlInterface^ Interface;
		
	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
