#ifndef __OWNERS_SIGNATURE_SYSTEM_H__
#define __OWNERS_SIGNATURE_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "DAVAEngine.h"
#include "Commands2/Base/Command2.h"

class OwnersSignatureSystem : public DAVA::SceneSystem
{
public:
    OwnersSignatureSystem(DAVA::Scene* scene);

    void ProcessCommand(const Command2* command, bool redo);

    void AddEntity(DAVA::Entity* entity) override;
    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;
};


#endif /* defined(__OWNERS_SIGNATURE_SYSTEM_H__) */
