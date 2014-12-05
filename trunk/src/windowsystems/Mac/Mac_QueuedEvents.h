/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_QUEUED_EVENTS_H
#define APRIL_MAC_QUEUED_EVENTS_H

#include "Mac_Window.h"

namespace april
{
    class QueuedEvent
    {
    public:
        Mac_Window* window;
        
        QueuedEvent(Mac_Window* wnd);
        virtual ~QueuedEvent();
        virtual void execute() = 0;
    };
    
    class WindowSizeChangedEvent : public QueuedEvent
    {
        int w, h;
        bool fullscreen;
    public:
        WindowSizeChangedEvent(Mac_Window* wnd, int w, int h, bool fullscreen);
        virtual void execute();
    };
    
    class FocusChangedEvent : public QueuedEvent
    {
        bool focused;
    public:
        FocusChangedEvent(Mac_Window* wnd, bool focused);
        virtual void execute();
    };

}

#endif
