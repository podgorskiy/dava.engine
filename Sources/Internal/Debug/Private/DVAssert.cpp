#include "Debug/DVAssert.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"

#include <csignal>

namespace DAVA
{
namespace Assert
{
static Vector<Handler> registredHandlers;

static Mutex registredHandlersMutex;

void AddHandler(const Handler handler)
{
    LockGuard<Mutex> lock(registredHandlersMutex);

    const Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        return;
    }

    registredHandlers.push_back(handler);
}

void RemoveHandler(const Handler handler)
{
    LockGuard<Mutex> lock(registredHandlersMutex);

    const Vector<Handler>::iterator position = std::find(registredHandlers.begin(), registredHandlers.end(), handler);
    if (position != registredHandlers.end())
    {
        registredHandlers.erase(position);
    }
}
}
}

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
void RaiseSigTrap()
{
    raise(SIGTRAP);
}
#endif

int GetFailBehaviourPriority(const DAVA::Assert::FailBehaviour behaviour)
{
    switch (behaviour)
    {
    case DAVA::Assert::FailBehaviour::Default:
        return 0;

    case DAVA::Assert::FailBehaviour::Continue:
        return 1;

    case DAVA::Assert::FailBehaviour::Halt:
        return 2;

    default:
        return -1;
    }
}

DAVA::Assert::FailBehaviour HandleAssert(const char* const expr,
                                         const char* const fileName,
                                         const int lineNumber,
                                         const DAVA::Vector<DAVA::Debug::StackFrame>& backtrace,
                                         const char* const message)
{
    // Copy handlers list to avoid data race in case some handler uses AddHandler or RemoveHandler functions
    DAVA::Vector<DAVA::Assert::Handler> handlersCopy;
    {
        DAVA::LockGuard<DAVA::Mutex> lock(DAVA::Assert::registredHandlersMutex);
        handlersCopy = DAVA::Assert::registredHandlers;
    }

    if (handlersCopy.empty())
    {
        return DAVA::Assert::FailBehaviour::Default;
    }

    // Invoke all the handlers with according assert info and return result behaviour
    // Each behaviour is more prioritized than the previous one, in this order:
    // Default -> Continue -> Halt

    const DAVA::Assert::AssertInfo assertInfo(expr, fileName, lineNumber, message, backtrace);

    DAVA::Assert::FailBehaviour resultBehaviour = DAVA::Assert::FailBehaviour::Default;
    for (const DAVA::Assert::Handler& handler : handlersCopy)
    {
        const DAVA::Assert::FailBehaviour requestedBehaviour = handler(assertInfo);

        if (GetFailBehaviourPriority(requestedBehaviour) > GetFailBehaviourPriority(resultBehaviour))
        {
            resultBehaviour = requestedBehaviour;
        }
    }

    return resultBehaviour;
}