#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/ValidatorDelegate.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Base/BaseTypes.h>
#include <QWidget>

class QLineEdit;
class QToolButton;
namespace DAVA
{
namespace TArc
{
class FilePathEdit : public ControlProxy<QWidget>, private ValidatorDelegate
{
public:
    enum class Fields : uint32
    {
        Value,
        PlaceHolder,
        IsReadOnly,
        IsEnabled,
        Filters,
        FieldCount
    };

    struct Params
    {
        UI* ui;
        WindowKey wndKey = FastName("");
        ControlDescriptorBuilder<Fields> fields;
    };

    FilePathEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    FilePathEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
    void SetupControl();

    void EditingFinished();
    void ButtonClicked();

    M::ValidationResult FixUp(const Any& value) const override;
    M::ValidationResult Validate(const Any& value) const override;
    void ShowHint(const QString& message) override;

    void ExtractMetaInfo(bool& isFile, bool& shouldExists, QString& filters) const;

private:
    UI* ui = nullptr;
    WindowKey wndKey;
    QtConnections connections;
    QLineEdit* edit = nullptr;
    QToolButton* button = nullptr;
};
}
} // namespace DAVA