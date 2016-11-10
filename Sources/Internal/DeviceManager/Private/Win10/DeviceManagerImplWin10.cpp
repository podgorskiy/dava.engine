#include "DeviceManager/Private/Win10/DeviceManagerImplWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "DeviceManager/DeviceManager.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

namespace DAVA
{
namespace Private
{
// Windows Universal Application is an "outstanding" platform where Microsoft tries to hide as much API as possible.
// Application cannot enumerate all available displays, it only has access to current display window is on.
// Also DisplayInformation provides display size only early on application start, further calls will return
// only window dimension. As a result DeviceManagerImpl reads all display information one time on initialization,
// and when display properties change (only DisplayInformation::LogicalDpi are supported) update only dpi
// providing cached display size.

DeviceManagerImpl::DeviceManagerImpl(DeviceManager* devManager, Private::MainDispatcher* dispatcher)
    : deviceManager(devManager)
    , mainDispatcher(dispatcher)
{
    using ::Windows::UI::ViewManagement::ApplicationView;
    using ::Windows::Graphics::Display::DisplayInformation;

    ApplicationView ^ applicationView = ApplicationView::GetForCurrentView();
    DisplayInformation ^ displayInformation = DisplayInformation::GetForCurrentView();

    ::Windows::Foundation::Rect displayRect = applicationView->VisibleBounds;
    float32 rawPixelsPerViewPixel = static_cast<float32>(displayInformation->RawPixelsPerViewPixel);

    displayInfo.systemId = 0;
    displayInfo.name = "display"; // Any name as DisplayInformation does not provide display name
    displayInfo.dpiX = displayInformation->LogicalDpi;
    displayInfo.dpiY = displayInformation->LogicalDpi;
    displayInfo.rect.x = displayRect.X;
    displayInfo.rect.y = displayRect.Y;
    displayInfo.rect.dx = displayRect.Width * rawPixelsPerViewPixel;
    displayInfo.rect.dy = displayRect.Height * rawPixelsPerViewPixel;

    deviceManager->displays.push_back(displayInfo);
}

void DeviceManagerImpl::UpdateDisplayConfig()
{
    using ::Windows::UI::ViewManagement::ApplicationView;
    using ::Windows::Graphics::Display::DisplayInformation;

    DisplayInformation ^ displayInformation = DisplayInformation::GetForCurrentView();

    DisplayInfo* di = new DisplayInfo[1];
    *di = displayInfo;

    di->dpiX = displayInformation->LogicalDpi;
    di->dpiY = displayInformation->LogicalDpi;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateDisplayConfigChangedEvent(di, 1));
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
