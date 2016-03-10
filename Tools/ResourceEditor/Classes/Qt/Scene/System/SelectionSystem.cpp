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


#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"

ENUM_DECLARE(SelectionSystemDrawMode)
{
    ENUM_ADD(SS_DRAW_SHAPE);
    ENUM_ADD(SS_DRAW_CORNERS);
    ENUM_ADD(SS_DRAW_BOX);
    ENUM_ADD(SS_DRAW_NO_DEEP_TEST);
}

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene* scene, SceneCollisionSystem* collSys, HoodSystem* hoodSys)
    : DAVA::SceneSystem(scene)
    , collisionSystem(collSys)
    , hoodSystem(hoodSys)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

SceneSelectionSystem::~SceneSelectionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
    }
}

void SceneSelectionSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    if ((EventSystem::SWITCH_CHANGED == event) && currentSelection.ContainsEntity(component->GetEntity()))
    {
        invalidSelectionBoxes = true;
    }
}

void SceneSelectionSystem::UpdateGroupSelectionMode()
{
    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();

    bool addSelection = keyboard.IsKeyPressed(DAVA::Key::LCTRL) || keyboard.IsKeyPressed(DAVA::Key::RCTRL);
    bool excludeSelection = keyboard.IsKeyPressed(DAVA::Key::LALT) || keyboard.IsKeyPressed(DAVA::Key::RALT);

    if (addSelection)
    {
        groupSelectionMode = GroupSelectionMode::Add;
    }
    else if (excludeSelection)
    {
        groupSelectionMode = GroupSelectionMode::Remove;
    }
    else
    {
        groupSelectionMode = GroupSelectionMode::Replace;
    }
}

void SceneSelectionSystem::Process(DAVA::float32 timeElapsed)
{
    ForceEmitSignals();

    if (IsLocked())
    {
        return;
    }

    // if boxes are invalid we should request them from collision system
    // and store them in selection entityGroup
    if (invalidSelectionBoxes)
    {
        for (auto& item : currentSelection.GetMutableContent())
        {
            item.second = GetUntransformedBoundingBox(item.first);
        }
        invalidSelectionBoxes = false;
    }

    UpdateGroupSelectionMode();
    UpdateHoodPos();
}

void SceneSelectionSystem::ForceEmitSignals()
{
    if (selectionHasChanges)
    {
        SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2*)GetScene(), &currentSelection, &recentlySelectedEntities);
        selectionHasChanges = false;
        recentlySelectedEntities.Clear();
    }
}

void SceneSelectionSystem::ProcessSelectedGroup(const EntityGroup::EntityVector& allObjects)
{
    EntityGroup::EntityVector collisionEntities;
    collisionEntities.reserve(allObjects.size());

    auto i = allObjects.begin();
    for (const auto& item : allObjects)
    {
        auto flags = i->first->GetAvailableComponentFlags();
        if (componentMaskForSelection & flags)
        {
            auto selectableEntity = GetSelectableEntity(item.first);
            collisionEntities.emplace_back(selectableEntity, GetUntransformedBoundingBox(selectableEntity));
        }
    }

    DAVA::Entity* firstEntity = collisionEntities.empty() ? nullptr : collisionEntities.front().first;
    DAVA::Entity* nextEntity = firstEntity;

    // sequent selection?
    if (SettingsManager::GetValue(Settings::Scene_SelectionSequent).AsBool() && (currentSelection.Size() <= 1))
    {
        // find first after currently selected items
        for (size_t i = 0, e = collisionEntities.size(); i < e; i++)
        {
            DAVA::Entity* entity = collisionEntities[i].first;
            if (currentSelection.ContainsEntity(entity))
            {
                if ((i + 1) < e)
                {
                    nextEntity = collisionEntities[i + 1].first;
                    break;
                }
            }
        }
    }

    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();

    bool addSelection = keyboard.IsKeyPressed(DAVA::Key::LCTRL) || keyboard.IsKeyPressed(DAVA::Key::RCTRL);
    bool excludeSelection = keyboard.IsKeyPressed(DAVA::Key::LALT) || keyboard.IsKeyPressed(DAVA::Key::RALT);

    if (addSelection)
    {
        AddEntityToSelection(firstEntity);
    }
    else if (excludeSelection)
    {
        ExcludeEntityFromSelection(firstEntity);
    }
    else
    {
        bool selectOnClick = SettingsManager::GetValue(Settings::Scene_SelectionOnClick).AsBool();
        // if new selection is NULL or is one of already selected items
        // we should change current selection only on phase end
        if (selectOnClick || (nextEntity == nullptr) || (currentSelection.IntersectedEntity(collisionEntities) != nullptr))
        {
            applyOnPhaseEnd = true;
            objectsToSelect.Clear();
            if (nextEntity != nullptr)
            {
                objectsToSelect.Add(nextEntity, GetUntransformedBoundingBox(nextEntity));
            }
        }
        else
        {
            EntityGroup newSelection(nextEntity, GetUntransformedBoundingBox(nextEntity));
            SetSelection(newSelection);
        }
    }
}

