#ifndef __DAVEAENGINE_UI_SCREENSHOTER__
#define __DAVEAENGINE_UI_SCREENSHOTER__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/UIControl.h"

namespace DAVA
{
class UIControl;
class UI3DView;
class Texture;

/**
 * \brief Class for creating screenshot of UIControl to Texture
 */
class UIScreenshoter
{
public:
    /**
    * \brief Default c-tor
    */
    UIScreenshoter();

    /**
    * \brief D-tor
    */
    ~UIScreenshoter();

    /**
    * \brief Call each frame to control internal resources and render flow
    */
    void OnFrame();

    /**
     * \brief Render control to texture and return it immediately (but not rendered) 
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \return target texture
     */
    Texture* MakeScreenshot(UIControl* control, const PixelFormat format, bool clearAlpha = false);

    /**
     * \brief Render control to texture and call callback when it will rendered
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \param callback function which be called after render
     */
    void MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback);

    /**
     * \brief Render control to target texture
     * 
     * \param control pointer to source UIControl
     * \param screenshot pointer to target Texture
     */
    void MakeScreenshot(UIControl* control, Texture* screenshot);

    /**
    * \brief Render control to target texture
    *
    * \param control pointer to source UIControl
    * \param screenshot pointer to target Texture
    * \param callback function which be called after render
    */
    void MakeScreenshot(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback);

private:
    struct Control3dInfo
    {
        UI3DView* control = nullptr;
        rhi::RenderPassConfig scenePassConfig;
    };

    struct ScreenshotWaiter
    {
        Texture* texture = nullptr;
        rhi::HSyncObject syncObj;
        Function<void(Texture*)> callback;
    };

    void MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback, bool clearAlpha);

    List<ScreenshotWaiter> waiters;
};
};

#endif //__DAVEAENGINE_UI_SCREENSHOTER__