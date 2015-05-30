#ifndef __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
#define __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__

#include "SectionProperty.h"

namespace DAVA
{
    class UIControl;
    class UIControlBackground;
}

class BackgroundPropertiesSection : public SectionProperty
{
public:
    BackgroundPropertiesSection(DAVA::UIControl *control, int bgNum, const BackgroundPropertiesSection *sourceSection, eCloneType copyType);
protected:
    virtual ~BackgroundPropertiesSection();
public:
    DAVA::UIControlBackground *GetBg() const;
    void CreateControlBackground();

    bool HasChanges() const override;
    void Serialize(PackageSerializer *serializer) const override;
    void Accept(PropertyVisitor *visitor) override;

private:
    DAVA::UIControl *control;
    DAVA::UIControlBackground *bg;
    
    int bgNum;
};

#endif // __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
