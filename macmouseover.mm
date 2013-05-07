#include "macmouseover.hh"
#include <AppKit/NSTouch.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSScreen.h>
#include <Foundation/NSAutoreleasePool.h>

const int mouseOverInterval = 300;

CGEventRef eventCallback( CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon )
{
(void) proxy;
  if( type != kCGEventMouseMoved )
    return event;
  static_cast< MacMouseOver * >( refcon )->mouseMoved();
  return event;
}

static CGPoint carbonScreenPointFromCocoaScreenPoint( NSPoint cocoaPoint )
{
  NSScreen *foundScreen = nil;
  CGPoint thePoint;

  for (NSScreen *screen in [NSScreen screens]) {
    if (NSPointInRect(cocoaPoint, [screen frame])) {
        foundScreen = screen;
    }
  }

  if (foundScreen) {
    CGFloat screenHeight = [foundScreen frame].size.height;
    thePoint = CGPointMake(cocoaPoint.x, screenHeight - cocoaPoint.y - 1);
  }
  else
    thePoint = CGPointMake(0.0, 0.0);

  return thePoint;
}

MacMouseOver & MacMouseOver::instance()
{
  static MacMouseOver m;

  return m;
}

MacMouseOver::MacMouseOver() :
  pPref(NULL)
, tapRef( 0 )
, loop( 0 )
{
  mouseTimer.setSingleShot( true );
  connect( &mouseTimer, SIGNAL( timeout() ), this, SLOT( timerShot() ) );

  elementSystemWide = AXUIElementCreateSystemWide();
}

MacMouseOver::~MacMouseOver()
{
  disableMouseOver();

  if( tapRef )
    CFRelease( tapRef );

  if( loop )
    CFRelease( loop );

  if( elementSystemWide )
    CFRelease( elementSystemWide );
}

QString MacMouseOver::CFStringRefToQString( CFStringRef str )
{
  int length = CFStringGetLength( str );
  if( length == 0 )
    return QString();

  UniChar *chars = new UniChar[ length ];
  CFStringGetCharacters( str, CFRangeMake( 0, length ), chars );

  QString result = QString::fromUtf16( chars, length );

  delete  chars;
  return result;
}

void MacMouseOver::mouseMoved()
{
  mouseTimer.start( mouseOverInterval );
}

void MacMouseOver::enableMouseOver()
{
  mouseTimer.stop();
  if( !AXAPIEnabled() )
    return;
  if( !tapRef )
    tapRef = CGEventTapCreate( kCGAnnotatedSessionEventTap, kCGHeadInsertEventTap,
                               kCGEventTapOptionListenOnly,
                               CGEventMaskBit( kCGEventMouseMoved ),
                               eventCallback, this );
  if( !tapRef )
    return;
  if( !loop )
    loop = CFMachPortCreateRunLoopSource( kCFAllocatorDefault, tapRef, 0 );
  if( loop )
    CFRunLoopAddSource( CFRunLoopGetMain(), loop, kCFRunLoopCommonModes );
}

void MacMouseOver::disableMouseOver()
{
  mouseTimer.stop();
  if( loop )
    CFRunLoopRemoveSource( CFRunLoopGetMain(), loop, kCFRunLoopCommonModes );
}

void MacMouseOver::timerShot()
{
  if( mouseMutex.tryLock( 0 ) )
    mouseMutex.unlock();
  else
    return;
  if( !pPref )
    return;
  if( pPref->enableScanPopupModifiers && checkModifiersPressed( pPref->scanPopupModifiers ) )
    handlePosition();
}

