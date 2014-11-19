/****************************************************************************
** Meta object code from reading C++ file 'groups_widgets.hh'
**
** Created: Mon Apr 28 13:36:46 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../groups_widgets.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'groups_widgets.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DictListModel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_DictListModel[] = {
    "DictListModel\0\0contentChanged()\0"
};

void DictListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictListModel *_t = static_cast<DictListModel *>(_o);
        switch (_id) {
        case 0: _t->contentChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData DictListModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictListModel::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_DictListModel,
      qt_meta_data_DictListModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictListModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictListModel))
        return static_cast<void*>(const_cast< DictListModel*>(this));
    return QAbstractListModel::qt_metacast(_clname);
}

int DictListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void DictListModel::contentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_DictListWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_DictListWidget[] = {
    "DictListWidget\0\0gotFocus()\0"
};

void DictListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictListWidget *_t = static_cast<DictListWidget *>(_o);
        switch (_id) {
        case 0: _t->gotFocus(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData DictListWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictListWidget::staticMetaObject = {
    { &QListView::staticMetaObject, qt_meta_stringdata_DictListWidget,
      qt_meta_data_DictListWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictListWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictListWidget))
        return static_cast<void*>(const_cast< DictListWidget*>(this));
    return QListView::qt_metacast(_clname);
}

int DictListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void DictListWidget::gotFocus()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_DictGroupWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   16,   16,   16, 0x08,
      76,   72,   16,   16, 0x08,
      97,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DictGroupWidget[] = {
    "DictGroupWidget\0\0id\0showDictionaryInfo(QString)\0"
    "groupIconActivated(int)\0pos\0"
    "showDictInfo(QPoint)\0"
    "removeCurrentItem(QModelIndex)\0"
};

void DictGroupWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictGroupWidget *_t = static_cast<DictGroupWidget *>(_o);
        switch (_id) {
        case 0: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->groupIconActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->showDictInfo((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 3: _t->removeCurrentItem((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DictGroupWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictGroupWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DictGroupWidget,
      qt_meta_data_DictGroupWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictGroupWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictGroupWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictGroupWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictGroupWidget))
        return static_cast<void*>(const_cast< DictGroupWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int DictGroupWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void DictGroupWidget::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_DictGroupsWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      49,   17,   17,   17, 0x08,
      69,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DictGroupsWidget[] = {
    "DictGroupsWidget\0\0id\0showDictionaryInfo(QString)\0"
    "contextMenu(QPoint)\0tabDataChanged()\0"
};

void DictGroupsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictGroupsWidget *_t = static_cast<DictGroupsWidget *>(_o);
        switch (_id) {
        case 0: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->contextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 2: _t->tabDataChanged(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DictGroupsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictGroupsWidget::staticMetaObject = {
    { &QTabWidget::staticMetaObject, qt_meta_stringdata_DictGroupsWidget,
      qt_meta_data_DictGroupsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictGroupsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictGroupsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictGroupsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictGroupsWidget))
        return static_cast<void*>(const_cast< DictGroupsWidget*>(this));
    return QTabWidget::qt_metacast(_clname);
}

int DictGroupsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void DictGroupsWidget::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QuickFilterLine[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   16,   16,   16, 0x08,
      71,   16,   16,   16, 0x08,
      91,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QuickFilterLine[] = {
    "QuickFilterLine\0\0filter\0filterChanged(QString)\0"
    "filterChangedInternal()\0emitFilterChanged()\0"
    "focusFilterLine()\0"
};

void QuickFilterLine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QuickFilterLine *_t = static_cast<QuickFilterLine *>(_o);
        switch (_id) {
        case 0: _t->filterChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->filterChangedInternal(); break;
        case 2: _t->emitFilterChanged(); break;
        case 3: _t->focusFilterLine(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QuickFilterLine::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QuickFilterLine::staticMetaObject = {
    { &ExtLineEdit::staticMetaObject, qt_meta_stringdata_QuickFilterLine,
      qt_meta_data_QuickFilterLine, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QuickFilterLine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QuickFilterLine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QuickFilterLine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QuickFilterLine))
        return static_cast<void*>(const_cast< QuickFilterLine*>(this));
    return ExtLineEdit::qt_metacast(_clname);
}

int QuickFilterLine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ExtLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QuickFilterLine::filterChanged(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
