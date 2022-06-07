#include <AppKit/NSWindow.h>
#include <AppKit/NSScreen.h>
#include "lionsupport.h"

bool LionSupport::isLion()
{
    NSString *string = [NSString string];
    // this selector was added only in Lion. so we can check if it's responding, we are on Lion
    return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}

void LionSupport::addFullscreen(MainWindow *window)
{
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (isLion()) // checks if lion is running
    {
        NSView *nsview = (NSView *) window->winId();
        NSWindow *nswindow = [nsview window];
        [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
#else
#warning No fullscreen support will be included in this build
#endif
}

bool LionSupport::isRetinaDisplay()
{
#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
  return( [ [ NSScreen mainScreen ] respondsToSelector:@selector( backingScaleFactor ) ]
          && [ [ NSScreen mainScreen ] backingScaleFactor ] > 1.5 );
#else
  return false;
#endif
}
