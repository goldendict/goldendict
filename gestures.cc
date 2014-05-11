/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QTouchEvent>
#include <QSwipeGesture>
#include <QVariant>
#include <math.h>
#include "articleview.hh"
#include "gestures.hh"

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)

namespace Gestures
{

Qt::GestureType GDPinchGestureType;
Qt::GestureType GDSwipeGestureType;
static bool fewTouchPointsPresented;

bool isFewTouchPointsPresented()
{
  return fewTouchPointsPresented;
}

QSwipeGesture::SwipeDirection getHorizontalDirection( qreal angle )
{
  if ( ( 0 <= angle && angle <= 45 )
       || ( 315 <= angle && angle <= 360 ) )
    return QSwipeGesture::Right;

  if ( 135 <= angle && angle <= 225 )
    return QSwipeGesture::Left;

  return QSwipeGesture::NoDirection;
}

QSwipeGesture::SwipeDirection getVerticalDirection( qreal angle )
{
  if ( 45 < angle && angle < 135 )
    return QSwipeGesture::Up;

  if ( 225 < angle && angle < 315 )
    return QSwipeGesture::Down;

  return QSwipeGesture::NoDirection;
}

GDPinchGesture::GDPinchGesture() :
isNewSequence( true )
{
}

QGesture * GDPinchGestureRecognizer::create( QObject * pTarget )
{
  if ( pTarget && pTarget->isWidgetType()) {
    static_cast< QWidget * >( pTarget )->setAttribute( Qt::WA_AcceptTouchEvents );
  }
  QGesture *pGesture = new GDPinchGesture;
  return pGesture;
}

void GDPinchGestureRecognizer::reset( QGesture * pGesture )
{
  QGestureRecognizer::reset( pGesture );
}

QGestureRecognizer::Result GDPinchGestureRecognizer::recognize( QGesture * state,
                                                                QObject *,
                                                                QEvent * event)
{
  const QTouchEvent *ev = static_cast< const QTouchEvent * >( event );
  QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
  GDPinchGesture * gest = static_cast< GDPinchGesture * >( state );

  switch ( event->type() )
  {
    case QEvent::TouchBegin:
      {
        result = QGestureRecognizer::MayBeGesture;
        gest->isNewSequence = true;
        break;
      }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    case QEvent::TouchCancel:
#endif
    case QEvent::TouchEnd:
      {
        result = QGestureRecognizer::CancelGesture;
        fewTouchPointsPresented = false;
        break;
      }
    case QEvent::TouchUpdate:
      {
        gest->scaleChanged = false;
        fewTouchPointsPresented = ( ev->touchPoints().size() > 1 );
        if ( ev->touchPoints().size() == 2 )
        {
          QTouchEvent::TouchPoint p1 = ev->touchPoints().at( 0 );
          QTouchEvent::TouchPoint p2 = ev->touchPoints().at( 1 );

          QPointF centerPoint = ( p1.screenPos() + p2.screenPos() ) / 2.0;
          gest->setHotSpot( centerPoint );
          if ( gest->isNewSequence )
          {
            gest->startPosition[ 0 ] = p1.screenPos();
            gest->startPosition[ 1 ] = p2.screenPos();
            gest->lastCenterPoint = centerPoint;
          }
          else
          {
            gest->lastCenterPoint = gest->centerPoint;
          }
          gest->centerPoint = centerPoint;

          if ( gest->isNewSequence )
          {
            gest->scaleFactor = 1.0;
            gest->lastScaleFactor = 1.0;
            gest->totalScaleFactor = 1.0;
          }
          else
          {
            gest->lastScaleFactor = gest->scaleFactor;
            QLineF line( p1.screenPos(), p2.screenPos() );
            QLineF lastLine( p1.lastScreenPos(),  p2.lastScreenPos() );
            gest->scaleFactor = line.length() / lastLine.length();
          }

          gest->totalScaleFactor = gest->totalScaleFactor * gest->scaleFactor;
          gest->scaleChanged = true;

          gest->isNewSequence = false;
          if( gest->totalScaleFactor <= OUT_SCALE_LIMIT || gest->totalScaleFactor >= IN_SCALE_LIMIT )
          {
            result = QGestureRecognizer::TriggerGesture;
            gest->isNewSequence = true;
          }
          else
            result = QGestureRecognizer::MayBeGesture;
        }
        else
        {
          gest->isNewSequence = true;
          if ( gest->state() == Qt::NoGesture )
            result = QGestureRecognizer::Ignore;
          else
            result = QGestureRecognizer::FinishGesture;
        }
        break;
    }
  case QEvent::MouseButtonPress:
  case QEvent::MouseMove:
  case QEvent::MouseButtonRelease:
      result = QGestureRecognizer::Ignore;
      break;
  default:
      result = QGestureRecognizer::Ignore;
      break;
  }
  return result;
}


GDSwipeGesture::GDSwipeGesture() :
vertDirection( QSwipeGesture::NoDirection ),
horizDirection( QSwipeGesture::NoDirection ),
started( false )
{
}

QGesture * GDSwipeGestureRecognizer::create( QObject * pTarget )
{
  if ( pTarget && pTarget->isWidgetType() ) {
    static_cast< QWidget * >( pTarget )->setAttribute( Qt::WA_AcceptTouchEvents );
  }
  QGesture *pGesture = new GDSwipeGesture;
  return pGesture;
}

void GDSwipeGestureRecognizer::reset( QGesture * pGesture )
{
  GDSwipeGesture * gest = static_cast< GDSwipeGesture * >( pGesture );
  gest->vertDirection = QSwipeGesture::NoDirection;
  gest->horizDirection = QSwipeGesture::NoDirection;
  gest->lastPositions[ 0 ] = QPoint();
  gest->lastPositions[ 1 ] = QPoint();

  QGestureRecognizer::reset( pGesture );
}

qreal GDSwipeGestureRecognizer::computeAngle( int dx, int dy )
{
   double PI = 3.14159265;

   // Need to convert from screen coordinates direction
   // into classical coordinates direction.
   dy = -dy;

   double result = atan2( (double)dy, (double)dx ) ;
   result = ( result * 180 ) / PI;

   // Always return positive angle.
   if ( result < 0 )
      result += 360;

   return result;
}

QGestureRecognizer::Result GDSwipeGestureRecognizer::recognize( QGesture * state,
                                                                QObject *,
                                                                QEvent * event)
{
  GDSwipeGesture * swipe = static_cast< GDSwipeGesture * >( state );
  const QTouchEvent * ev = static_cast< const QTouchEvent * >( event );
  QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

  switch( event->type() )
  {
    case QEvent::TouchBegin:
      {
        swipe->unsetHotSpot();
        swipe->lastPositions[ 0 ] = QPoint();
        swipe->started = true;
        result = QGestureRecognizer::MayBeGesture;
        break;
      }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    case QEvent::TouchCancel:
#endif
    case QEvent::TouchEnd:
      {
        fewTouchPointsPresented = false;
        result = QGestureRecognizer::CancelGesture;
        break;
      }
    case QEvent::TouchUpdate:
      {
        fewTouchPointsPresented = ( ev->touchPoints().size() > 1 );
        if( !swipe->started )
          result = QGestureRecognizer::CancelGesture;
        else
        if( ev->touchPoints().size() == 2 )
        {
          //2-point gesture

          QTouchEvent::TouchPoint p1 = ev->touchPoints().at( 0 );
          QTouchEvent::TouchPoint p2 = ev->touchPoints().at( 1 );

          if (swipe->lastPositions[0].isNull()) {
            swipe->lastPositions[ 0 ] = p1.startScreenPos().toPoint();
            swipe->lastPositions[ 1 ] = p2.startScreenPos().toPoint();
          }

          if( !swipe->hasHotSpot() )
          {
            swipe->setHotSpot( ( p1.startScreenPos() + p2.startScreenPos() ) / 2 );
          }

          int dx1 = p1.screenPos().toPoint().x() - swipe->lastPositions[ 0 ].x();
          int dx2 = p2.screenPos().toPoint().x() - swipe->lastPositions[ 1 ].x();
          int dy1 = p1.screenPos().toPoint().y() - swipe->lastPositions[ 0 ].y();
          int dy2 = p2.screenPos().toPoint().y() - swipe->lastPositions[ 1 ].y();

          if( qAbs( ( dx1 + dx2 ) / 2.0 ) >= MOVE_X_TRESHOLD ||
              qAbs( ( dy1 + dy2 ) / 2.0 ) >= MOVE_Y_TRESHOLD )
          {
            qreal angle1 = computeAngle( dx1, dy1 );
            qreal angle2 = computeAngle( dx2, dy2 );
            QSwipeGesture::SwipeDirection vertDir = getVerticalDirection( angle1 );
            QSwipeGesture::SwipeDirection horizDir = getHorizontalDirection( angle1 );

            if( vertDir != getVerticalDirection( angle2 ) ||
                horizDir != getHorizontalDirection( angle2 ) )
            {
              // It seems it is no swipe gesture
              result = QGestureRecognizer::CancelGesture;
              break;
            }

            if( ( swipe->vertDirection != QSwipeGesture::NoDirection && horizDir != QSwipeGesture::NoDirection ) ||
                ( swipe->horizDirection != QSwipeGesture::NoDirection && vertDir != QSwipeGesture::NoDirection ) )
            {
              // Gesture direction changed
              result = QGestureRecognizer::CancelGesture;
              break;
            }

            swipe->vertDirection = vertDir;
            swipe->horizDirection = horizDir;

            swipe->lastPositions[ 0 ] = p1.screenPos().toPoint();
            swipe->lastPositions[ 1 ] = p2.screenPos().toPoint();

            result = QGestureRecognizer::TriggerGesture;
          }
          else
            result = QGestureRecognizer::MayBeGesture;
        }
        else // No 2-point gesture
          result = QGestureRecognizer::CancelGesture;
        break;
      }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
          result = QGestureRecognizer::Ignore;
          break;
    default:
        result = QGestureRecognizer::Ignore;
        break;
  }

  return result;
}

const qreal GDPinchGestureRecognizer::OUT_SCALE_LIMIT = 0.5;
const qreal GDPinchGestureRecognizer::IN_SCALE_LIMIT = 2;

bool handleGestureEvent( QObject * obj, QEvent * event, GestureResult & result,
                         QPoint & point )
{
  result = NOT_HANLDED;

  if( event->type() != QEvent::Gesture || !obj->isWidgetType() )
    return false;

  QGestureEvent * gev = static_cast<QGestureEvent *>( event );

  gev->accept( Qt::PanGesture );

  QGesture * gesture = gev->gesture( GDSwipeGestureType );
  if( gesture )
  {
    GDSwipeGesture * swipe = static_cast< GDSwipeGesture * >( gesture );
    point = swipe->hotSpot().toPoint();

    if( swipe->getHorizDirection() != QSwipeGesture::NoDirection )
      result = swipe->getHorizDirection() == QSwipeGesture::Left ?
               SWIPE_LEFT : SWIPE_RIGHT;

    if( swipe->getVertDirection() != QSwipeGesture::NoDirection )
      result = swipe->getVertDirection() == QSwipeGesture::Up ?
                SWIPE_UP : SWIPE_DOWN;

    gev->setAccepted( true );
    return true;
  }

  gesture = gev->gesture( GDPinchGestureType );
  if( gesture )
  {
    GDPinchGesture * pinch = static_cast< GDPinchGesture * >( gesture );
    point = pinch->getCenterPoint().toPoint();

    if( pinch->isScaleChanged() )
    {
      if( pinch->getTotalScaleFactor() <= GDPinchGestureRecognizer::OUT_SCALE_LIMIT )
        result = ZOOM_OUT;
      if( pinch->getTotalScaleFactor() >= GDPinchGestureRecognizer::IN_SCALE_LIMIT )
        result = ZOOM_IN;
    }
    gev->setAccepted( true );
    return true;
  }

  gesture = gev->gesture( Qt::PanGesture );
  if( gesture )
  {
    gev->setAccepted( true );
    return true;
  }
  return false;
}

void registerRecognizers()
{
  QGestureRecognizer * pRecognizer = new Gestures::GDPinchGestureRecognizer();
  GDPinchGestureType = QGestureRecognizer::registerRecognizer( pRecognizer );

  pRecognizer = new Gestures::GDSwipeGestureRecognizer();
  GDSwipeGestureType = QGestureRecognizer::registerRecognizer( pRecognizer );
}

void unregisterRecognizers()
{
  if( GDPinchGestureType )
    QGestureRecognizer::unregisterRecognizer( GDPinchGestureType );

  if( GDSwipeGestureType )
    QGestureRecognizer::unregisterRecognizer( GDSwipeGestureType );
}

} // namespace

#endif //QT_VERSION
