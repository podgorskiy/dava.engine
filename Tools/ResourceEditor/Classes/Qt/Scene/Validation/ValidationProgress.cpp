#include "Qt/Scene/Validation/ValidationProgress.h"
#include "Qt/Scene/Validation/ValidationProgressConsumer.h"
#include "Debug/DVAssert.h"

namespace SceneValidation
{
void ValidationProgress::Started(const DAVA::String& title)
{
    DVASSERT(consumer != nullptr);
    result = DAVA::Result::RESULT_SUCCESS;
    consumer->ValidationStarted(title);
}

void ValidationProgress::Alerted(const DAVA::String& msg)
{
    DVASSERT(consumer != nullptr);
    result = DAVA::Result::RESULT_FAILURE;
    consumer->ValidationAlert(msg);
}

void ValidationProgress::Finished()
{
    DVASSERT(consumer != nullptr);
    consumer->ValidationDone();
}
}
