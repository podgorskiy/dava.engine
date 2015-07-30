/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "UI/UIScreenTransition.h"
#include "Render/RenderHelper.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/Image.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
Sprite * UIScreenTransition::renderTargetPrevScreen = 0;
Sprite * UIScreenTransition::renderTargetNextScreen = 0;

UIScreenTransition::UIScreenTransition()
{
    duration = 0.7f;
    interpolationFunc = Interpolation::GetFunction(Interpolation::EASY_IN_EASY_OUT);
    SetFillBorderOrder(UIScreen::FILL_BORDER_AFTER_DRAW);

}

UIScreenTransition::~UIScreenTransition()
{
}

void UIScreenTransition::CreateRenderTargets()
{
    if (renderTargetPrevScreen || renderTargetNextScreen)
    {
        Logger::FrameworkDebug("Render targets already created");
        return;
    }
    
    uint32 width = (uint32)VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx;
    uint32 height = (uint32)VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy;

    Texture * tex1 = Texture::CreateFBO(width, height, FORMAT_RGBA8888);
    Texture * tex2 = Texture::CreateFBO(width, height, FORMAT_RGBA8888);

    renderTargetPrevScreen = Sprite::CreateFromTexture(tex1, 0, 0, (float32)width, (float32)height, true);
    renderTargetNextScreen = Sprite::CreateFromTexture(tex2, 0, 0, (float32)width, (float32)height, true);

    SafeRelease(tex1);
    SafeRelease(tex2);
}

void UIScreenTransition::ReleaseRenderTargets()
{
    SafeRelease(renderTargetPrevScreen);
    SafeRelease(renderTargetNextScreen);
}

void UIScreenTransition::StartTransition(UIScreen * _prevScreen, UIScreen * _nextScreen)
{
    CreateRenderTargets();
    nextScreen = _nextScreen;
    prevScreen = _prevScreen;

    MakeScreenshot(renderTargetPrevScreen->GetTexture(), prevScreen);

    nextScreen->LoadGroup();
    nextScreen->SystemWillAppear();
    nextScreen->SystemUpdate(SystemTimer::FrameDelta());

    MakeScreenshot(renderTargetNextScreen->GetTexture(), nextScreen);
    
    currentTime = 0;
}

void UIScreenTransition::Update(float32 timeElapsed)
{
    currentTime += timeElapsed;
    normalizedTime = interpolationFunc(currentTime / duration);
    if (currentTime >= duration)
    {
        currentTime = duration;
        
        if (prevScreen)
        {
            if (prevScreen->IsOnScreen())
                prevScreen->SystemWillBecomeInvisible();
            prevScreen->SystemWillDisappear();
            if (prevScreen->GetGroupId() != nextScreen->GetGroupId())
                prevScreen->UnloadGroup();
            prevScreen->SystemDidDisappear();
            SafeRelease(prevScreen);
        }
        
        UIControlSystem::Instance()->ReplaceScreen(nextScreen);
        nextScreen->SystemDidAppear();
        if (nextScreen->IsOnScreen())
            nextScreen->SystemWillBecomeVisible();
        ReleaseRenderTargets();
        // go to next screen
        UIControlSystem::Instance()->UnlockInput();
        UIControlSystem::Instance()->UnlockSwitch();

        /*
            Right now we are in update so when we change control we miss Update for new screen
            Here we call update control to make calls to update / draw sequential and avoid problem with missing Update
            We pass current timeElapsed because we miss current frame time
            */
        nextScreen->SystemUpdate(timeElapsed); // 
    }
}

void UIScreenTransition::Draw(const UIGeometricData &geometricData)
{
    Sprite::DrawState drawState;
    drawState.SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);

    drawState.SetScale(0.5f, 1.0f);
    drawState.SetPosition(0, 0);

    RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState, Color::White);

    drawState.SetScale(0.5f, 1.0f);
    drawState.SetPosition((VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dx) / 2.0f, 0);

    RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState, Color::White);
}

void UIScreenTransition::SetDuration(float32 timeInSeconds)
{
    duration = timeInSeconds;
};

bool UIScreenTransition::IsLoadingTransition()
{
    return false;
}

void UIScreenTransition::MakeScreenshot(Texture* target, UIScreen* screen)
{
    static const int32 PRIORITY_SCREENSHOT_CLEAR_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 3;
    static const int32 PRIORITY_SCREENSHOT_3D_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 2;
    static const int32 PRIORITY_SCREENSHOT_2D_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 1;

    rhi::Viewport v;
    v.width = target->GetWidth();
    v.height = target->GetHeight();
    RenderHelper::CreateClearPass(target->handle, PRIORITY_SCREENSHOT_CLEAR_PASS, Color::Clear, v);
    
    if (screen)
    {
        UI3DView* view3d = FindFirst3dView(screen);
        int32 oldPriority = eDefaultPassPriority::PRIORITY_MAIN_3D;
        rhi::Handle oldTexture = rhi::InvalidHandle;
        if (view3d != nullptr && view3d->GetScene())
        {
            rhi::RenderPassConfig& config = view3d->GetScene()->GetMainPassConfig();
            oldPriority = config.priority;
            oldTexture = config.colorBuffer[0].texture;
            config.priority = PRIORITY_SCREENSHOT_3D_PASS;
            config.colorBuffer[0].texture = target->handle;
        }

        RenderSystem2D::Instance()->BeginRenderTargetPass(target, false, Color::Clear, PRIORITY_SCREENSHOT_2D_PASS);
        screen->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());
        RenderSystem2D::Instance()->EndRenderTargetPass();

        if (view3d != nullptr && view3d->GetScene())
        {
            rhi::RenderPassConfig& config = view3d->GetScene()->GetMainPassConfig();
            config.priority = oldPriority;
            config.colorBuffer[0].texture = oldTexture;
        }
    }
}

UI3DView* UIScreenTransition::FindFirst3dView(UIControl* control)
{
    List<UIControl*> processControls;
    processControls.push_back(control);
    while (!processControls.empty())
    {
        auto currentCtrl = processControls.front();
        processControls.pop_front();
        
        UI3DView* current3dView = dynamic_cast<UI3DView*>(currentCtrl);
        if (nullptr != current3dView)
        {
            return current3dView;
        }

        auto& children = currentCtrl->GetChildren();
        for (auto child : children)
        {
            processControls.push_back(child);
        }
    }
    return nullptr;
}

};

