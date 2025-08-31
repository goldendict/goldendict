/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef SQUARE_LABEL_HH_INCLUDED
#define SQUARE_LABEL_HH_INCLUDED

#include <QLabel>

class SquareLabel: public QLabel
{
  Q_OBJECT
public:
  explicit SquareLabel( QWidget * parent = 0 ): QLabel( parent ), sameHeightWidget( 0 )
  {}

  /// Binds this square label's side to @p widget's height.
  /// @warning This label must be destroyed along with or after @p widget.
  /// Make sure to call updateGeometry() on this label whenever @p widget's height may have changed.
  void setSameHeightWidget( QWidget const * widget )
  { sameHeightWidget = widget; }

  virtual QSize sizeHint() const
  {
    if( !sameHeightWidget )
      return QLabel::sizeHint();
    int const height = sameHeightWidget->height();
    return QSize( height, height );
  }

  virtual QSize minimumSizeHint() const
  { return sizeHint(); }

private:
  QWidget const * sameHeightWidget;
};

#endif // SQUARE_LABEL_HH_INCLUDED