void SceneSelectionSystem::PerformSelectionAtPoint(const DAVA::Vector2& point)
{
    DAVA::Vector3 traceFrom;
    DAVA::Vector3 traceTo;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    if (cameraSystem->GetCurCamera() != nullptr)
    {
        cameraSystem->GetRayTo2dPoint(point, cameraSystem->GetCurCamera()->GetZFar(), traceFrom, traceTo);
    }
    ProcessSelectedGroup(collisionSystem->ObjectsRayTest(traceFrom, traceTo));
}

void SceneSelectionSystem::PerformSelectionInCurrentBox()
{
    UpdateGroupSelectionMode();

    const float32 minSelectionSize = 2.0f;

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if ((std::abs(selectionSize.x) < minSelectionSize) || (std::abs(selectionSize.y) < minSelectionSize))
    {
        return;
    };

    float minX = std::min(selectionStartPoint.x, selectionEndPoint.x);
    float minY = std::min(selectionStartPoint.y, selectionEndPoint.y);
    float maxX = std::max(selectionStartPoint.x, selectionEndPoint.x);
    float maxY = std::max(selectionStartPoint.y, selectionEndPoint.y);

    Vector3 p0;
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
    Vector3 p4;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    cameraSystem->GetRayTo2dPoint(Vector2(minX, minY), 1.0f, p0, p1);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, minY), 1.0f, p0, p2);
    cameraSystem->GetRayTo2dPoint(Vector2(minX, maxY), 1.0f, p0, p4);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, maxY), 1.0f, p0, p3);

    Plane planes[4];
    planes[0] = Plane(p2, p1, p0);
    planes[1] = Plane(p3, p2, p0);
    planes[2] = Plane(p4, p3, p0);
    planes[3] = Plane(p1, p4, p0);

    const EntityGroup& allSelectedObjects = collisionSystem->ClipObjectsToPlanes(planes, 4);

    EntityGroup selectedObjects;
    for (const auto& item : allSelectedObjects.GetContent())
    {
        if (componentMaskForSelection & item.first->GetAvailableComponentFlags())
        {
            auto selectableEntity = GetSelectableEntity(item.first);
            if (!selectableEntity->GetLocked())
            {
                selectedObjects.Add(selectableEntity, GetUntransformedBoundingBox(selectableEntity));
            }
        }
    }

    UpdateSelectionGroup(selectedObjects);
    applyOnPhaseEnd = true;
}

void SceneSelectionSystem::Input(DAVA::UIEvent* event)
{
    if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection) || (event->mouseButton != DAVA::UIEvent::MouseButton::LEFT))
    {
        return;
    }

    if (DAVA::UIEvent::Phase::BEGAN == event->phase)
    {
        for (auto selectionDelegate : selectionDelegates)
        {
            if (selectionDelegate->AllowPerformSelectionHavingCurrent(currentSelection) == false)
            {
                return;
            }
        }

        selecting = true;
        selectionStartPoint = event->point;
        selectionEndPoint = selectionStartPoint;
        lastGroupSelection.Clear();
        PerformSelectionAtPoint(selectionStartPoint);
    }
    else if (selecting && (DAVA::UIEvent::Phase::DRAG == event->phase))
    {
        selectionEndPoint = event->point;
        PerformSelectionInCurrentBox();
    }
    else if (DAVA::UIEvent::Phase::ENDED == event->phase)
    {
        if ((event->mouseButton == DAVA::UIEvent::MouseButton::LEFT) && applyOnPhaseEnd)
        {
            FinishSelection();
        }
        applyOnPhaseEnd = false;
        selecting = false;
    }
}

