#include <QStyleOptionViewItemV4>

#include "delegate.hh"

WordListItemDelegate::WordListItemDelegate(  QAbstractItemDelegate * delegate  ) :
QStyledItemDelegate()
{
  mainDelegate = static_cast< QStyledItemDelegate * >( delegate );
}

void WordListItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyleOptionViewItemV4 opt4 = option;
  QStyleOptionViewItem opt = option;
  initStyleOption( &opt4, index );
  if( opt4.text.isRightToLeft() )
    opt.direction = Qt::RightToLeft;
  mainDelegate->paint( painter, opt, index );
}
