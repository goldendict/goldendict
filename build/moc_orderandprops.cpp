/****************************************************************************
** Meta object code from reading C++ file 'orderandprops.hh'
**
** Created: Mon Apr 28 13:37:14 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../orderandprops.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'orderandprops.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_OrderAndProps[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   55,   14,   14, 0x08,
     106,   55,   14,   14, 0x08,
     161,  157,   14,   14, 0x08,
     201,  190,   14,   14, 0x08,
     224,   14,   14,   14, 0x08,
     242,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_OrderAndProps[] = {
    "OrderAndProps\0\0dictId\0"
    "showDictionaryHeadwords(QString)\0"
    "current\0dictionarySelectionChanged(QItemSelection)\0"
    "inactiveDictionarySelectionChanged(QItemSelection)\0"
    "pos\0contextMenuRequested(QPoint)\0"
    "filterText\0filterChanged(QString)\0"
    "dictListFocused()\0inactiveDictListFocused()\0"
};

void OrderAndProps::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        OrderAndProps *_t = static_cast<OrderAndProps *>(_o);
        switch (_id) {
        case 0: _t->showDictionaryHeadwords((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->dictionarySelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1]))); break;
        case 2: _t->inactiveDictionarySelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1]))); break;
        case 3: _t->contextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 4: _t->filterChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->dictListFocused(); break;
        case 6: _t->inactiveDictListFocused(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData OrderAndProps::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject OrderAndProps::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_OrderAndProps,
      qt_meta_data_OrderAndProps, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OrderAndProps::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OrderAndProps::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OrderAndProps::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OrderAndProps))
        return static_cast<void*>(const_cast< OrderAndProps*>(this));
    return QWidget::qt_metacast(_clname);
}

int OrderAndProps::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void OrderAndProps::showDictionaryHeadwords(QString const & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
