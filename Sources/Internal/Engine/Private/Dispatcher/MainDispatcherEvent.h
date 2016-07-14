#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
struct MainDispatcherEvent final
{
    enum eType : int32
    {
        DUMMY = 0,
        WINDOW_CREATED,
        WINDOW_DESTROYED,
        WINDOW_FOCUS_CHANGED,
        WINDOW_VISIBILITY_CHANGED,
        WINDOW_SIZE_SCALE_CHANGED,

        MOUSE_BUTTON_DOWN,
        MOUSE_BUTTON_UP,
        MOUSE_WHEEL,
        MOUSE_MOVE,

        TOUCH_DOWN,
        TOUCH_UP,
        TOUCH_MOVE,

        KEY_DOWN,
        KEY_UP,
        KEY_CHAR,

        FUNCTOR,

        APP_SUSPENDED,
        APP_RESUMED,

        APP_TERMINATE,
    };

    struct WindowStateEvent
    {
        uint32 state;
    };

    struct WindowSizeEvent
    {
        float32 width;
        float32 height;
        float32 scaleX;
        float32 scaleY;
    };

    struct WindowCreatedEvent
    {
        WindowBackend* windowBackend;
        WindowSizeEvent size;
    };

    struct MouseClickEvent
    {
        uint32 button;
        uint32 clicks;
        float32 x;
        float32 y;
    };

    struct MouseWheelEvent
    {
        float32 x;
        float32 y;
        int32 delta;
    };

    struct MouseMoveEvent
    {
        float32 x;
        float32 y;
    };

    struct TouchClickEvent
    {
        uint32 touchId;
        float32 x;
        float32 y;
    };

    struct TouchMoveEvent
    {
        uint32 touchId;
        float32 x;
        float32 y;
    };

    struct KeyEvent
    {
        uint32 key;
        bool isRepeated;
    };

    eType type = DUMMY;
    uint64 timestamp = 0;
    Window* window = nullptr;
    Function<void()> functor;
    union
    {
        WindowCreatedEvent windowCreatedEvent;
        WindowStateEvent stateEvent;
        WindowSizeEvent sizeEvent;
        MouseClickEvent mclickEvent;
        MouseWheelEvent mwheelEvent;
        MouseMoveEvent mmoveEvent;
        TouchClickEvent tclickEvent;
        TouchMoveEvent tmoveEvent;
        KeyEvent keyEvent;
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
