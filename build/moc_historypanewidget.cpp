/****************************************************************************
** Meta object code from reading C++ file 'historypanewidget.hh'
**
** Created: Mon Apr 28 13:37:37 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../historypanewidget.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'historypanewidget.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HistoryPaneWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   18,   18,   18, 0x0a,
      76,   18,   18,   18, 0x08,
     124,  114,   18,   18, 0x08,
     163,  159,   18,   18, 0x08,
     194,  190,   18,   18, 0x08,
     217,   18,   18,   18, 0x08,
     239,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HistoryPaneWidget[] = {
    "HistoryPaneWidget\0\0word\0"
    "historyItemRequested(QString)\0"
    "updateHistoryCounts()\0"
    "emitHistoryItemRequested(QModelIndex)\0"
    "selection\0onSelectionChanged(QItemSelection)\0"
    "idx\0onItemClicked(QModelIndex)\0pos\0"
    "showCustomMenu(QPoint)\0deleteSelectedItems()\0"
    "copySelectedItems()\0"
};

void HistoryPaneWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HistoryPaneWidget *_t = static_cast<HistoryPaneWidget *>(_o);
        switch (_id) {
        case 0: _t->historyItemRequested((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->updateHistoryCounts(); break;
        case 2: _t->emitHistoryItemRequested((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 3: _t->onSelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1]))); break;
        case 4: _t->onItemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 5: _t->showCustomMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 6: _t->deleteSelectedItems(); break;
        case 7: _t->copySelectedItems(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData HistoryPaneWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HistoryPaneWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_HistoryPaneWidget,
      qt_meta_data_HistoryPaneWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HistoryPaneWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HistoryPaneWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HistoryPaneWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HistoryPaneWidget))
        return static_cast<void*>(const_cast< HistoryPaneWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int HistoryPaneWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void HistoryPaneWidget::historyItemRequested(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_HistoryModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HistoryModel[] = {
    "HistoryModel\0\0historyChanged()\0"
};

void HistoryModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HistoryModel *_t = static_cast<HistoryModel *>(_o);
        switch (_id) {
        case 0: _t->historyChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData HistoryModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HistoryModel::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_HistoryModel,
      qt_meta_data_HistoryModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HistoryModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HistoryModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HistoryModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HistoryModel))
        return static_cast<void*>(const_cast< HistoryModel*>(this));
    return QAbstractListModel::qt_metacast(_clname);
}

int HistoryModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
