#include "Input/Private/Ios/GamepadDeviceImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "Input/GamepadDevice.h"
#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
GamepadDeviceImpl::GamepadDeviceImpl(GamepadDevice* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadDeviceImpl::Update()
{
    if ([controller extendedGamepad])
    {
        GCExtendedGamepad* gamepad = [controller extendedGamepad];
        ReadExtendedGamepadElements(gamepad);
    }
    else if ([controller gamepad])
    {
        GCGamepad* gamepad = [controller gamepad];
        ReadGamepadElements(gamepad);
    }
}

void GamepadDeviceImpl::ReadExtendedGamepadElements(GCExtendedGamepad* gamepad)
{
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);

    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_LTHUMB, gamepad.leftThumbstick.xAxis.value, true);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_LTHUMB, gamepad.leftThumbstick.yAxis.value, false);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_RTHUMB, gamepad.rightThumbstick.xAxis.value, true);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_RTHUMB, gamepad.rightThumbstick.yAxis.value, false);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_LTRIGGER, gamepad.leftTrigger.value, true);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_AXIS_RTRIGGER, gamepad.rightTrigger.value, true);
}

void GamepadDeviceImpl::ReadGamepadElements(GCGamepad* gamepad)
{
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    if (controller == nullptr)
    {
        NSArray<GCController*>* controllers = [GCController controllers];
        if ([controllers count] != 0)
        {
            controller = (__bridge GCController*)CFBridgingRetain([controllers objectAtIndex:0]);
            DetermineSupportedElements();
        }
    }
    return controller != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    bool removed = true;
    for (GCController* c in [GCController controllers])
    {
        if (c == controller)
        {
            removed = false;
            break;
        }
    }
    if (removed && controller != nullptr)
    {
        CFBridgingRelease(controller);
        controller = nullptr;
    }
    return controller != nullptr;
}

void GamepadDeviceImpl::DetermineSupportedElements()
{
    if ([controller extendedGamepad])
    {
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_START - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_BACK - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_A - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_B - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_X - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_Y - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_LEFT - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_RIGHT - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_UP - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_DOWN - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LSHOUDER - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RSHOUDER - eInputElements::GAMEPAD_FIRST, true);

        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTRIGGER - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTRIGGER - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB_X - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB_Y - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB_X - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB_Y - eInputElements::GAMEPAD_FIRST, true);
    }
    else if ([controller gamepad])
    {
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_START - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_BACK - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_A - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_B - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_X - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_Y - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_LEFT - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_RIGHT - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_UP - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_DPAD_DOWN - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LSHOUDER - eInputElements::GAMEPAD_FIRST, true);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RSHOUDER - eInputElements::GAMEPAD_FIRST, true);

        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTRIGGER - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTRIGGER - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB_X - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_LTHUMB_Y - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB_X - eInputElements::GAMEPAD_FIRST, false);
        gamepadDevice->supportedElements.set(eInputElements::GAMEPAD_RTHUMB_Y - eInputElements::GAMEPAD_FIRST, false);
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
