//
//  ParticlesEditorSceneHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "ParticlesEditorSceneHelper.h"
#include "ParticleEditor/ParticlesEditorController.h"

#include "Commands/SceneGraphCommands.h"
#include "Commands/ParticleEditorCommands.h"

using namespace DAVA;

// Custom processing the Selection Changed in the Scene Graph model. Returns
// TRUE if no further processing needed.
bool ParticlesEditorSceneHelper::ProcessSelectionChanged(const QItemSelection &selected,
                                                         const QItemSelection &deselected)
{
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();

    if (selectedSize <= 0)
    {
        // De-selection happened - allow processing by caller.
        ParticlesEditorController::Instance()->CleanupSelectedNode();
        return false;
    }
    
    // Determine whether the selected node belongs to Particle Editor.
    QItemSelectionRange selectedRange = selected.at(0);
    SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());

    if (ParticlesEditorController::Instance()->IsBelongToParticlesEditor(selectedItem))
    {
        // Our one. Select and emit the appropriate event.
        ParticlesEditorController::Instance()->SetSelectedNode(selectedItem, true);
        return !ParticlesEditorController::Instance()->ShouldDisplayPropertiesInSceneEditor(selectedItem);
    }

    // Not ours. Cleanup the selected node and return control back to the caller.
    ParticlesEditorController::Instance()->CleanupSelectedNode();
    return false;
}

// Custom node selection.
SceneGraphItem* ParticlesEditorSceneHelper::GetGraphItemToBeSelected(GraphItem* rootItem, SceneNode* node)
{
    // Do we have something marked for selection on the Particles Editor? If yes,
    // do select it.
    return GetGraphItemToBeSelectedRecursive(rootItem, node);
    
    return false;
}

SceneGraphItem* ParticlesEditorSceneHelper::GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, SceneNode* node)
{
    SceneGraphItem* graphRootItem = dynamic_cast<SceneGraphItem*>(rootItem);
    if (!graphRootItem || graphRootItem->GetExtraUserData() == NULL)
    {
        return NULL;
    }
    
    BaseParticleEditorNode* editorNode = dynamic_cast<BaseParticleEditorNode*>(graphRootItem->GetExtraUserData());
    if (!editorNode)
    {
        return NULL;
    }
    
    if (editorNode->IsMarkedForSelection())
    {
        editorNode->SetMarkedToSelection(false);
        return graphRootItem;
    }
    
    // Verify all the child nodes for the Scene Graph.
    int32 childCount = rootItem->ChildrenCount();
    for (int32 i = 0; i < childCount; i ++)
    {
        SceneGraphItem* childGraphItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
        SceneGraphItem* graphItemToBeSelected = GetGraphItemToBeSelectedRecursive(childGraphItem, node);
        if (graphItemToBeSelected)
        {
            return graphItemToBeSelected;
        }
    }

    return NULL;
}

SceneNode* ParticlesEditorSceneHelper::PreprocessSceneNode(SceneNode* rawNode)
{
    // There is one and only preprocessing case - if the "raw" node is "orphaned" Particles Emitter
    // (without the ParticlesEffectNode parent), we'll create the new Particles Effect node and
    // move the raw Emitter node to it.
    ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(rawNode);
    if (!emitterNode)
    {
        return rawNode;
    }

    SceneNode* curParentNode = emitterNode->GetParent();
    ParticleEffectNode* effectParentNode = dynamic_cast<ParticleEffectNode*>(curParentNode);
    
    // If the Emitter Node has parent, but its parent is not ParticleEffectNode, we need to
    // "adopt" this node by creating ParticleEffect node and attaching.
    if (curParentNode && (effectParentNode == NULL))
    {
        ParticleEffectNode* newParentNodeParticleEffect = new ParticleEffectNode();
        curParentNode->AddNode(newParentNodeParticleEffect);

        // Add the emitter node to the new Effect (this will also remove it from the scene).
        newParentNodeParticleEffect->AddNode(emitterNode);

        // Register the Particle Editor structure for the new node.
        EffectParticleEditorNode* effectEditorNode =
        ParticlesEditorController::Instance()->RegisterParticleEffectNode(newParentNodeParticleEffect);
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(newParentNodeParticleEffect,
            emitterNode, QString::fromStdString(emitterNode->GetName()));
        effectEditorNode->AddChildNode(emitterEditorNode);

        return newParentNodeParticleEffect;
    }
    
    // No preprocessing needed.
    return rawNode;
}

