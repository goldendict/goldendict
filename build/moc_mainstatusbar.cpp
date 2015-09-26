/****************************************************************************
** Meta object code from reading C++ file 'mainstatusbar.hh'
**
** Created: Wed Nov 19 16:06:55 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainstatusbar.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainstatusbar.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainStatusBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       1,   34, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      35,   15,   14,   14, 0x0a,
      81,   68,   14,   14, 0x2a,
     111,  106,   14,   14, 0x2a,
     132,   14,   14,   14, 0x0a,

 // properties: name, type, flags
     152,  147, 0x01095001,

       0        // eod
};

static const char qt_meta_stringdata_MainStatusBar[] = {
    "MainStatusBar\0\0text,timeout,pixmap\0"
    "showMessage(QString,int,QPixmap)\0"
    "text,timeout\0showMessage(QString,int)\0"
    "text\0showMessage(QString)\0clearMessage()\0"
    "bool\0hasImage\0"
};

void MainStatusBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainStatusBar *_t = static_cast<MainStatusBar *>(_o);
        switch (_id) {
        case 0: _t->showMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QPixmap(*)>(_a[3]))); break;
        case 1: _t->showMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->showMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->clearMessage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainStatusBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainStatusBar::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MainStatusBar,
      qt_meta_data_MainStatusBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainStatusBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainStatusBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainStatusBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainStatusBar))
        return static_cast<void*>(const_cast< MainStatusBar*>(this));
    return QWidget::qt_metacast(_clname);
}

int MainStatusBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = hasImage(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
