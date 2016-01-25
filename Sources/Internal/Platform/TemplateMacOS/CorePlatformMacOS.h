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


#ifndef __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__
#define __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__

#include "DAVAEngine.h"
#include "Platform/TemplateMacOS/CoreMacOSPlatformBase.h"
#include "Functional/Signal.h"

namespace DAVA
{

class CoreMacOSPlatform : public CoreMacOSPlatformBase
{
public:
    eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    void Quit() override;

    virtual Vector2 GetMousePosition();
    
    // Signal is emitted when window has been miniaturized/deminiaturized or
    // when application has been hidden/unhidden.
    // Signal parameter meaning:
    //  - when true - application/window has been hidden/minimized
    //  - when false - application/window has been unhidden/restored
    Signal<bool> signalAppMinimizedRestored;

    void SetWindowMinimumSize(float32 width, float32 height) override;
    Vector2 GetWindowMinimumSize() const override;
    
private:
    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;
};

};

#endif // __DAVAENGINE_CORE_PLATFORM_MAC_OS_H__