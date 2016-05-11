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


#ifndef __PARTICLE_LAYER_MOVE_COMMAND_H__
#define __PARTICLE_LAYER_MOVE_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitterInstance.h"

class ParticleLayerMoveCommand : public Command2
{
public:
    ParticleLayerMoveCommand(DAVA::ParticleEmitterInstance* oldEmitter, DAVA::ParticleLayer* layer, DAVA::ParticleEmitterInstance* newEmitter, DAVA::ParticleLayer* newBefore = NULL);
    ~ParticleLayerMoveCommand();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const override;

private:
    DAVA::ParticleLayer* layer = nullptr;
    DAVA::ParticleEmitterInstance* oldEmitter = nullptr;
    DAVA::ParticleLayer* oldBefore = nullptr;
    DAVA::ParticleEmitterInstance* newEmitter = nullptr;
    DAVA::ParticleLayer* newBefore = nullptr;
};

inline DAVA::Entity* ParticleLayerMoveCommand::GetEntity() const
{
    return nullptr;
}

#endif // __PARTICLE_LAYER_MOVE_COMMAND_H__
