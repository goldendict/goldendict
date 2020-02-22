/* This file is (c) 2008-2018 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __TREEVIEW_HH__INCLUDED
#define __TREEVIEW_HH__INCLUDED

#include <QTreeView>

class TreeView : public QTreeView
{
public:
    TreeView( QWidget * parent = 0 ) :
        QTreeView( parent )
    {}

protected:
    virtual void dropEvent( QDropEvent * event );
};

#endif // TREEVIEW_HH