void SceneSelectionSystem::DrawItem(DAVA::Entity* entity, const DAVA::AABBox3& originalBox, DAVA::int32 drawMode,
                                    DAVA::RenderHelper::eDrawType wireDrawType, DAVA::RenderHelper::eDrawType solidDrawType, const DAVA::Color& color)
{
    DAVA::AABBox3 bbox;
    originalBox.GetTransformedBox(entity->GetWorldTransform(), bbox);

    // draw selection share
    if (drawMode & SS_DRAW_SHAPE)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, color, wireDrawType);
    }
    // draw selection share
    else if (drawMode & SS_DRAW_CORNERS)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxCorners(bbox, color, wireDrawType);
    }
    // fill selection shape
    if (drawMode & SS_DRAW_BOX)
    {
        DAVA::Color alphaScaledColor = color;
        alphaScaledColor.a *= 0.15f;
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, alphaScaledColor, solidDrawType);
    }
}

void SceneSelectionSystem::Draw()
{
    if (IsLocked())
    {
        return;
    }

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if (selecting && (selectionSize.Length() >= 1.0f))
    {
        DAVA::Rect targetRect(selectionStartPoint, selectionSize);
        RenderSystem2D::Instance()->FillRect(targetRect, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f / 3.0f));
        RenderSystem2D::Instance()->DrawRect(targetRect, DAVA::Color::White);
    }

    DAVA::int32 drawMode = SettingsManager::GetValue(Settings::Scene_SelectionDrawMode).AsInt32();

    DAVA::RenderHelper::eDrawType wireDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    DAVA::RenderHelper::DRAW_WIRE_DEPTH :
    DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH;

    DAVA::RenderHelper::eDrawType solidDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    DAVA::RenderHelper::DRAW_SOLID_DEPTH :
    DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH;

    bool replacingSelection = selecting && (groupSelectionMode == GroupSelectionMode::Replace);
    if (!replacingSelection)
    {
        for (const auto& item : currentSelection.GetContent())
        {
            DrawItem(item.first, item.second, drawMode, wireDrawType, solidDrawType, DAVA::Color::White);
        }
    }

    DAVA::Color drawColor = DAVA::Color::White;
    if (groupSelectionMode == GroupSelectionMode::Add)
    {
        drawColor = DAVA::Color(0.5f, 1.0f, 0.5f, 1.0f);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        drawColor = DAVA::Color(1.0f, 0.5f, 0.5f, 1.0f);
    }

    for (const auto& item : objectsToSelect.GetContent())
    {
        DrawItem(item.first, item.second, drawMode, wireDrawType, solidDrawType, drawColor);
    }
}

void SceneSelectionSystem::ProcessCommand(const Command2* command, bool redo)
{
    if (nullptr != command)
    {
        auto commandId = command->GetId();

        if ((CMDID_ENTITY_REMOVE == commandId))
        {
            // remove from selection entity that was removed by command
            ExcludeEntityFromSelection(command->GetEntity());
        }
        else if ((CMDID_ENTITY_CHANGE_PARENT == commandId) || (CMDID_TRANSFORM == commandId))
        {
            invalidSelectionBoxes = true;
        }
    }
}

