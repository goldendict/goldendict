#include <QStyleOptionViewItem>

#include "delegate.hh"

WordListItemDelegate::WordListItemDelegate(  QAbstractItemDelegate * delegate  ) :
QStyledItemDelegate()
{
  mainDelegate = static_cast< QStyledItemDelegate * >( delegate );
}

void WordListItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyleOptionViewItem opt4 = option;

  QStyleOptionViewItem opt = option;
  initStyleOption( &opt4, index );
  if( opt4.text.isRightToLeft() )
  {
    opt.direction = Qt::RightToLeft;
    if( opt4.textElideMode != Qt::ElideNone )
      opt.textElideMode = Qt::ElideLeft;
  }
  else
  {
    opt.direction = Qt::LeftToRight;
    if( opt4.textElideMode != Qt::ElideNone )
      opt.textElideMode = Qt::ElideRight;
  }
  mainDelegate->paint( painter, opt, index );
}
