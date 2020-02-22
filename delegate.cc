#include <QStyleOptionViewItemV4>
#include <QAbstractItemView>
#include "delegate.hh"

WordListItemDelegate::WordListItemDelegate(QAbstractItemView *parent) :
    QStyledItemDelegate(parent)
{
    mainDelegate = qobject_cast< QStyledItemDelegate * >( parent->itemDelegate());
    parent->setItemDelegate( this );
}

void WordListItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QStyleOptionViewItem opt4 = option;
#else
    QStyleOptionViewItemV4 opt4 = option;
#endif
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