bool ParticlesEditorSceneHelper::AddNodeToSceneGraph(SceneGraphItem *graphItem, SceneNode *node)
{
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode == NULL)
    {
        // Not ours - process as-is.
        return false;
    }

    EffectParticleEditorNode* effectEditorNode = ParticlesEditorController::Instance()->GetRootForParticleEffectNode(effectNode);
    if (!effectEditorNode)
    {
        // Possible while loading projects - register the node in this case.
        effectEditorNode = ParticlesEditorController::Instance()->RegisterParticleEffectNode(effectNode);
    }
    
    if (graphItem->GetExtraUserData() == NULL)
    {
        // Register the root. Note - it has both User Data and Extra Data.
        graphItem->SetExtraUserData(effectEditorNode);
    }

    // Build the Scene Graph in a recursive way.
    BuildSceneGraphRecursive(effectEditorNode, graphItem);
    return true;
}

void ParticlesEditorSceneHelper::BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem)
{
    // Prior to buildind the tree - need to check whether the children are in sync and synchronize them,
    // if not. This is actually needed for Load() process.
    SynchronizeParticleEditorTree(rootNode);
    
    // Set the UserData for inner Emitter nodes.
    if (dynamic_cast<EmitterParticleEditorNode*>(rootNode))
    {
        SceneNode* emitterNode = (static_cast<EmitterParticleEditorNode*>(rootNode))->GetEmitterNode();
        rootItem->SetUserData(emitterNode);
    }

    int childrenCount = rootNode->GetChildren().size();
    for (BaseParticleEditorNode::PARTICLEEDITORNODESLISTCONSTITER iter = rootNode->GetChildren().begin();
         iter != rootNode->GetChildren().end(); iter ++)
    {
        BaseParticleEditorNode* childNode = (*iter);

        SceneGraphItem* childItem = new SceneGraphItem();
        
        childItem->SetExtraUserData(childNode);
        rootItem->AppendChild(childItem);

        // Repeat for all children.
        BuildSceneGraphRecursive(childNode, childItem);
    }
}

void ParticlesEditorSceneHelper::SynchronizeParticleEditorTree(BaseParticleEditorNode* node)
{
    EffectParticleEditorNode* effectEditorNode = dynamic_cast<EffectParticleEditorNode*>(node);
    if (effectEditorNode)
    {
        SynchronizeEffectParticleEditorNode(effectEditorNode);
    }

    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(node);
    if (emitterEditorNode)
    {
        SynchronizeEmitterParticleEditorNode(emitterEditorNode);
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(node);
    if (layerEditorNode)
    {
        SynchronizeLayerParticleEditorNode(layerEditorNode);
    }
}

void ParticlesEditorSceneHelper::SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }
    
    ParticleEffectNode* effectNode = node->GetRootNode();
    if (!effectNode)
    {
        return;
    }

    // All the children of Effect Node are actually Emitters.
    int emittersCountInEffect = effectNode->GetChildrenCount();
    int emittersCountInEffectNode = node->GetEmittersCount();

    if (emittersCountInEffect > 0 && emittersCountInEffectNode == 0)
    {
        // Root Node has no Emitters - need to add them.
        for (int32 i = 0; i < emittersCountInEffect; i ++)
        {
            // Create the new Emitter and add it to the tree.
            ParticleEmitterNode* emitterSceneNode = dynamic_cast<ParticleEmitterNode*>(effectNode->GetChild(i));
            if (!emitterSceneNode)
            {
                continue;
            }
 
            EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectNode, emitterSceneNode,
                                                                                         QString::fromStdString(emitterSceneNode->GetName()));
            effectNode->AddNode(emitterSceneNode);
            node->AddChildNode(emitterEditorNode);
        }
    }
}

void ParticlesEditorSceneHelper::SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }

    ParticleEmitterNode* emitterNode = node->GetEmitterNode();
    if (!emitterNode)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterNode->GetEmitter();
    if (!emitter)
    {
        return;
    }

    int layersCountInEmitter = emitter->GetLayers().size();
    int layersCountInEmitterNode = node->GetLayersCount();

    if (layersCountInEmitter > 0 && layersCountInEmitterNode == 0)
    {
        // Emitter Node has no layers - need to add them.
        for (int32 i = 0; i < layersCountInEmitter; i ++)
        {
            // Create the new node and add it to the tree.
            LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(node, emitter->GetLayers()[i]);
            node->AddChildNode(layerNode);
        }

        node->UpdateLayerNames();
     }
}

