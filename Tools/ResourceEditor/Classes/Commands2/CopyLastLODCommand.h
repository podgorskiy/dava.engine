#pragma once

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "DAVAEngine.h"

class CopyLastLODToLod0Command : public CommandWithoutExecute
{
public:
    //TODO: remove after lod editing implementation
    DAVA_DEPRECATED(CopyLastLODToLod0Command(DAVA::LodComponent* lod));
    ~CopyLastLODToLod0Command();

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    DAVA::LodComponent* lodComponent;
    DAVA::Vector<DAVA::RenderBatch*> newBatches;
    DAVA::Vector<DAVA::int32> switchIndices;
};
