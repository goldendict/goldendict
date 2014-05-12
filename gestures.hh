#ifndef __GESTURES_HH_INCLUDED__
#define __GESTURES_HH_INCLUDED__

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)

#include <QGestureRecognizer>
#include <QGesture>
#include <QTimer>
#include <QEvent>
#include <QAction>

namespace Gestures
{

enum GestureResult
{
  NOT_HANLDED,
  SWIPE_LEFT,
  SWIPE_RIGHT,
  SWIPE_UP,
  SWIPE_DOWN,
  ZOOM_IN,
  ZOOM_OUT
};

extern Qt::GestureType GDPinchGestureType;
extern Qt::GestureType GDSwipeGestureType;

void registerRecognizers();

void unregisterRecognizers();

bool isFewTouchPointsPresented();

class GDPinchGestureRecognizer;
class GDPinchGesture : public QGesture
{
public:
  GDPinchGesture();

  bool isScaleChanged() const
  { return scaleChanged; }
  QPointF const & getCenterPoint() const
  { return centerPoint; }
  qreal getTotalScaleFactor() const
  { return totalScaleFactor; }

protected:
  QPointF startCenterPoint;
  QPointF lastCenterPoint;
  QPointF centerPoint;

  qreal lastScaleFactor;
  qreal scaleFactor;
  qreal totalScaleFactor;

  bool isNewSequence;
  QPointF startPosition[ 2 ];
  bool scaleChanged;

  friend class GDPinchGestureRecognizer;
};

class GDSwipeGestureRecognizer;
class GDSwipeGesture : public QGesture
{
public:
  GDSwipeGesture();
  QSwipeGesture::SwipeDirection getHorizDirection() const
  { return horizDirection; }
  QSwipeGesture::SwipeDirection getVertDirection() const
  { return vertDirection; }

protected:
  QSwipeGesture::SwipeDirection vertDirection;
  QSwipeGesture::SwipeDirection horizDirection;
  QPoint lastPositions[ 2 ];
  bool started;

  friend class GDSwipeGestureRecognizer;
};

class GDPinchGestureRecognizer : public QGestureRecognizer
{
public:
  static const qreal OUT_SCALE_LIMIT;
  static const qreal IN_SCALE_LIMIT;

  GDPinchGestureRecognizer() {}
private:

  virtual QGesture* create( QObject* pTarget );
  virtual QGestureRecognizer::Result recognize( QGesture* pGesture, QObject *pWatched, QEvent * pEvent );
  void reset( QGesture *pGesture );
};

class GDSwipeGestureRecognizer : public QGestureRecognizer
{
public:
  GDSwipeGestureRecognizer(){}
private:
  static const int MOVE_X_TRESHOLD = 100;
  static const int MOVE_Y_TRESHOLD = 50;

  virtual QGesture * create( QObject* pTarget );
  virtual QGestureRecognizer::Result recognize( QGesture* pGesture, QObject * pWatched, QEvent * pEvent );
  void reset ( QGesture * pGesture );
  qreal computeAngle( int dx, int dy );
};

bool handleGestureEvent( QObject * obj, QEvent * event, GestureResult & result, QPoint & point );

} // namespace

#endif

#endif // __GESTURES_HH_INCLUDED__
