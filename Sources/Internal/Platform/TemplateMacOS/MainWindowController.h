#import <Cocoa/Cocoa.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

#import "OpenGLView.h"
#import "AppDelegate.h"

@interface MainWindowController : NSWindowController<NSWindowDelegate, NSFileManagerDelegate>
{
@public
    float32 currFPS;
    OpenGLView* openGLView;
    NSWindow* mainWindow;
    NSTimer* animationTimer;

    ApplicationCore* core;
    bool fullScreen;
    bool willQuit;

@private
    IOPMAssertionID assertionID;
}

@property(assign) bool willQuit;

- (void)createWindows;

- (bool)isFullScreen;
- (bool)setFullScreen:(bool)_fullScreen;

- (void)allowDisplaySleep:(bool)sleep;

- (void)windowWillMiniaturize:(NSNotification*)notification;
- (void)windowDidDeminiaturize:(NSNotification*)notification;

- (void)windowDidEnterFullScreen:(NSNotification*)notification;
- (void)windowDidExitFullScreen:(NSNotification*)notification;

- (void)windowDidBecomeKey:(NSNotification*)notification;
- (void)windowDidResignKey:(NSNotification*)notification;

- (void)OnSuspend;
- (void)OnResume;

@end
