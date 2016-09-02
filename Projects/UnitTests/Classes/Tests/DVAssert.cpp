#include "UnitTests/UnitTests.h"
#include "Debug/DVAssert.h"

using namespace DAVA::Assert;

static bool firstHandlerInvoked = false;
static FailBehaviour FirstHandler(const AssertInfo& assertInfo)
{
    firstHandlerInvoked = true;
    return FailBehaviour::Continue;
}

static bool secondHandlerInvoked = false;
static FailBehaviour SecondHandler(const AssertInfo& assertInfo)
{
    secondHandlerInvoked = true;
    return FailBehaviour::Continue;
}

static void ResetHandlersState()
{
    firstHandlerInvoked = false;
    secondHandlerInvoked = false;
}

static std::string lastHandlerMessage;
static FailBehaviour AssertMessageSavingHandler(const AssertInfo& assertInfo)
{
    lastHandlerMessage = std::string(assertInfo.message);
    return FailBehaviour::Continue;
}

DAVA_TESTCLASS(DVAssertTestClass)
{
    DAVA_TEST(AssertTestFunction)
    {
        AddHandler(FirstHandler);
        AddHandler(SecondHandler);

        // If an assert doesn't fail, none should be called
        DVASSERT(true);
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(!secondHandlerInvoked);

        // If it fails, both should be called in debug mode
        DVASSERT(false);
#if __DAVAENGINE_DEBUG__
        TEST_VERIFY(firstHandlerInvoked);
        TEST_VERIFY(secondHandlerInvoked);
#else
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(!secondHandlerInvoked);
#endif
        
        // Check that first removed handler doesn't get called
        RemoveHandler(FirstHandler);
        ResetHandlersState();
        DVASSERT(false);
#if __DAVAENGINE_DEBUG__
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(secondHandlerInvoked);
#else
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(!secondHandlerInvoked);
#endif

        // Reset
        RemoveHandler(SecondHandler);
        ResetHandlersState();
    }

    DAVA_TEST(CriticalAssertTestFunction)
    {
        AddHandler(FirstHandler);
        AddHandler(SecondHandler);

        // If an assert doesn't fail, none should be called
        DVASSERT_CRITICAL(true);
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(!secondHandlerInvoked);

        // If it fails, both should be called in both debug & release modes
        DVASSERT_CRITICAL(false);
        TEST_VERIFY(firstHandlerInvoked);
        TEST_VERIFY(secondHandlerInvoked);

        // Check that first removed handler doesn't get called
        RemoveHandler(FirstHandler);
        ResetHandlersState();
        DVASSERT_CRITICAL(false);
        TEST_VERIFY(!firstHandlerInvoked);
        TEST_VERIFY(secondHandlerInvoked);

        // Reset
        RemoveHandler(SecondHandler);
        ResetHandlersState();
    }

    DAVA_TEST(AssertMessageTestFunction)
    {
        AddHandler(AssertMessageSavingHandler);

        // Check that message is empty if none was specified
        DVASSERT_CRITICAL(false);
        TEST_VERIFY(lastHandlerMessage == "");

        // Check message without formatting
        std::string message = "such assert";
        DVASSERT_CRITICAL(false, message.c_str());
        TEST_VERIFY(lastHandlerMessage == message);

        // Check message with formatting
        std::string messageFormat = "very test: %d, %s";
        std::string formattedMessage = "very test: 42, wow";
        DVASSERT_CRITICAL(false, messageFormat.c_str(), 42, "wow");
        TEST_VERIFY(lastHandlerMessage == formattedMessage);
    }
};