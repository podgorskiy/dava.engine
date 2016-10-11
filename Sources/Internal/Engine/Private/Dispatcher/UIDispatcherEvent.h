#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Engine/EngineTypes.h"

namespace DAVA
{
namespace Private
{
struct UIDispatcherEvent final
{
    enum eType : int32
    {
        DUMMY = 0,
        RESIZE_WINDOW,
        CREATE_WINDOW,
        CLOSE_WINDOW,
        SET_TITLE,
        FUNCTOR,
        SET_CURSOR_CAPTURE,
        SET_CURSOR_VISIBLE,
    };

    struct ResizeEvent
    {
        float32 width;
        float32 height;
    };

    struct SetTitleEvent
    {
        const char8* title;
    };

    struct SetCursorCaptureEvent
    {
        eCursorCapture mode;
    };

    struct SetCursorVisibilityEvent
    {
        bool visible;
    };

    UIDispatcherEvent() = default;
    UIDispatcherEvent(eType type)
        : type(type)
    {
    }

    eType type = DUMMY;
    Function<void()> functor;
    union
    {
        ResizeEvent resizeEvent;
        SetTitleEvent setTitleEvent;
        SetCursorCaptureEvent setCursorCaptureEvent;
        SetCursorVisibilityEvent setCursorVisibilityEvent;
    };

    static UIDispatcherEvent CreateResizeEvent(float32 width, float32 height);
    static UIDispatcherEvent CreateCloseEvent();
    static UIDispatcherEvent CreateSetTitleEvent(const String& title);
    static UIDispatcherEvent CreateSetCursorCaptureEvent(eCursorCapture mode);
    static UIDispatcherEvent CreateSetCursorVisibilityEvent(bool visible);
    static UIDispatcherEvent CreateFunctorEvent(const Function<void()>& functor);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
