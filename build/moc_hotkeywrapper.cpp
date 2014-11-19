/****************************************************************************
** Meta object code from reading C++ file 'hotkeywrapper.hh'
**
** Created: Mon Apr 28 13:37:05 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../hotkeywrapper.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hotkeywrapper.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HotkeyWrapper[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      43,   36,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   14,   14,   14, 0x09,
      88,   36,   83,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HotkeyWrapper[] = {
    "HotkeyWrapper\0\0hotkeyActivated(int)\0"
    "vk,mod\0keyRecorded(quint32,quint32)\0"
    "waitKey2()\0bool\0checkState(quint32,quint32)\0"
};

void HotkeyWrapper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HotkeyWrapper *_t = static_cast<HotkeyWrapper *>(_o);
        switch (_id) {
        case 0: _t->hotkeyActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->keyRecorded((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 2: _t->waitKey2(); break;
        case 3: { bool _r = _t->checkState((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData HotkeyWrapper::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HotkeyWrapper::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_HotkeyWrapper,
      qt_meta_data_HotkeyWrapper, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HotkeyWrapper::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HotkeyWrapper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HotkeyWrapper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HotkeyWrapper))
        return static_cast<void*>(const_cast< HotkeyWrapper*>(this));
    return QThread::qt_metacast(_clname);
}

int HotkeyWrapper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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
void HotkeyWrapper::hotkeyActivated(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HotkeyWrapper::keyRecorded(quint32 _t1, quint32 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
