/****************************************************************************
** Meta object code from reading C++ file 'qtsinglecoreapplication.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qtsinglecoreapplication.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtsinglecoreapplication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtSingleCoreApplication_t {
    QByteArrayData data[6];
    char stringdata0[69];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtSingleCoreApplication_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtSingleCoreApplication_t qt_meta_stringdata_QtSingleCoreApplication = {
    {
QT_MOC_LITERAL(0, 0, 23), // "QtSingleCoreApplication"
QT_MOC_LITERAL(1, 24, 15), // "messageReceived"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 7), // "message"
QT_MOC_LITERAL(4, 49, 11), // "sendMessage"
QT_MOC_LITERAL(5, 61, 7) // "timeout"

    },
    "QtSingleCoreApplication\0messageReceived\0"
    "\0message\0sendMessage\0timeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtSingleCoreApplication[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    2,   32,    2, 0x0a /* Public */,
       4,    1,   37,    2, 0x2a /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Bool, QMetaType::QString, QMetaType::Int,    3,    5,
    QMetaType::Bool, QMetaType::QString,    3,

       0        // eod
};

void QtSingleCoreApplication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtSingleCoreApplication *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->messageReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: { bool _r = _t->sendMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 2: { bool _r = _t->sendMessage((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QtSingleCoreApplication::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QtSingleCoreApplication::messageReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QtSingleCoreApplication::staticMetaObject = { {
    &QCoreApplication::staticMetaObject,
    qt_meta_stringdata_QtSingleCoreApplication.data,
    qt_meta_data_QtSingleCoreApplication,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtSingleCoreApplication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtSingleCoreApplication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtSingleCoreApplication.stringdata0))
        return static_cast<void*>(this);
    return QCoreApplication::qt_metacast(_clname);
}

int QtSingleCoreApplication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QCoreApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QtSingleCoreApplication::messageReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
