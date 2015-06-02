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


#ifndef __DAVAENGINE_WAYPOINT_COMPONENT_H__
#define __DAVAENGINE_WAYPOINT_COMPONENT_H__

#include "Entity/Component.h"
#include "Base/Introspection.h"

namespace DAVA
{

class SerializationContext;
class KeyedArchive;
class Entity;
    
class WaypointComponent: public Component
{
protected:
    ~WaypointComponent();
public:
	IMPLEMENT_COMPONENT_TYPE(WAYPOINT_COMPONENT);

	WaypointComponent();
	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    void SetProperties(KeyedArchive *archieve);
    KeyedArchive * GetProperties() const;
    
    void SetPathName(const FastName & name);
    const FastName & GetPathName() const;

    void SetStarting(bool);
    bool IsStarting() const;

private:
    
    FastName pathName;
    KeyedArchive *properties;
    bool isStarting = false;
    
public:
	INTROSPECTION_EXTEND(WaypointComponent, Component,
        MEMBER(pathName, "Path Name", I_VIEW)
        MEMBER(properties, "Waypoint properties", I_VIEW | I_EDIT)
    );
};


inline KeyedArchive * WaypointComponent::GetProperties() const
{
    return properties;
}
    
inline void WaypointComponent::SetPathName(const FastName & name)
{
    pathName = name;
}

inline const FastName & WaypointComponent::GetPathName() const
{
    return pathName;
}

inline void WaypointComponent::SetStarting(bool newVal)
{
    isStarting = newVal;
}

inline bool WaypointComponent::IsStarting() const
{
    return isStarting;
}


    
}
#endif //__DAVAENGINE_WAYPOINT_COMPONENT_H__