void MacMouseOver::handlePosition()
{
  Mutex::Lock _( mouseMutex );

  QString strToTranslate;

  NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
  CGPoint pt = carbonScreenPointFromCocoaScreenPoint( [NSEvent mouseLocation] );
  [ pool drain ];

  CFArrayRef names = 0;

  AXUIElementRef elem = 0;
  AXError err = AXUIElementCopyElementAtPosition( elementSystemWide, pt.x, pt.y, &elem );

  if( err != kAXErrorSuccess )
    return;

  for( ; ; )
  {
    CFTypeRef parameter = AXValueCreate( kAXValueCGPointType, &pt );
    CFTypeRef rangeValue;
    err = AXUIElementCopyParameterizedAttributeNames( elem, &names );
    if( err != kAXErrorSuccess )
      break;

    int numOfAttributes = CFArrayGetCount( names );
    if( CFArrayContainsValue( names, CFRangeMake( 0, numOfAttributes ), CFSTR( "AXRangeForPosition" ) ) )
    {
      // Standard interface
      err = AXUIElementCopyParameterizedAttributeValue( elem, kAXRangeForPositionParameterizedAttribute,
                                                            parameter, ( CFTypeRef * )&rangeValue );
      CFRelease( parameter );
      if( err != kAXErrorSuccess )
        break;

      CFStringRef stringValue;

      CFRange decodedRange = CFRangeMake( 0, 0 );
      bool b = AXValueGetValue( (AXValueRef)rangeValue, kAXValueCFRangeType, &decodedRange );
      CFRelease( rangeValue );
      if( b )
      {
        int fromPos = decodedRange.location - 127;
        if( fromPos < 0 )
          fromPos = 0;
        int wordPos = decodedRange.location - fromPos;  // Cursor position in result string

        CFRange range = CFRangeMake( fromPos, wordPos + 1 );
        parameter = AXValueCreate( kAXValueCFRangeType, &range );
        err = AXUIElementCopyParameterizedAttributeValue( elem, kAXStringForRangeParameterizedAttribute,
                                                            parameter, (CFTypeRef *)&stringValue );
        CFRelease( parameter );
        if( err != kAXErrorSuccess )
          break;

        strToTranslate = CFStringRefToQString( stringValue );
        CFRelease( stringValue );

        // Read string further
        for( int i = 1; i < 128; i++ )
        {
          range = CFRangeMake( decodedRange.location + i, 1 );
          parameter = AXValueCreate( kAXValueCFRangeType, &range );
          err = AXUIElementCopyParameterizedAttributeValue( elem, kAXStringForRangeParameterizedAttribute,
                                                              parameter, (CFTypeRef *)&stringValue );
          CFRelease( parameter );

          if( err != kAXErrorSuccess )
            break;

          QString s = CFStringRefToQString( stringValue );
          CFRelease( stringValue );

          if( s[ 0 ].isLetterOrNumber() || s[ 0 ] == '-' )
            strToTranslate += s;
          else
            break;
        }

        handleRetrievedString( strToTranslate, wordPos );
      }
    }
    else if( CFArrayContainsValue( names, CFRangeMake( 0, numOfAttributes ), CFSTR( "AXTextMarkerForPosition" ) ) )
    {
      // Safari interface
      CFTypeRef marker, range;
      CFStringRef str;
      err = AXUIElementCopyParameterizedAttributeValue( elem, CFSTR( "AXTextMarkerForPosition" ),
                                                            parameter, ( CFTypeRef * )&marker );
      CFRelease( parameter );
      if( err != kAXErrorSuccess )
        break;

      err = AXUIElementCopyParameterizedAttributeValue( elem, CFSTR( "AXLeftWordTextMarkerRangeForTextMarker" ),
                                                        marker, ( CFTypeRef * )&range );
      CFRelease( marker );
      if( err != kAXErrorSuccess )
        break;

      err = AXUIElementCopyParameterizedAttributeValue( elem, CFSTR( "AXStringForTextMarkerRange" ),
                                                        range, ( CFTypeRef * )&str );
      CFRelease( range );
      if( err == kAXErrorSuccess )
      {
        strToTranslate = CFStringRefToQString( str );
        CFRelease( str );
        handleRetrievedString( strToTranslate, 0 );
      }
    }
    break;
  }
  if( elem )
    CFRelease( elem );
  if( names )
    CFRelease( names );
}

void MacMouseOver::handleRetrievedString( QString & wordSeq, int wordSeqPos )
{

  // locate the word inside the sequence

  QString word;

  if ( wordSeq[ wordSeqPos ].isSpace() )
  {
    // Currently we ignore such cases
    return;
  }
  else
  if ( !wordSeq[ wordSeqPos ].isLetterOrNumber() )
  {
    // Special case: the cursor points to something which doesn't look like a
    // middle of the word -- assume that it's something that joins two words
    // together.

    int begin = wordSeqPos;

    for( ; begin; --begin )
      if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
        break;

    int end = wordSeqPos;

    while( ++end < wordSeq.size() )
      if ( !wordSeq[ end ].isLetterOrNumber() )
        break;

    if ( end - begin == 1 )
    {
      // Well, turns out it was just a single non-letter char, discard it
      return;
    }

    word = wordSeq.mid( begin, end - begin );
  }
  else
  {
    // Cursor points to a letter -- cut the word it points to

    int begin = wordSeqPos;

    for( ; begin; --begin )
      if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
        break;

    int end = wordSeqPos;

    while( ++end < wordSeq.size() )
    {
      if ( !wordSeq[ end ].isLetterOrNumber() )
        break;
    }
    word = wordSeq.mid( begin, end - begin );
  }

  // See if we have an RTL char. Reverse the whole string if we do.

  for( int x = 0; x < word.size(); ++x )
  {
    QChar::Direction d = word[ x ].direction();

    if ( d == QChar::DirR || d == QChar::DirAL ||
         d == QChar::DirRLE || d == QChar::DirRLO )
    {
      std::reverse( word.begin(), word.end() );
      break;
    }
  }

  emit instance().hovered( word, false );
}
