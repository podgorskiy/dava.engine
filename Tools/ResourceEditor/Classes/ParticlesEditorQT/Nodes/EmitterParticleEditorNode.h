//
//  EmitterParticleEditorNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__EmitterParticleEditorNode__
#define __ResourceEditorQt__EmitterParticleEditorNode__

#include "EmitterContainerNode.h"
#include "Scene3D/ParticleEmitterNode.h"

namespace DAVA {
    
// Emitter Particle Editor node.
class EmitterParticleEditorNode : public EmitterContainerNode
{
public:
    EmitterParticleEditorNode(ParticleEffectNode* rootNode, ParticleEmitterNode* emitter,
                              const QString& nodeName);
    
    // Get the layers count.
    int32 GetLayersCount() const;

    // Update the name for child layers.
    void UpdateLayerNames();
};

};

#endif /* defined(__ResourceEditorQt__EmitterParticleEditorNode__) */
