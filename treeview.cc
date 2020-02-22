/* This file is (c) 2008-2018 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QDropEvent>
#include "treeview.hh"

void TreeView::dropEvent( QDropEvent * event )
{
    QTreeView::dropEvent( event );

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // Qt 4 don't check success of drop operation. Add turnaround.

    if( !event->isAccepted() )
        event->setDropAction( Qt::IgnoreAction );
#endif
}
