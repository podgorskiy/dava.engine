#pragma once

#ifndef __DAVA_ReflectedType__
#include "Reflection/Public/ReflectedType.h"
#endif

#include "Reflection/Public/Wrappers.h"
#include "Reflection/Public/ReflectedObject.h"

namespace DAVA
{
namespace ReflectionDetail
{
template <typename T>
const ReflectedType* GetVirtualReflectedTypeImpl(const T* ptr, std::false_type)
{
    return nullptr;
}

template <typename T>
const ReflectedType* GetVirtualReflectedTypeImpl(const T* ptr, std::true_type)
{
    return static_cast<const ReflectedBase*>(ptr)->GetReflectedType();
}

template <typename T>
const ReflectedType* GetVirtualReflectedType(const T* ptr)
{
    static const bool isReflectedBase = std::is_base_of<ReflectedBase, T>::value;
    return GetVirtualReflectedTypeImpl(ptr, std::integral_constant<bool, isReflectedBase>());
}

template <typename T>
const ReflectedType* GetByThisPointer(const T* this_)
{
    return ReflectedType::Get<T>();
}

template <typename T>
struct ReflectionInitializerRunner
{
protected:
    template <typename U, void (*)()>
    struct SFINAE
    {
    };

    template <typename U>
    static char Test(SFINAE<U, &U::__ReflectionInitializer>*);

    template <typename U>
    static int Test(...);

    static const bool value = std::is_same<decltype(Test<T>(0)), char>::value;

    inline static void RunImpl(std::true_type)
    {
        // T has TypeInitializer function,
        // so we should run it
        T::__ReflectionInitializer();
    }

    inline static void RunImpl(std::false_type)
    {
        // T don't have TypeInitializer function,
        // so nothing to do here
    }

public:
    static void Run()
    {
        using CheckType = typename std::conditional<std::is_class<T>::value, ReflectionInitializerRunner<T>, std::false_type>::type;
        RunImpl(std::integral_constant<bool, CheckType::value>());
    }
};
} // namespace ReflectionDetail

template <typename T>
ReflectedType* ReflectedType::Create()
{
    static ReflectedType ret;
    return &ret;
}

template <typename T>
ReflectedType* ReflectedType::Edit()
{
    using DecayT = std::decay_t<T>;
    ReflectedType* ret = Create<DecayT>();

    if (nullptr == ret->type)
    {
        ret->type = Type::Instance<DecayT>();
        ret->rttiName = typeid(DecayT).name();
        ret->structureWrapper.reset(StructureWrapperCreator<T>::Create());
        ret->structureEditorWrapper.reset(StructureEditorWrapperCreator<T>::Create());

        typeToReflectedTypeMap[ret->type] = ret;
        rttiNameToReflectedTypeMap[ret->rttiName] = ret;

        ReflectionDetail::ReflectionInitializerRunner<T>::Run();
    }

    return ret;
}

template <typename T>
const ReflectedType* ReflectedType::Get()
{
    return Edit<T>();
}

template <typename T>
const ReflectedType* ReflectedType::GetByPointer(const T* ptr)
{
    const ReflectedType* ret = nullptr;

    if (nullptr != ptr)
    {
        ret = ReflectionDetail::GetVirtualReflectedType(ptr);
        if (nullptr == ret)
        {
            ret = ReflectedType::Edit<T>();
        }
    }

    return ret;
}

template <typename T, typename... Bases>
void ReflectedType::RegisterBases()
{
    Type::RegisterBases<T, Bases...>();
    bool basesUnpack[] = { false, ReflectedType::Edit<Bases>() != nullptr... };
}

inline const Type* ReflectedType::GetType() const
{
    return type;
}

inline const String& ReflectedType::GetPermanentName() const
{
    return permanentName;
}

} // namespace DAVA
