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


#include "Entity/SceneSystem.h"

namespace DAVA 
{
    
SceneSystem::SceneSystem(Scene * _scene)
:	requiredComponents(0),
	scene(_scene)
,	locked(false)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

SceneSystem::~SceneSystem()
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::SetRequiredComponents(uint32 _requiredComponents)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)

    requiredComponents = _requiredComponents;
}
    
uint32 SceneSystem::GetRequiredComponents()
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)

    return requiredComponents;
}

void SceneSystem::AddEntity(Entity * entity)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::RemoveEntity(Entity * entity)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}
    
void SceneSystem::SetParent(Entity *entity, Entity *parent)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}
    
void SceneSystem::SceneDidLoaded()
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::ImmediateEvent(Entity * entity, uint32 event)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::Process(float32 timeElapsed)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::SetLocked(bool locked)
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)

	this->locked = locked;
}

bool SceneSystem::IsLocked()
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)

	return locked;
}

void SceneSystem::AddComponent( Entity * entity, Component * component )
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

void SceneSystem::RemoveComponent( Entity * entity, Component * component )
{
    TAG_SWITCH(MemoryManager::TAG_SYSTEMS)
}

};
