/* This file is (c) 2013 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __HISTORYPANEWIDGET_HH_INCLUDED__
#define __HISTORYPANEWIDGET_HH_INCLUDED__

#include <QAbstractListModel>
#include <QListView>
#include <QLabel>
#include <QHBoxLayout>

class QMenu;
class History;
namespace  Config { struct Class; }

/// A widget holding the contents of the 'History' docklet.
class HistoryPaneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryPaneWidget( QWidget * parent = 0 ): QWidget( parent ),
        itemSelectionChanged( false )
    {}
    virtual ~HistoryPaneWidget();

    virtual QSize sizeHint() const
    { return QSize( 204, 204 ); }

    void setUp( Config::Class * cfg,  History * history, QMenu * menu );

signals:
    void historyItemRequested( QString const & word );

public slots:
    void updateHistoryCounts();

private slots:
    void emitHistoryItemRequested(QModelIndex const &);
    void onSelectionChanged(QItemSelection const & selection);
    void onItemClicked(QModelIndex const & idx);
    void showCustomMenu(QPoint const & pos);
    void deleteSelectedItems();
    void copySelectedItems();

private:
    virtual bool eventFilter( QObject *, QEvent * );

    Config::Class * m_cfg ;
    History * m_history;
    QListView * m_historyList;
    QMenu * m_historyMenu;
    QAction * m_deleteSelectedAction;
    QAction * m_separator;
    QAction * m_copySelectedToClipboard;

    QWidget historyPaneTitleBar;
    QHBoxLayout historyPaneTitleBarLayout;
    QLabel historyLabel;
    QLabel historyCountLabel;

    /// needed to avoid multiple notifications
    /// when selecting history items via mouse and keyboard
    bool itemSelectionChanged;

};

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit HistoryModel( History * history , QObject * parent = 0 );

    int rowCount( QModelIndex const & parent = QModelIndex() ) const;

    QVariant data( QModelIndex const & index, int role = Qt::DisplayRole ) const;

private slots:
    void historyChanged();

private:
    History * m_history;
};

#endif // HISTORYPANEWIDGET_HH
