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

#include "pch.h"
#include "WinRT_XamlInterface.xaml.h"
//#include "windowsystems/WinRT/WinRT_XamlInterface.g.h" // because it's an auto-generated file

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Markup;

namespace april
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class WinRT_XamlApp sealed : public Application, public IComponentConnector
    {
    public:
        WinRT_XamlApp();
        virtual void Connect(int connectionId, Object^ target);

		void setCursorVisible(bool value);

		void OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args);
		void OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args);
		void OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args);
		
    internal:
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
		Windows::Foundation::EventRegistrationToken renderEventToken;

		~WinRT_XamlApp();


		/*
        void PauseRequested() { if (m_updateState == UpdateEngineState::Dynamics) m_pauseRequested = true; };
        void PressComplete()  { if (m_updateState == UpdateEngineState::WaitingForPress) m_pressComplete = true; };
        void ResetGame();
        void SetBackground(unsigned int background);
        void CycleBackground();

#ifdef USE_STORE_SIMULATOR
        void ResetLicense();
#endif
		*/

		/*
    private:

        void OnSuspending(
            _In_ Object^ sender,
            _In_ Windows::ApplicationModel::SuspendingEventArgs^ args
            );

        void OnResuming(
            _In_ Object^ sender,
            _In_ Object^ args
            );

        void UpdateViewState();

        void OnWindowActivationChanged(
            _In_ Object^ sender,
            _In_ Windows::UI::Core::WindowActivatedEventArgs^ args
            );

        void OnLogicalDpiChanged(
            _In_ Object^ sender
            );

        void OnDisplayContentsInvalidated(
            _In_ Object^ sender
            );

        void OnVisibilityChanged(
            _In_ Windows::UI::Core::CoreWindow^ sender,
            _In_ Windows::UI::Core::VisibilityChangedEventArgs^ args
            );

        void OnRendering(
            _In_ Object^ sender,
            _In_ Object^ args
            );

        void OnLicenseChanged();
        void InitializeLicense();
        void InitializeLicenseCore();

        void InitializeGameState();
        void OnDeviceLost();
        void OnDeviceReset();
        void Update();
        void SetGameInfoOverlay(GameInfoOverlayState state);
        void SetAction (GameInfoOverlayCommand command);
        void ShowGameInfoOverlay();
        void HideGameInfoOverlay();
        void SetSnapped();
        void HideSnapped();

        Windows::Foundation::EventRegistrationToken         m_onRenderingEventToken;
        bool                                                m_pauseRequested;
        bool                                                m_pressComplete;
        bool                                                m_renderNeeded;
        bool                                                m_haveFocus;
        bool                                                m_visible;

        MainPage^                                           m_mainPage;
        MoveLookController^                                 m_controller;
        GameRenderer^                                       m_renderer;
        Simple3DGame^                                       m_game;

        UpdateEngineState                                   m_updateState;
        UpdateEngineState                                   m_updateStateNext;
        PressResultState                                    m_pressResult;
        GameInfoOverlayState                                m_gameInfoOverlayState;
        Windows::ApplicationModel::Store::LicenseInformation^ m_licenseInformation;
        Windows::ApplicationModel::Store::ListingInformation^ m_listingInformation;
#ifdef USE_STORE_SIMULATOR
        PersistentState^                                    m_licenseState;
        bool                                                m_isTrial;
#endif
		*/
    };
}
#endif
#endif
