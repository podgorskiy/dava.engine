#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

namespace DAVA
{
template <typename T>
inline ReflectedObject::ReflectedObject(T* ptr_)
    : ptr(ptr_)
    , reflectedType(ReflectedTypeDB::GetByPointer(ptr_))
{
}

template <typename T>
inline ReflectedObject::ReflectedObject(const T* ptr_)
    : ptr(const_cast<T*>(ptr_))
    , reflectedType(ReflectedTypeDB::GetByPointer(ptr_))
    , isConst(true)
{
}

/*
inline ReflectedObject::ReflectedObject(void* ptr_, const ReflectedType* reflectedType_)
    : ptr(ptr_)
    , reflectedType(reflectedType_)
{
}

inline ReflectedObject::ReflectedObject(const void* ptr_, const ReflectedType* reflectedType_)
    : ptr(const_cast<void*>(ptr_))
    , reflectedType(reflectedType_)
    , isConst(true)
{
}
*/

inline const ReflectedType* ReflectedObject::GetReflectedType() const
{
    return reflectedType;
}

inline bool ReflectedObject::IsValid() const
{
    return ((nullptr != ptr) && (nullptr != reflectedType));
}

inline bool ReflectedObject::IsConst() const
{
    return isConst;
}

template <typename T>
inline T* ReflectedObject::GetPtr() const
{
    const RtType* reqType = RtType::Instance<T>();
    const RtType* curType = reflectedType->GetRtType();

    if (reqType == curType || reqType->Decay() == curType)
    {
        return static_cast<T*>(ptr);
    }

    void* ret = nullptr;
    bool canCast = RtTypeInheritance::DownCast(curType, reqType, ptr, &ret);

    DVASSERT(canCast);

    return static_cast<T*>(ret);
}

inline void* ReflectedObject::GetVoidPtr() const
{
    return ptr;
}

/*
inline ReflectedObject ReflectedObject::Deref() const
{
    ReflectedObject ret;

    if (nullptr == ptr)
        return ret;

    const RttiType* derefType = reflectedType->GetRtType();

    if (nullptr == derefType)
        return ret;

    if (derefType->IsPointer())
    {
        void* inPtr = *(static_cast<void**>(ptr));
        //ret = ReflectedObject(inPtr, derefType);
    }

    return ret;
}
*/

} // namespace DAVA
