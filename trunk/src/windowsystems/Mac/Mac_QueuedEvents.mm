/// @file
/// @version 3.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
#include "Mac_QueuedEvents.h"

namespace april
{
    QueuedEvent::QueuedEvent(Mac_Window* wnd)
    {
        this->window = wnd;
    }
    
    QueuedEvent::~QueuedEvent()
    {
    }

    
    WindowSizeChangedEvent::WindowSizeChangedEvent(Mac_Window* wnd, int w, int h, bool fullscreen) : QueuedEvent(wnd)
    {
        this->w = w;
        this->h = h;
        this->fullscreen = fullscreen;
    }
    
    void WindowSizeChangedEvent::execute()
    {
        this->window->dispatchWindowSizeChanged(this->w, this->h, this->fullscreen);
    }
    
    FocusChangedEvent::FocusChangedEvent(Mac_Window* wnd, bool focused) : QueuedEvent(wnd)
    {
        this->focused = focused;
    }
    
    void FocusChangedEvent::execute()
    {
        aprilWindow->handleFocusChangeEvent(this->focused);
    }
}
