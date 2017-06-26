#pragma once

#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"
#include "UI/RichContent/UIRichAliasMap.h"

namespace DAVA
{
/**
    RichContent compoent support tag aliases. Any alias contains alias
    name, original tag and original attributes. After declaration of alias you
    can use it like an any tag in the content source. But all attributes witch
    will be added to alias in content source will be ignored and replaced by
    attributes from alias data.

    For example:
    <code>
        Defined aliases:
        h1 = <p class="header" />

        Source:
        <h1 class="ordinary_text">Header</h1>
    </code>
    In this case tag `h1` with its attributes will be raplaced by tag `p` with
    attributes `class="header"`.
*/
class UIRichContentAliasesComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIRichContentAliasesComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIRichContentAliasesComponent);

public:
    /** Default constructor. */
    UIRichContentAliasesComponent() = default;
    /** Copy constructor. */
    UIRichContentAliasesComponent(const UIRichContentAliasesComponent& src);
    /** Removed operator overloading. */
    UIRichContentAliasesComponent& operator=(const UIRichContentAliasesComponent&) = delete;

    UIRichContentAliasesComponent* Clone() const override;

    /** Set aliases for tags with attributes. */
    void SetAliases(const UIRichAliasMap& aliases);
    /** Return aliases for tags. */
    const UIRichAliasMap& GetAliases() const;

    /** Set aliases for tags from specified string. */
    void SetAliasesFromString(const String& aliases);
    /** Return aliases for tags as string. */
    const String& GetAliasesAsString();

    /** Set modification flag. */
    void SetModified(bool modified);
    /** Return value of modification flag. */
    bool IsModified() const;

protected:
    ~UIRichContentAliasesComponent() override = default;

private:
    UIRichAliasMap aliases;
    bool modified = false;
};

inline bool UIRichContentAliasesComponent::IsModified() const
{
    return modified;
}

inline const UIRichAliasMap& UIRichContentAliasesComponent::GetAliases() const
{
    return aliases;
}
}
