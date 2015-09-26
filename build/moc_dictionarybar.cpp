/****************************************************************************
** Meta object code from reading C++ file 'dictionarybar.hh'
**
** Created: Wed Nov 19 16:06:41 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../dictionarybar.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dictionarybar.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DictionaryBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      39,   36,   14,   14, 0x05,
      67,   36,   14,   14, 0x05,
     100,   36,   14,   14, 0x05,
     130,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     147,   14,   14,   14, 0x08,
     174,   14,   14,   14, 0x08,
     203,   14,   14,   14, 0x08,
     244,  229,   14,   14, 0x08,
     291,  285,   14,   14, 0x28,

       0        // eod
};

static const char qt_meta_stringdata_DictionaryBar[] = {
    "DictionaryBar\0\0editGroupRequested()\0"
    "id\0showDictionaryInfo(QString)\0"
    "showDictionaryHeadwords(QString)\0"
    "openDictionaryFolder(QString)\0"
    "closePopupMenu()\0mutedDictionariesChanged()\0"
    "actionWasTriggered(QAction*)\0"
    "dictsPaneClicked(QString)\0event,extended\0"
    "showContextMenu(QContextMenuEvent*,bool)\0"
    "event\0showContextMenu(QContextMenuEvent*)\0"
};

void DictionaryBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DictionaryBar *_t = static_cast<DictionaryBar *>(_o);
        switch (_id) {
        case 0: _t->editGroupRequested(); break;
        case 1: _t->showDictionaryInfo((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->showDictionaryHeadwords((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->openDictionaryFolder((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->closePopupMenu(); break;
        case 5: _t->mutedDictionariesChanged(); break;
        case 6: _t->actionWasTriggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 7: _t->dictsPaneClicked((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->showContextMenu((*reinterpret_cast< QContextMenuEvent*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 9: _t->showContextMenu((*reinterpret_cast< QContextMenuEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DictionaryBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DictionaryBar::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_DictionaryBar,
      qt_meta_data_DictionaryBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DictionaryBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DictionaryBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DictionaryBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DictionaryBar))
        return static_cast<void*>(const_cast< DictionaryBar*>(this));
    return QToolBar::qt_metacast(_clname);
}

int DictionaryBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void DictionaryBar::editGroupRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void DictionaryBar::showDictionaryInfo(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DictionaryBar::showDictionaryHeadwords(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DictionaryBar::openDictionaryFolder(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DictionaryBar::closePopupMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE
