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

#include "WASDControllerSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

namespace DAVA
{
    
WASDControllerSystem::WASDControllerSystem(Scene * scene)
    : SceneSystem(scene)
    , moveSpeed(1.f)
{
}

WASDControllerSystem::~WASDControllerSystem()
{
}

void WASDControllerSystem::AddEntity(Entity * entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");
    
    entities.push_back(entity);
}

void WASDControllerSystem::RemoveEntity(Entity * entity)
{
    uint32 size = entities.size();
    for(uint32 i = 0; i < size; ++i)
    {
        if(entities[i] == entity)
        {
            entities[i] = entities[size-1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void WASDControllerSystem::Process(float32 timeElapsed)
{
    const uint32 size = entities.size();
    if(0 == size) return;
    
    Camera * camera = GetScene()->GetDrawCamera();
    if(!camera) return;
    
    Entity * cameraHolder = NULL;
    for(uint32 i = 0; i < size; ++i)
    {
        if(GetCamera(entities[i]) == camera)
        {
            cameraHolder = entities[i];
            break;
        }
    }

    DVASSERT(cameraHolder);

    WASDControllerComponent *wasdController = static_cast<WASDControllerComponent *>(cameraHolder->GetComponent(Component::WASD_CONTROLLER_COMPONENT));
    DVASSERT(wasdController);
    
    const float32 actualMoveSpeed = moveSpeed * timeElapsed;
    
    KeyboardDevice *keyboard = InputSystem::Instance()->GetKeyboard();
    if(keyboard->IsKeyPressed(DVKEY_UP) || keyboard->IsKeyPressed(DVKEY_W))
    {
        MoveForward(camera, actualMoveSpeed, false);
    }
    if(keyboard->IsKeyPressed(DAVA::DVKEY_DOWN) || keyboard->IsKeyPressed(DAVA::DVKEY_S))
    {
        MoveForward(camera, actualMoveSpeed, true);
    }
    if(keyboard->IsKeyPressed(DAVA::DVKEY_RIGHT) || keyboard->IsKeyPressed(DAVA::DVKEY_D))
    {
        MoveRight(camera, actualMoveSpeed, false);
    }
    if(keyboard->IsKeyPressed(DVKEY_LEFT) || keyboard->IsKeyPressed(DVKEY_A))
    {
        MoveRight(camera, actualMoveSpeed, true);
    }
}
    
void WASDControllerSystem::MoveForward(Camera *camera, float32 speed, bool inverseDirection)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();
    
    if(inverseDirection)
    {
        pos -= dir * speed;
    }
    else
    {
        pos += dir * speed;
    }
    
    camera->SetPosition(pos);
    camera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
}
    
void WASDControllerSystem::MoveRight(Camera *camera, float32 speed, bool inverseDirection)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();
    Vector3 left = camera->GetLeft();
    
    if(inverseDirection)
    {
        pos -= left * speed;
    }
    else
    {
        pos += left * speed;
    }
    
    camera->SetPosition(pos);
    camera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
}

    
};
