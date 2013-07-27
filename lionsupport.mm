#include "lionsupport.h"

bool LionSupport::isLion()
{
    NSString *string = [NSString string];
    // this selector was added only in Lion. so we can check if it's responding, we are on Lion
    return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}

void LionSupport::addFullscreen(MainWindow *window)
{
}
