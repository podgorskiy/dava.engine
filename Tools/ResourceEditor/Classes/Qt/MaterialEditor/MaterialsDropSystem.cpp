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


#include "MaterialsDropSystem.h"
#include "Commands2/MaterialAssignCommand.h"
#include "Scene/EntityGroup.h"

#include "Base/BaseTypes.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"


bool MaterialsDropSystem::EntityGroupHasMaterials(const EntityGroup *group, const bool recursive)
{
    if(!group) return false;
    
    const size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        bool hasMaterials = EntityHasMaterials(group->GetEntity(i), recursive);
        if(hasMaterials) return true;
    }
    
    return false;
}

bool MaterialsDropSystem::EntityHasMaterials(const DAVA::Entity * entity, const bool recursive)
{
    if(!entity) return false;
    
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);
    if(ro && ro->GetRenderBatchCount())
    {
        return true;
    }
    
    if(recursive)
    {
        const DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            bool hasMaterials = EntityHasMaterials(entity->GetChild(i), recursive);
            if(hasMaterials) return true;
        }
    }
    
    return false;
}

bool MaterialsDropSystem::DropMaterialToGroup(const EntityGroup *group, DAVA::NMaterial *material, const bool recursive)
{
    bool dropWasSuccessful = true;
    
    size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        dropWasSuccessful &= DropMaterialToEntity(group->GetEntity(i), material, recursive);
    }

    return dropWasSuccessful;
}


bool MaterialsDropSystem::DropMaterialToEntity(const DAVA::Entity *entity, DAVA::NMaterial *material, const bool recursive)
{
    bool dropWasSuccessful = true;
    
    DAVA::RenderObject * ro = DAVA::GetRenderObject(entity);
    if(ro)
    {
        DAVA::uint32 count = ro->GetRenderBatchCount();
        dropWasSuccessful = (count == 1);
        
        if(dropWasSuccessful)
        {
            DAVA::RenderBatch *rb = ro->GetRenderBatch(0);
            DAVA::NMaterial *oldMaterial = rb->GetMaterial();

            DVASSERT(oldMaterial);
            
            oldMaterial->SetParent(material);
        }
    }
    
    if(recursive)
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            dropWasSuccessful &= DropMaterialToEntity(entity->GetChild(i), material, recursive);
        }
    }
    
    return dropWasSuccessful;
}

DAVA::Vector<const DAVA::Entity *> MaterialsDropSystem::GetDropRejectedEntities(const EntityGroup *group, const bool recursive)
{
    DAVA::Vector<const DAVA::Entity *> rejectedEntities;
    
    size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        GetDropRejectedEntities(rejectedEntities, group->GetEntity(i), recursive);
    }
    
    return rejectedEntities;
}


DAVA::Vector<const DAVA::Entity *> MaterialsDropSystem::GetDropRejectedEntities(const DAVA::Entity *entity, const bool recursive)
{
    DAVA::Vector<const DAVA::Entity *> rejectedEntities;
    
    GetDropRejectedEntities(rejectedEntities, entity, recursive);
    
    return rejectedEntities;
}
    
void MaterialsDropSystem::GetDropRejectedEntities(DAVA::Vector<const DAVA::Entity *> &rejectedEntities, const DAVA::Entity *entity, const bool recursive)
{
    DAVA::RenderObject * ro = DAVA::GetRenderObject(entity);
    if(ro)
    {
        DAVA::uint32 count = ro->GetRenderBatchCount();
        if(count != 1)
        {
            rejectedEntities.push_back(entity);
        }
    }
    
    if(recursive)
    {
        DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            GetDropRejectedEntities(rejectedEntities, entity->GetChild(i), recursive);
        }
    }
}
