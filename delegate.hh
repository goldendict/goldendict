#ifndef __DELEGATE_HH_INCLUDED__
#define __DELEGATE_HH_INCLUDED__

#include <QStyledItemDelegate>

class WordListItemDelegate : public QStyledItemDelegate
{
public:
  WordListItemDelegate(QAbstractItemView *parent );
  virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
  QStyledItemDelegate * mainDelegate;
};

#endif

