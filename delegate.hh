#ifndef __DELEGATE_HH_INCLUDED__
#define __DELEGATE_HH_INCLUDED__

#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>

class WordListItemDelegate : public QStyledItemDelegate
{
public:
  WordListItemDelegate( QAbstractItemDelegate * delegate );
  virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:
  QStyledItemDelegate * mainDelegate;
};

#endif

