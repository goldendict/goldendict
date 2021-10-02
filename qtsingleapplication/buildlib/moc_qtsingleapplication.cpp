/****************************************************************************
** Meta object code from reading C++ file 'qtsingleapplication.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/qtsingleapplication.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtsingleapplication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtSingleApplication_t {
    QByteArrayData data[7];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSingleApplication_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSingleApplication_t qt_meta_stringdata_QtSingleApplication = {
    {
QT_MOC_LITERAL(0, 0, 19), // "QtSingleApplication"
QT_MOC_LITERAL(1, 20, 15), // "messageReceived"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 7), // "message"
QT_MOC_LITERAL(4, 45, 11), // "sendMessage"
QT_MOC_LITERAL(5, 57, 7), // "timeout"
QT_MOC_LITERAL(6, 65, 14) // "activateWindow"

    },
    "QtSingleApplication\0messageReceived\0"
    "\0message\0sendMessage\0timeout\0"
    "activateWindow"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSingleApplication[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    2,   37,    2, 0x0a /* Public */,
       4,    1,   42,    2, 0x2a /* Public | MethodCloned */,
       6,    0,   45,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Bool, QMetaType::QString, QMetaType::Int,    3,    5,
    QMetaType::Bool, QMetaType::QString,    3,
    QMetaType::Void,

       0        // eod
};

void QtSingleApplication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtSingleApplication *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->messageReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: { bool _r = _t->sendMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 2: { bool _r = _t->sendMessage((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->activateWindow(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QtSingleApplication::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QtSingleApplication::messageReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QtSingleApplication::staticMetaObject = { {
    &QApplication::staticMetaObject,
    qt_meta_stringdata_QtSingleApplication.data,
    qt_meta_data_QtSingleApplication,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtSingleApplication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSingleApplication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtSingleApplication.stringdata0))
        return static_cast<void*>(this);
    return QApplication::qt_metacast(_clname);
}

int QtSingleApplication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QtSingleApplication::messageReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
