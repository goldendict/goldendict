/****************************************************************************
** Meta object code from reading C++ file 'mouseover.hh'
**
** Created: Wed Nov 19 16:06:25 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mouseover.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mouseover.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MouseOver[] = {

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
      23,   11,   10,   10, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_MouseOver[] = {
    "MouseOver\0\0,forcePopup\0hovered(QString,bool)\0"
};

void MouseOver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MouseOver *_t = static_cast<MouseOver *>(_o);
        switch (_id) {
        case 0: _t->hovered((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MouseOver::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MouseOver::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MouseOver,
      qt_meta_data_MouseOver, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MouseOver::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MouseOver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MouseOver::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MouseOver))
        return static_cast<void*>(const_cast< MouseOver*>(this));
    if (!strcmp(_clname, "KeyboardState"))
        return static_cast< KeyboardState*>(const_cast< MouseOver*>(this));
    return QObject::qt_metacast(_clname);
}

int MouseOver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void MouseOver::hovered(QString const & _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
