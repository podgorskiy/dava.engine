#pragma once

#include <type_traits>
#include "Reflection/Reflection.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/ValueWrapperDirect.h"
#include "Reflection/Private/ValueWrapperClass.h"
#include "Reflection/Private/ValueWrapperClassFn.h"
#include "Reflection/Private/ValueWrapperClassFnPtr.h"
#include "Reflection/Private/ValueWrapperStatic.h"
#include "Reflection/Private/ValueWrapperStaticFn.h"
#include "Reflection/Private/ValueWrapperStaticFnPtr.h"
#include "Reflection/Private/CtorWrapperDefault.h"
#include "Reflection/Private/DtorWrapperDefault.h"
#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/Private/StructureWrapperPtr.h"
#include "Reflection/Private/StructureWrapperStd.h"
#include "Reflection/Private/StructureEditorWrapperPtr.h"
#include "Reflection/Private/StructureEditorWrapperStd.h"

namespace DAVA
{
/// \brief A reflection registration, that is used to register complex types structure.
template <typename C>
class ReflectionRegistrator final
{
public:
    static ReflectionRegistrator& Begin()
    {
        static ReflectionRegistrator ret;
        return ret;
    }

    template <typename... Args>
    ReflectionRegistrator& Constructor()
    {
        ReflectedType* type = ReflectedTypeDB::Edit<C>();
        auto ctorWrapper = std::make_unique<CtorWrapperDefault<C, Args...>>();
        //type->ctorWrappers.emplace(std::move(ctorWrapper));
        return *this;
    }

    ReflectionRegistrator& Destructor()
    {
        ReflectedType* type = ReflectedTypeDB::Edit<C>();
        auto dtorWrapper = std::make_unique<DtorWrapperDefault<C>>();
        //type->dtorWrapper = std::move(dtorWrapper);
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T* field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperStatic<T>>(field);
        //sw->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T C::*field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperClass<T, C>>(field);
        //sw->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = void (*)(SetT);

        return Field(name, getter, static_cast<SetFn>(nullptr));
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperStaticFnPtr<GetT, SetT>>(getter, setter);
        //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = void (C::*)(SetT);

        return Field(name, getter, static_cast<SetFn>(nullptr));
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using GetFn = GetT (C::*)();
        using SetFn = void (C::*)(SetT);

        return Field(name, reinterpret_cast<GetFn>(getter), static_cast<SetFn>(nullptr));
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT))
    {
        using GetFn = GetT (C::*)();

        return Field(name, reinterpret_cast<GetFn>(getter), setter);
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperClassFnPtr<GetT, SetT, C>>(getter, setter);
        //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = Function<void(SetT)>;

        return Field(name, getter, SetFn());
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter)
    {
        auto valueWrapper = std::make_unique<ValueWrapperStaticFn<GetT, SetT>>(getter, setter);
        //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = Function<void(C*, SetT)>;

        return Field(name, getter, SetFn());
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter)
    {
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<C, GetT, SetT>>(getter, setter);
        //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename Mt>
    ReflectionRegistrator& Method(const char* name, const Mt& method)
    {
        //sw->AddMethod(name, method);
        return *this;
    }

    ReflectionRegistrator& operator[](ReflectedMeta&& meta)
    {
        //sw->AddMeta(std::move(meta));
        return *this;
    }

    void End()
    {
        // override children for class C in appropriate ReflectedType
        ReflectedType* type = ReflectedTypeDB::Edit<C>();
        type->structure.reset(structure);
        structure = nullptr;

        // TODO:
        // ...
        // Add class wrapper
        // ...
    }

private:
    ReflectionRegistrator() = default;
    ReflectedStructure* structure = nullptr;
};

} // namespace DAVA
