#pragma once

#include "QECommands/Private/QEPackageCommand.h"

#include <FileSystem/VariantType.h>

class ControlNode;
class AbstractProperty;

class ResizeCommand : public QEPackageCommand
{
public:
    ResizeCommand(PackageNode* package);
    void AddNodePropertyValue(ControlNode* node, AbstractProperty* sizeProperty, const DAVA::VariantType& sizeValue, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue);

    ~ResizeCommand() override = default;

    void Redo() override;
    void Undo() override;

    bool MergeWith(const DAVA::Command* command) override;

private:
    struct Item
    {
        Item(ControlNode* node, AbstractProperty* sizeProperty, const DAVA::VariantType& sizeValue, AbstractProperty* pivotProperty, const DAVA::VariantType& pivotValue);
        ControlNode* node = nullptr;
        AbstractProperty* sizeProperty = nullptr;
        DAVA::VariantType sizeNewValue;
        DAVA::VariantType sizeOldValue;

        AbstractProperty* pivotProperty = nullptr;
        DAVA::VariantType pivotNewValue;
        DAVA::VariantType pivotOldValue;
    };
    DAVA::Vector<Item> items;
};