void ParticlesEditorSceneHelper::SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }
    
    ParticleEmitterNode* emitterNode = node->GetEmitterNode();
    if (!emitterNode)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterNode->GetEmitter();
    if (!emitter)
    {
        return;
    }

    ParticleLayer* layer = node->GetLayer();
    if (!layer)
    {
        return;
    }

    // Synchronize the Forces.
    int32 forcesCountInLayer = layer->forces.size();
    int32 forcesCountInLayerNode = node->GetForcesCount();

    if (forcesCountInLayer > 0 && forcesCountInLayerNode == 0)
    {
        // Emitter Layer has no Forces - need to add them.
        for (int32 i = 0; i < forcesCountInLayer; i ++)
        {
            ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(node, i);
            node->AddChildNode(forceNode);
        }

        node->UpdateForcesIndices();
    }
}

bool ParticlesEditorSceneHelper::NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return true;
    }
    
    // We should block Scene Editor popup menu items for all child nodes for Particle Effect Node.
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        return false;
    }
    
    return true;
}

ExtraUserData* ParticlesEditorSceneHelper::GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const
{
    if (!modelIndex.isValid())
    {
        return NULL;
    }

    SceneGraphItem *item = static_cast<SceneGraphItem*>(modelIndex.internalPointer());
    if (!item)
    {
        return NULL;
    }
    
    return item->GetExtraUserData();
}

void ParticlesEditorSceneHelper::AddPopupMenuItems(SceneData* sceneData, QMenu& menu, const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return;
    }
    
    // Which kind of Node is it?
    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
        // Effect Node. Allow to add Particle Emitters and start/stop the animation.
        sceneData->AddActionToMenu(&menu, QString("Add Particle Emitter"), new CommandAddParticleEmitter());
        sceneData->AddActionToMenu(&menu, QString("Start Particle Effect"), new CommandStartStopParticleEffect(true));
        sceneData->AddActionToMenu(&menu, QString("Stop Particle Effect"), new CommandStartStopParticleEffect(false));
        sceneData->AddActionToMenu(&menu, QString("Restart Particle Effect"), new CommandRestartParticleEffect());
    }
    else if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        // For Particle Emitter we also allow to load/save it.
        sceneData->AddActionToMenu(&menu, QString("Load Emitter from Yaml"), new CommandLoadParticleEmitterFromYaml());
        sceneData->AddActionToMenu(&menu, QString("Save Emitter to Yaml"), new CommandSaveParticleEmitterToYaml(false));
        sceneData->AddActionToMenu(&menu, QString("Save Emitter to Yaml As"), new CommandSaveParticleEmitterToYaml(true));
        
        // Emitter node. Allow to remove it and also add Layers.
        sceneData->AddActionToMenu(&menu, QString("Remove Particle Emitter"), new CommandRemoveSceneNode());
        sceneData->AddActionToMenu(&menu, QString("Add Layer"), new CommandAddParticleEmitterLayer());
    }
    else if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        // Layer Node. Allow to remove it and add new Force.
        sceneData->AddActionToMenu(&menu, QString("Remove Layer"), new CommandRemoveParticleEmitterLayer());
        sceneData->AddActionToMenu(&menu, QString("Clone Layer"), new CommandCloneParticleEmitterLayer());
        sceneData->AddActionToMenu(&menu, QString("Add Force"), new CommandAddParticleEmitterForce());
    }
    else if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        // Force Node. Allow to remove it.
        sceneData->AddActionToMenu(&menu, QString("Remove Force"), new CommandRemoveParticleEmitterForce());
    }
}

bool ParticlesEditorSceneHelper::AddSceneNode(SceneNode* node) const
{
    ParticleEmitterNode* emitterSceneNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (emitterSceneNode)
    {
        ParticlesEditorController::Instance()->AddParticleEmitterNodeToScene(emitterSceneNode);
        return true;
    }
    
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode == NULL)
    {
        // Not relevant.
        return false;
    }

    // Register in Particles Editor.
    ParticlesEditorController::Instance()->RegisterParticleEffectNode(effectNode);
    return false;
}

void ParticlesEditorSceneHelper::RemoveSceneNode(SceneNode *node) const
{
    // If the Particle Effect node is removed - unregister it.
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode)
    {
        ParticlesEditorController::Instance()->UnregiserParticleEffectNode(effectNode);
    }
    
    // If the Particle Emitter node is removed - remove it from the tree.
    ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (emitterNode)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(emitterNode);
    }
}