void SceneSelectionSystem::SetSelection(EntityGroup& newSelection)
{
    if (IsLocked())
    {
        return;
    }

    newSelection.RemoveIf([this](Entity* entity)
                          {
                              return IsEntitySelectable(entity) == false;
                          });

    /*
	 * Ask delegates if selection could be changed
	 */
    selectionHasChanges = true;
    for (auto selectionDelegate : selectionDelegates)
    {
        if (selectionDelegate->AllowChangeSelectionReplacingCurrent(currentSelection, newSelection) == false)
        {
            selectionHasChanges = false;
            return;
        }
    }

    /*
	 * Actually change selection
	 */
    for (const auto& i : currentSelection.GetContent())
    {
        objectsToSelect.Remove(i.first);
        recentlySelectedEntities.Add(i.first, i.second);
    }
    currentSelection = newSelection;

    if (selectionHasChanges)
    {
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::AddEntityToSelection(DAVA::Entity* entity)
{
    if (!IsLocked() && IsEntitySelectable(entity) && !currentSelection.ContainsEntity(entity))
    {
        auto newSelection = currentSelection;
        newSelection.Add(entity, GetUntransformedBoundingBox(entity));
        SetSelection(newSelection);
    }
}

void SceneSelectionSystem::AddSelection(const EntityGroup& entities)
{
    if (IsLocked())
    {
        return;
    }

    EntityGroup newSelection = currentSelection;
    for (const auto& item : entities.GetContent())
    {
        if (IsEntitySelectable(item.first) && !currentSelection.ContainsEntity(item.first))
        {
            newSelection.Add(item.first, GetUntransformedBoundingBox(item.first));
        }
    }
    SetSelection(newSelection);
}

bool SceneSelectionSystem::IsEntitySelectable(DAVA::Entity* entity) const
{
    if (!IsLocked() && entity)
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }

    return false;
}

void SceneSelectionSystem::ExcludeSingleItem(DAVA::Entity* entity)
{
    auto newSelection = currentSelection;
    auto& selectionContent = newSelection.GetMutableContent();
    auto i = selectionContent.find(entity);
    if (i != selectionContent.end())
    {
        recentlySelectedEntities.Add(i->first, i->second);
        selectionContent.erase(i);
    }
    if (objectsToSelect.ContainsEntity(entity))
    {
        objectsToSelect.Remove(entity);
    }
    SetSelection(newSelection);
}

void SceneSelectionSystem::ExcludeEntityFromSelection(DAVA::Entity* entity)
{
    if (!IsLocked())
    {
        ExcludeSingleItem(entity);
        currentSelection.RebuildBoundingBox();
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::ExcludeSelection(const EntityGroup& entities)
{
    if (!IsLocked())
    {
        for (const auto& item : entities.GetContent())
        {
            ExcludeSingleItem(item.first);
        }
        currentSelection.RebuildBoundingBox();
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::Clear()
{
    if (!IsLocked())
    {
        auto allItems = currentSelection.CopyContentToVector();
        for (const auto& item : allItems)
        {
            ExcludeSingleItem(item.first);
        }
        currentSelection.RebuildBoundingBox();
        UpdateHoodPos();
    }
}

const EntityGroup& SceneSelectionSystem::GetSelection() const
{
    static const EntityGroup emptyGroup = EntityGroup();
    return IsLocked() == false ? currentSelection : emptyGroup;
}

size_t SceneSelectionSystem::GetSelectionCount() const
{
    return IsLocked() == false ? currentSelection.Size() : 0;
}

DAVA::Entity* SceneSelectionSystem::GetFirstSelectionEntity() const
{
    return IsLocked() == false ? currentSelection.GetFirstEntity() : nullptr;
}

bool SceneSelectionSystem::IsEntitySelected(Entity* entity)
{
    return IsLocked() == false ? currentSelection.ContainsEntity(entity) : false;
}

bool SceneSelectionSystem::IsEntitySelectedHierarchically(Entity* entity)
{
    if (IsLocked())
        return false;

    while (entity)
    {
        if (currentSelection.ContainsEntity(entity))
            return true;

        entity = entity->GetParent();
    }
    return false;
}

void SceneSelectionSystem::CancelSelection()
{
    // don't change selection on phase end
    applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(ST_PivotPoint pp)
{
    curPivotPoint = pp;
}

ST_PivotPoint SceneSelectionSystem::GetPivotPoint() const
{
    return curPivotPoint;
}

void SceneSelectionSystem::SetLocked(bool lock)
{
    bool lockChanged = IsLocked() != lock;
    SceneSystem::SetLocked(lock);

    hoodSystem->LockAxis(lock);
    hoodSystem->SetVisible(!lock);

    if (!lock)
    {
        UpdateHoodPos();
    }

    if (lockChanged)
    {
        EntityGroup emptyGroup;
        EntityGroup* selected = nullptr;
        EntityGroup* deselected = nullptr;
        if (lock == true)
        {
            selected = &emptyGroup;
            deselected = &currentSelection;
        }
        else
        {
            selected = &currentSelection;
            deselected = &emptyGroup;
        }

        SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2*)GetScene(), selected, deselected);
    }
}

void SceneSelectionSystem::UpdateHoodPos() const
{
    if (currentSelection.IsEmpty())
    {
        hoodSystem->LockModif(false);
        hoodSystem->SetVisible(false);
    }
    else
    {
        DAVA::Vector3 p;
        bool lockHoodModif = false;

        switch (curPivotPoint)
        {
        case ST_PIVOT_ENTITY_CENTER:
            p = currentSelection.GetAnyEntityTranslationVector();
            break;

        default:
            p = currentSelection.GetCommonTranslationVector();
            break;
        }

        // check if we have locked entities in selection group
        // if so - lock modification hood
        for (const auto& item : currentSelection.GetContent())
        {
            if (item.first->GetLocked())
            {
                lockHoodModif = true;
                break;
            }
        }

        hoodSystem->LockModif(lockHoodModif);
        hoodSystem->SetPosition(p);
        hoodSystem->SetVisible(true);
    }

    SceneEditor2* sc = (SceneEditor2*)GetScene();
    sc->cameraSystem->UpdateDistanceToCamera();
}

DAVA::Entity* SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
    DAVA::Entity* parent = entity;
    while (nullptr != parent)
    {
        if (parent->GetSolid())
        {
            entity = parent;
        }
        parent = parent->GetParent();
    }
    return entity;
}

DAVA::AABBox3 SceneSelectionSystem::GetUntransformedBoundingBox(DAVA::Entity* entity) const
{
    return GetTransformedBoundingBox(entity, DAVA::Matrix4::IDENTITY);
}

DAVA::AABBox3 SceneSelectionSystem::GetTransformedBoundingBox(DAVA::Entity* entity, const DAVA::Matrix4& transform) const
{
    DAVA::AABBox3 ret = DAVA::AABBox3(DAVA::Vector3(0, 0, 0), 0);

    if (nullptr != entity)
    {
        // we will get selection bbox from collision system
        DAVA::AABBox3 entityBox = collisionSystem->GetBoundingBox(entity);

        // add childs boxes into entity box
        for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
        {
            DAVA::Entity* childEntity = entity->GetChild(i);
            DAVA::AABBox3 childBox = GetTransformedBoundingBox(childEntity, childEntity->GetLocalTransform());

            if (entityBox.IsEmpty())
            {
                entityBox = childBox;
            }
            else
            {
                if (!childBox.IsEmpty())
                {
                    entityBox.AddAABBox(childBox);
                }
            }
        }

        // we should return box with specified transformation
        if (!entityBox.IsEmpty())
        {
            entityBox.GetTransformedBox(transform, ret);
        }
    }

    return ret;
}

DAVA::AABBox3 SceneSelectionSystem::GetTransformedBoundingBox(const EntityGroup& group) const
{
    DAVA::AABBox3 result;
    for (const auto& objects : group.GetContent())
    {
        DAVA::AABBox3 transformed;
        objects.second.GetTransformedBox(objects.first->GetWorldTransform(), transformed);
        result.AddAABBox(transformed);
    }
    return result.IsEmpty() ? DAVA::AABBox3(Vector3(0.0f, 0.0f, 0.0f), 1.0f) : result;
}

void SceneSelectionSystem::SetSelectionComponentMask(DAVA::uint64 mask)
{
    componentMaskForSelection = mask;

    if (currentSelection.IsEmpty())
    {
        selectionHasChanges = true; // magic to say to selectionModel() of scene tree to reset selection
    }
    else
    {
        Clear();
    }
}

void SceneSelectionSystem::Activate()
{
    SetLocked(false);
}

void SceneSelectionSystem::Deactivate()
{
    SetLocked(true);
}

void SceneSelectionSystem::UpdateSelectionGroup(const EntityGroup& newSelection)
{
    objectsToSelect.Exclude(lastGroupSelection);

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        objectsToSelect.Join(newSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        for (const auto& item : newSelection.GetContent())
        {
            if (!currentSelection.ContainsEntity(item.first))
            {
                objectsToSelect.Add(item.first, item.second);
            }
        }
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        for (const auto& item : newSelection.GetContent())
        {
            if (currentSelection.ContainsEntity(item.first))
            {
                objectsToSelect.Add(item.first, item.second);
            }
        }
    }

    lastGroupSelection = newSelection;
}

void SceneSelectionSystem::FinishSelection()
{
    EntityGroup newSelection;

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        newSelection.Join(objectsToSelect);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        newSelection.Join(objectsToSelect);
        newSelection.Join(currentSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        newSelection.Join(currentSelection);
        newSelection.Exclude(objectsToSelect);
    }
    else
    {
        DVASSERT_MSG(0, "Invalid selection mode");
    }
    objectsToSelect.Clear();

    SetSelection(newSelection);
}

void SceneSelectionSystem::AddSelectionDelegate(SceneSelectionSystemDelegate* delegate_)
{
    DVASSERT(std::find(selectionDelegates.begin(), selectionDelegates.end(), delegate_) == selectionDelegates.end());
    selectionDelegates.push_back(delegate_);
}

void SceneSelectionSystem::RemoveSelectionDelegate(SceneSelectionSystemDelegate* delegate_)
{
    auto i = std::remove(selectionDelegates.begin(), selectionDelegates.end(), delegate_);
    selectionDelegates.erase(i, selectionDelegates.end());
}
